#include "codegen.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    int offset;
} VarBinding;

typedef struct {
    VarBinding vars[128];
    int var_count;
    int current_offset;
} SymbolTable;

static void sym_init(SymbolTable* sym) {
    sym->var_count = 0;
    sym->current_offset = 0;
}

static int sym_get_or_add(SymbolTable* sym, const char* name) {
    for (int i = 0; i < sym->var_count; i++) {
        if (strcmp(sym->vars[i].name, name) == 0) {
            return sym->vars[i].offset;
        }
    }
    sym->current_offset += 8;
    strcpy(sym->vars[sym->var_count].name, name);
    sym->vars[sym->var_count].offset = sym->current_offset;
    sym->var_count++;
    return sym->current_offset;
}

static void load_operand(FILE* out, IrOperand* op, const char* reg, SymbolTable* sym, int temp_base_offset) {
    if (op->type == OP_TYPE_CONST) {
        fprintf(out, "    mov %s, %d\n", reg, op->index_or_val);
    } else if (op->type == OP_TYPE_VAR) {
        int offset = sym_get_or_add(sym, op->var_name);
        fprintf(out, "    mov %s, qword [rbp - %d]\n", reg, offset);
    } else if (op->type == OP_TYPE_TEMP) {
        int offset = temp_base_offset + (op->index_or_val * 8);
        fprintf(out, "    mov %s, qword [rbp - %d]\n", reg, offset);
    }
}

static void store_operand(FILE* out, IrOperand* op, const char* reg, SymbolTable* sym, int temp_base_offset) {
    if (op->type == OP_TYPE_VAR) {
        int offset = sym_get_or_add(sym, op->var_name);
        fprintf(out, "    mov qword [rbp - %d], %s\n", offset, reg);
    } else if (op->type == OP_TYPE_TEMP) {
        int offset = temp_base_offset + (op->index_or_val * 8);
        fprintf(out, "    mov qword [rbp - %d], %s\n", offset, reg);
    }
}

void codegen_generate(IrContext* ir_ctx, FILE* out) {
    SymbolTable sym;
    sym_init(&sym);
    
    // Pass 1: Find all unique variables to compute stack frame
    for (int i = 0; i < ir_ctx->count; i++) {
        IrInstruction* inst = &ir_ctx->instrs[i];
        if (inst->dest.type == OP_TYPE_VAR) sym_get_or_add(&sym, inst->dest.var_name);
        if (inst->src1.type == OP_TYPE_VAR) sym_get_or_add(&sym, inst->src1.var_name);
        if (inst->src2.type == OP_TYPE_VAR) sym_get_or_add(&sym, inst->src2.var_name);
    }
    
    int temp_base_offset = sym.current_offset + 8;
    int max_temps = ir_ctx->next_temp;
    int stack_size = temp_base_offset + (max_temps * 8);
    // Align stack size to 16 bytes
    stack_size = (stack_size + 15) & ~15;

    fprintf(out, "section .text\n");
    fprintf(out, "global _start\n\n");
    fprintf(out, "_start:\n");
    fprintf(out, "    push rbp\n");
    fprintf(out, "    mov rbp, rsp\n");
    fprintf(out, "    sub rsp, %d\n\n", stack_size);

    for (int i = 0; i < ir_ctx->count; i++) {
        IrInstruction* inst = &ir_ctx->instrs[i];
        
        switch (inst->op) {
            case IR_OP_CONST:
                load_operand(out, &inst->src1, "rax", &sym, temp_base_offset);
                store_operand(out, &inst->dest, "rax", &sym, temp_base_offset);
                break;
                
            case IR_OP_ASSIGN:
                load_operand(out, &inst->src1, "rax", &sym, temp_base_offset);
                store_operand(out, &inst->dest, "rax", &sym, temp_base_offset);
                break;
                
            case IR_OP_ADD:
                load_operand(out, &inst->src1, "rax", &sym, temp_base_offset);
                load_operand(out, &inst->src2, "rbx", &sym, temp_base_offset);
                fprintf(out, "    add rax, rbx\n");
                store_operand(out, &inst->dest, "rax", &sym, temp_base_offset);
                break;
                
            case IR_OP_CMP_EQ:
            case IR_OP_CMP_NE:
            case IR_OP_CMP_LT:
            case IR_OP_CMP_GT:
            case IR_OP_CMP_LE:
            case IR_OP_CMP_GE: {
                load_operand(out, &inst->src1, "rax", &sym, temp_base_offset);
                load_operand(out, &inst->src2, "rbx", &sym, temp_base_offset);
                fprintf(out, "    cmp rax, rbx\n");
                
                const char* setcc = "";
                if (inst->op == IR_OP_CMP_EQ) setcc = "sete";
                else if (inst->op == IR_OP_CMP_NE) setcc = "setne";
                else if (inst->op == IR_OP_CMP_LT) setcc = "setl";
                else if (inst->op == IR_OP_CMP_GT) setcc = "setg";
                else if (inst->op == IR_OP_CMP_LE) setcc = "setle";
                else if (inst->op == IR_OP_CMP_GE) setcc = "setge";
                
                fprintf(out, "    %s al\n", setcc);
                fprintf(out, "    movzx rax, al\n");
                store_operand(out, &inst->dest, "rax", &sym, temp_base_offset);
                break;
            }
                
            case IR_OP_JMP:
                fprintf(out, "    jmp L%d\n", inst->src1.index_or_val);
                break;
                
            case IR_OP_JMP_IF_FALSE:
                load_operand(out, &inst->src1, "rax", &sym, temp_base_offset);
                fprintf(out, "    test rax, rax\n");
                fprintf(out, "    jz L%d\n", inst->src2.index_or_val);
                break;
                
            case IR_OP_EXIT:
                load_operand(out, &inst->src1, "rdi", &sym, temp_base_offset);
                fprintf(out, "    mov rax, 60\n"); // syscall: exit
                fprintf(out, "    syscall\n");
                break;
                
            case IR_OP_LABEL:
                fprintf(out, "L%d:\n", inst->src1.index_or_val);
                break;
        }
    }

    // Default exit if execution reaches end without explicit exit
    fprintf(out, "\n    ; Default exit 0\n");
    fprintf(out, "    mov rax, 60\n");
    fprintf(out, "    mov rdi, 0\n");
    fprintf(out, "    syscall\n");
}
