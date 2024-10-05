#include "tokenizer.h"
#include "utils.h"
#include "parser.h"
#include "codegen.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Token *token;
char *user_input;


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid number of arguments\n");
		return 1;
	}

  // Tokenize
  user_input = argv[1];
	token = tokenize(user_input);

  // Create AST
  Program *prog = program();
  for (Program *pg = prog; pg; pg = pg->next) {
    if (pg->func) {
      Function *fn = pg->func;
      int offset = 0;
      for (LVarList *varList = fn->locals; varList; varList = varList->next) {
        offset += varList->var->type->size;
        varList->var->offset = offset;
      }
      fn->stack_size = offset;
    }
  }

  codegen(prog);

	return 0;
}

