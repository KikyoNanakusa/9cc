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
  program();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  // Generate assembly code for the AST
  for(int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}

