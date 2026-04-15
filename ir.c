#include "ir.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void ir_context_init(IrContext* ctx) {
    ctx->capacity = 64;
    ctx->count = 0;
    ctx->next_temp = 0;
    ctx->next_label = 0;
    ctx->access_count = 0;
    ctx->instrs = (IrInstruction*)malloc(sizeof(IrInstruction) * ctx->capacity);
}

int compute_reuse_distance(IrContext* ctx, const char* var) {
    for (int i = 0; i < ctx->access_count; i++) {
        if (strcmp(ctx->accesses[i].name, var) == 0) {
            int current_inst = ctx->count;
            int dist = current_inst - ctx->accesses[i].last_used_inst;
            ctx->accesses[i].last_used_inst = current_inst;
            return dist;
        }
    }
    
    strcpy(ctx->accesses[ctx->access_count].name, var);
    ctx->accesses[ctx->access_count].last_used_inst = ctx->count;
    ctx->access_count++;
    
    return -1;
}

AccessPattern infer_access_pattern(IrOperand src1, IrOperand src2) {
    if (src1.type == OP_TYPE_VAR || src2.type == OP_TYPE_VAR)
        return ACCESS_SEQUENTIAL;
    return ACCESS_UNKNOWN;
}

ReuseHint infer_reuse_hint(IrOperand src1, IrOperand src2) {
    if (src1.type == OP_TYPE_VAR || src2.type == OP_TYPE_VAR)
        return REUSE_HIGH;
    
    if (src1.type == OP_TYPE_CONST && src2.type == OP_TYPE_CONST)
        return REUSE_LOW;
        
    return REUSE_MEDIUM;
}

static void ir_emit(IrContext* ctx, IrOpcode op, IrOperand dest, IrOperand src1, IrOperand src2) {
    if (ctx->count >= ctx->capacity) {
        ctx->capacity *= 2;
        ctx->instrs = (IrInstruction*)realloc(ctx->instrs, sizeof(IrInstruction) * ctx->capacity);
    }
    
    IrInstruction* inst = &ctx->instrs[ctx->count];
    
    inst->op = op;
    inst->dest = dest;
    inst->src1 = src1;
    inst->src2 = src2;
    
    inst->access_pattern = infer_access_pattern(src1, src2);
    inst->reuse_hint = infer_reuse_hint(src1, src2);
    inst->reuse_distance = -1;
    
    if (src1.type == OP_TYPE_VAR) {
        inst->reuse_distance = compute_reuse_distance(ctx, src1.var_name);
    }
    
    if (src2.type == OP_TYPE_VAR) {
        int d = compute_reuse_distance(ctx, src2.var_name);
        if (d != -1) inst->reuse_distance = d;
    }
    
    if (dest.type == OP_TYPE_VAR) {
        compute_reuse_distance(ctx, dest.var_name);
    }
    
    ctx->count++;
}

static IrOperand make_empty() { IrOperand o; o.type = OP_TYPE_EMPTY; return o; }
static IrOperand make_temp(int idx) { IrOperand o; o.type = OP_TYPE_TEMP; o.index_or_val = idx; return o; }
static IrOperand make_var(const char* name) { IrOperand o; o.type = OP_TYPE_VAR; strcpy(o.var_name, name); return o; }
static IrOperand make_const(int val) { IrOperand o; o.type = OP_TYPE_CONST; o.index_or_val = val; return o; }
static IrOperand make_label(int idx) { IrOperand o; o.type = OP_TYPE_LABEL; o.index_or_val = idx; return o; }

// Generate IR for an expression and return the operand where the result is stored
static IrOperand ir_generate_expr(IrContext* ctx, AstNode* node) {
    if (node->type == AST_INT_LIT) {
        IrOperand dest = make_temp(ctx->next_temp++);
        IrOperand src1 = make_const(node->int_val);
        ir_emit(ctx, IR_OP_CONST, dest, src1, make_empty());
        return dest;
    } else if (node->type == AST_IDENTIFIER) {
        IrOperand dest = make_temp(ctx->next_temp++);
        IrOperand src1 = make_var(node->var_name);
        // Loading variable: dest = var
        ir_emit(ctx, IR_OP_ASSIGN, dest, src1, make_empty());
        return dest;
    } else if (node->type == AST_BINARY_OP) {
        IrOperand t1 = ir_generate_expr(ctx, node->left);
        IrOperand t2 = ir_generate_expr(ctx, node->right);
        IrOperand dest = make_temp(ctx->next_temp++);
        
        IrOpcode op = IR_OP_ADD; // default
        if (node->op == TOK_EQ) op = IR_OP_CMP_EQ;
        else if (node->op == TOK_NEQ) op = IR_OP_CMP_NE;
        else if (node->op == TOK_LT) op = IR_OP_CMP_LT;
        else if (node->op == TOK_GT) op = IR_OP_CMP_GT;
        else if (node->op == TOK_LTE) op = IR_OP_CMP_LE;
        else if (node->op == TOK_GTE) op = IR_OP_CMP_GE;
        
        ir_emit(ctx, op, dest, t1, t2);
        return dest;
    }
    return make_empty();
}

void ir_generate(IrContext* ctx, AstNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
        case AST_BLOCK:
            for (int i = 0; i < node->stmt_count; i++) {
                ir_generate(ctx, node->stmts[i]);
            }
            break;
            
        case AST_VAR_DECL: {
            IrOperand val = ir_generate_expr(ctx, node->left);
            IrOperand var = make_var(node->var_name);
            ir_emit(ctx, IR_OP_ASSIGN, var, val, make_empty());
            break;
        }
            
        case AST_EXIT_STMT: {
            IrOperand val = ir_generate_expr(ctx, node->left);
            ir_emit(ctx, IR_OP_EXIT, make_empty(), val, make_empty());
            break;
        }
            
        case AST_IF_STMT: {
            IrOperand cond = ir_generate_expr(ctx, node->condition);
            
            int false_label = ctx->next_label++;
            int end_label = ctx->next_label++;
            
            ir_emit(ctx, IR_OP_JMP_IF_FALSE, make_empty(), cond, make_label(false_label));
            
            ir_generate(ctx, node->true_branch);
            if (node->false_branch) {
                ir_emit(ctx, IR_OP_JMP, make_empty(), make_label(end_label), make_empty());
            }
            
            ir_emit(ctx, IR_OP_LABEL, make_empty(), make_label(false_label), make_empty());
            
            if (node->false_branch) {
                ir_generate(ctx, node->false_branch);
                ir_emit(ctx, IR_OP_LABEL, make_empty(), make_label(end_label), make_empty());
            }
            break;
        }
            
        default:
            break;
    }
}

void ir_print(IrContext* ctx) {
    for (int i = 0; i < ctx->count; i++) {
        IrInstruction* inst = &ctx->instrs[i];
        
        printf("INST %d | op=%d | reuse=%d | pattern=%d | dist=%d\n",
               i,
               inst->op,
               inst->reuse_hint,
               inst->access_pattern,
               inst->reuse_distance);
    }
}
