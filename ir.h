#ifndef IR_H
#define IR_H

#include "ast.h"

typedef enum {
    IR_OP_CONST,    // t1 = const 5
    IR_OP_ASSIGN,   // dest = src1
    IR_OP_ADD,      // t3 = t1 + t2
    IR_OP_CMP_EQ,
    IR_OP_CMP_NE,
    IR_OP_CMP_LT,
    IR_OP_CMP_GT,
    IR_OP_CMP_LE,
    IR_OP_CMP_GE,
    IR_OP_JMP,      // goto L1
    IR_OP_JMP_IF_FALSE, // if !src1 goto L1
    IR_OP_EXIT,     // exit src1
    IR_OP_LABEL     // L1:
} IrOpcode;

typedef enum {
    ACCESS_SEQUENTIAL,
    ACCESS_UNKNOWN
} AccessPattern;

typedef enum {
    REUSE_HIGH,
    REUSE_LOW
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
        int index_or_val;  // for temp t1 (index=1), const (val=5), or label L1 (index=1)
        char var_name[64]; // for variables
    };
} IrOperand;

typedef struct {
    IrOpcode op;
    IrOperand dest;
    IrOperand src1;
    IrOperand src2;
    
    // Memory-aware extension
    AccessPattern access_pattern;
    ReuseHint reuse_hint;
} IrInstruction;

typedef struct {
    IrInstruction* instrs;
    int count;
    int capacity;
    int next_temp;
    int next_label;
} IrContext;

void ir_context_init(IrContext* ctx);
void ir_generate(IrContext* ctx, AstNode* node);

#endif
