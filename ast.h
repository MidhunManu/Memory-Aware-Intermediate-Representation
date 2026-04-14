#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL, // let x = ...
    AST_IF_STMT,  // if (cond) { true_br } else { false_br }
    AST_EXIT_STMT,// exit(expr)
    AST_BLOCK,    // list of statements
    AST_BINARY_OP,// a < b
    AST_INT_LIT,  // 5
    AST_IDENTIFIER// x
} AstNodeType;

typedef struct AstNode {
    AstNodeType type;
    
    // For INT_LIT
    int int_val;
    
    // For IDENTIFIER / VAR_DECL
    char var_name[64];
    
    // For BINARY_OP (operator type like TOK_LT, TOK_GT)
    TokenType op;
    
    // Children
    struct AstNode* left;
    struct AstNode* right;
    
    // For Blocks / Programs (list of stmts)
    struct AstNode** stmts;
    int stmt_count;
    int stmt_capacity;
    
    // For IF_STMT
    struct AstNode* condition;
    struct AstNode* true_branch;
    struct AstNode* false_branch;
    
} AstNode;

static inline AstNode* ast_new_node(AstNodeType type) {
    AstNode* node = (AstNode*)calloc(1, sizeof(AstNode));
    node->type = type;
    return node;
}

static inline void ast_add_stmt(AstNode* block, AstNode* stmt) {
    if (block->stmt_count >= block->stmt_capacity) {
        block->stmt_capacity = block->stmt_capacity == 0 ? 8 : block->stmt_capacity * 2;
        block->stmts = (AstNode**)realloc(block->stmts, block->stmt_capacity * sizeof(AstNode*));
    }
    block->stmts[block->stmt_count++] = stmt;
}

#endif
