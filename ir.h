#ifndef IR_H
#define IR_H

#include "ast.h"

typedef enum {
    IR_OP_CONST,
    IR_OP_ASSIGN,
    IR_OP_ADD,
    IR_OP_CMP_EQ,
    IR_OP_CMP_NE,
    IR_OP_CMP_LT,
    IR_OP_CMP_GT,
    IR_OP_CMP_LE,
    IR_OP_CMP_GE,
    IR_OP_JMP,
    IR_OP_JMP_IF_FALSE,
    IR_OP_EXIT,
    IR_OP_LABEL
} IrOpcode;

typedef enum {
    ACCESS_UNKNOWN,
    ACCESS_SEQUENTIAL
} AccessPattern;

typedef enum {
    REUSE_LOW,
    REUSE_MEDIUM,
    REUSE_HIGH
} ReuseHint;

typedef enum {
    OP_TYPE_EMPTY,
    OP_TYPE_TEMP,
    OP_TYPE_VAR,
    OP_TYPE_CONST,
    OP_TYPE_LABEL
} IrOperandType;

typedef struct {
    IrOperandType type;
    union {
        int index_or_val;
        char var_name[64];
    };
} IrOperand;

typedef struct {
    IrOpcode op;
    IrOperand dest;
    IrOperand src1;
    IrOperand src2;
    
    AccessPattern access_pattern;
    ReuseHint reuse_hint;
    int reuse_distance;
} IrInstruction;

typedef struct {
    char name[64];
    int last_used_inst;
} MemoryAccess;

typedef struct {
    IrInstruction* instrs;
    int count;
    int capacity;
    int next_temp;
    int next_label;
    
    MemoryAccess accesses[128];
    int access_count;
} IrContext;

void ir_context_init(IrContext* ctx);
void ir_generate(IrContext* ctx, AstNode* node);
void ir_print(IrContext* ctx);

#endif
