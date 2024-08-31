#include "parser.h"
#include <stdio.h>

int get_lvar_size(Node *node);
void gen_load(Node *node);
void gen_store(Node *node);
void gen(Node *node);
void gen_lval(Node *node);
void codegen(Function *prog);
