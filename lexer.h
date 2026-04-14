#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOK_EOF,
    TOK_LET,
    TOK_IF,
    TOK_ELSE,
    TOK_EXIT,
    TOK_ID,
    TOK_INT,
    TOK_ASSIGN,     // =
    TOK_EQ,         // ==
    TOK_NEQ,        // !=
    TOK_LT,         // <
    TOK_GT,         // >
    TOK_LTE,        // <=
    TOK_GTE,        // >=
    TOK_SEMI,       // ;
    TOK_LPAREN,     // (
    TOK_RPAREN,     // )
    TOK_LBRACE,     // {
    TOK_RBRACE,     // }
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
    int value; // Used if type == TOK_INT
} Token;

typedef struct {
    const char* source;
    int pos;
    int length;
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
Token get_next_token(Lexer* lexer);

#endif
