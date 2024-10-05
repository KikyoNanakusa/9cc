#include "parser.h"
#include <stdio.h>

void gen_glval(Node *node);
void gen_load(Node *node);
void gen_store(Node *node);
void gen(Node *node);
void gen_lval(Node *node);
void codegen(Program *program);
