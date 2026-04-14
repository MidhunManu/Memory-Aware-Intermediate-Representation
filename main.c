#include "lexer.h"
#include "parser.h"
#include "ir.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>

char* read_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Could not open file %s\n", filepath);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.lang>\n", argv[0]);
        return 1;
    }

    const char* filepath = argv[1];
    char* source = read_file(filepath);

    Lexer lexer;
    lexer_init(&lexer, source);

    Parser parser;
    parser_init(&parser, &lexer);

    AstNode* program = parse_program(&parser);

    IrContext ir_ctx;
    ir_context_init(&ir_ctx);
    ir_generate(&ir_ctx, program);

    FILE* out = fopen("out.asm", "w");
    if (!out) {
        fprintf(stderr, "Could not create out.asm\n");
        exit(1);
    }

    codegen_generate(&ir_ctx, out);
    fclose(out);

    printf("Compiled %s to out.asm successfully.\n", filepath);

    free(source);
    // A production compiler would thoroughly free the AST and IR structures here.
    return 0;
}
