#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance(Parser* parser) {
    parser->current_token = get_next_token(parser->lexer);
}

static void expect(Parser* parser, TokenType type) {
    if (parser->current_token.type == type) {
        advance(parser);
    } else {
        fprintf(stderr, "Syntax error: Expected token type %d, got %d ('%s')\n", type, parser->current_token.type, parser->current_token.text);
        exit(1);
    }
}

void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    advance(parser);
}

// Forward declarations
static AstNode* parse_statement(Parser* parser);
static AstNode* parse_expression(Parser* parser);

static AstNode* parse_primary(Parser* parser) {
    if (parser->current_token.type == TOK_INT) {
        AstNode* node = ast_new_node(AST_INT_LIT);
        node->int_val = parser->current_token.value;
        advance(parser);
        return node;
    } else if (parser->current_token.type == TOK_ID) {
        AstNode* node = ast_new_node(AST_IDENTIFIER);
        strcpy(node->var_name, parser->current_token.text);
        advance(parser);
        return node;
    }
    fprintf(stderr, "Syntax error: Expected integer or identifier, got %d ('%s')\n", parser->current_token.type, parser->current_token.text);
    exit(1);
}

// Parse simple conditions for if (...)
static AstNode* parse_expression(Parser* parser) {
    AstNode* left = parse_primary(parser);
    
    TokenType op = parser->current_token.type;
    if (op == TOK_EQ || op == TOK_NEQ || op == TOK_LT || op == TOK_GT || op == TOK_LTE || op == TOK_GTE) {
        advance(parser); // consume operator
        AstNode* node = ast_new_node(AST_BINARY_OP);
        node->op = op;
        node->left = left;
        node->right = parse_primary(parser);
        return node;
    }
    
    return left; // no binary op
}

static AstNode* parse_block(Parser* parser) {
    AstNode* block = ast_new_node(AST_BLOCK);
    expect(parser, TOK_LBRACE);
    while (parser->current_token.type != TOK_RBRACE && parser->current_token.type != TOK_EOF) {
        ast_add_stmt(block, parse_statement(parser));
    }
    expect(parser, TOK_RBRACE);
    return block;
}

static AstNode* parse_if_statement(Parser* parser) {
    AstNode* node = ast_new_node(AST_IF_STMT);
    expect(parser, TOK_IF);
    expect(parser, TOK_LPAREN);
    node->condition = parse_expression(parser);
    expect(parser, TOK_RPAREN);
    
    node->true_branch = parse_block(parser);
    
    if (parser->current_token.type == TOK_ELSE) {
        advance(parser);
        if (parser->current_token.type == TOK_IF) {
            node->false_branch = parse_if_statement(parser); // else if
        } else {
            node->false_branch = parse_block(parser); // else
        }
    }
    
    return node;
}

static AstNode* parse_statement(Parser* parser) {
    if (parser->current_token.type == TOK_LET) {
        advance(parser);
        AstNode* node = ast_new_node(AST_VAR_DECL);
        
        if (parser->current_token.type == TOK_ID) {
            strcpy(node->var_name, parser->current_token.text);
            advance(parser);
        } else {
            fprintf(stderr, "Syntax error: Expected identifier after let\n");
            exit(1);
        }
        
        expect(parser, TOK_ASSIGN);
        
        node->left = parse_expression(parser); // store expression in left for var decl
        
        expect(parser, TOK_SEMI);
        return node;
        
    } else if (parser->current_token.type == TOK_IF) {
        return parse_if_statement(parser);
        
    } else if (parser->current_token.type == TOK_EXIT) {
        advance(parser);
        AstNode* node = ast_new_node(AST_EXIT_STMT);
        expect(parser, TOK_LPAREN);
        node->left = parse_expression(parser); // value to exit with
        expect(parser, TOK_RPAREN);
        expect(parser, TOK_SEMI);
        return node;
        
    } else {
        fprintf(stderr, "Syntax error: Unexpected token %d ('%s')\n", parser->current_token.type, parser->current_token.text);
        exit(1);
    }
}

AstNode* parse_program(Parser* parser) {
    AstNode* program = ast_new_node(AST_PROGRAM);
    while (parser->current_token.type != TOK_EOF) {
        ast_add_stmt(program, parse_statement(parser));
    }
    return program;
}
