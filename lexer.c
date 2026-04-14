#include "lexer.h"
#include <ctype.h>
#include <string.h>

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->length = strlen(source);
}

static char peek_char(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->source[lexer->pos];
}

static char next_char(Lexer* lexer) {
    if (lexer->pos >= lexer->length) return '\0';
    return lexer->source[lexer->pos++];
}

static void skip_whitespace(Lexer* lexer) {
    while (isspace(peek_char(lexer))) {
        next_char(lexer);
    }
}

Token get_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    Token token;
    memset(&token, 0, sizeof(Token));
    
    char c = peek_char(lexer);
    if (c == '\0') {
        token.type = TOK_EOF;
        return token;
    }
    
    // Identifiers and keywords
    if (isalpha(c) || c == '_') {
        int i = 0;
        while (isalnum(peek_char(lexer)) || peek_char(lexer) == '_') {
            if (i < 63) token.text[i++] = next_char(lexer);
            else next_char(lexer);
        }
        token.text[i] = '\0';
        
        if (strcmp(token.text, "let") == 0) token.type = TOK_LET;
        else if (strcmp(token.text, "if") == 0) token.type = TOK_IF;
        else if (strcmp(token.text, "else") == 0) token.type = TOK_ELSE;
        else if (strcmp(token.text, "exit") == 0) token.type = TOK_EXIT;
        else token.type = TOK_ID;
        
        return token;
    }
    
    // Integers
    if (isdigit(c)) {
        int val = 0;
        int i = 0;
        while (isdigit(peek_char(lexer))) {
            char d = next_char(lexer);
            if (i < 63) token.text[i++] = d;
            val = val * 10 + (d - '0');
        }
        token.text[i] = '\0';
        token.value = val;
        token.type = TOK_INT;
        return token;
    }
    
    // Operators and symbols
    c = next_char(lexer);
    token.text[0] = c;
    token.text[1] = '\0';
    
    switch (c) {
        case '=':
            if (peek_char(lexer) == '=') {
                next_char(lexer);
                strcpy(token.text, "==");
                token.type = TOK_EQ;
            } else {
                token.type = TOK_ASSIGN;
            }
            break;
        case '!':
            if (peek_char(lexer) == '=') {
                next_char(lexer);
                strcpy(token.text, "!=");
                token.type = TOK_NEQ;
            } else {
                token.type = TOK_ERROR;
            }
            break;
        case '<':
            if (peek_char(lexer) == '=') {
                next_char(lexer);
                strcpy(token.text, "<=");
                token.type = TOK_LTE;
            } else {
                token.type = TOK_LT;
            }
            break;
        case '>':
            if (peek_char(lexer) == '=') {
                next_char(lexer);
                strcpy(token.text, ">=");
                token.type = TOK_GTE;
            } else {
                token.type = TOK_GT;
            }
            break;
        case ';': token.type = TOK_SEMI; break;
        case '(': token.type = TOK_LPAREN; break;
        case ')': token.type = TOK_RPAREN; break;
        case '{': token.type = TOK_LBRACE; break;
        case '}': token.type = TOK_RBRACE; break;
        default: token.type = TOK_ERROR; break;
    }
    
    return token;
}
