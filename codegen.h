#ifndef CODEGEN_H
#define CODEGEN_H

#include "ir.h"
#include <stdio.h>

void codegen_generate(IrContext* ir_ctx, FILE* out);

#endif
