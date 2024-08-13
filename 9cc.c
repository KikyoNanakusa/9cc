#include "tokenizer.h"
#include "utils.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Node type of AST
typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
} NodeKind;

typedef struct Node Node;

// Node type of AST
struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;    // val is used only if only kind is ND_NUM
};

Token *token;
char *user_input;

Node *primary();
Node *mul();
Node *expr();


// Create a new node
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;

  return node;
}

// Create a new node for number
Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}


Node *expr() {
  Node *node = mul();

  for(;;) {
    if (consume('+')) {
      node = new_node(ND_ADD, node, mul());
    }
    else if (consume('-')) {
      node = new_node(ND_SUB, node, mul());
    }
    else {
      return node;
    }
  }
}

Node *primary() {
  if (consume('(')) {
    Node *node = expr();
    expect(')');
    return node;
  }
  return new_node_num(expect_number());
}

Node *mul() {
  Node *node = primary();
  for(;;) {
    if (consume('*')) {
      node = new_node(ND_MUL, node, primary());
    }
    else if (consume('/')) {
      node = new_node(ND_DIV, node, primary());
    }
    else {
      return node;
    }
  }
}


// Generate assembly code
void gen(Node *node) {
  if(node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid number of arguments\n");
		return 1;
	}

  // Tokenize
  user_input = argv[1];
	token = tokenize(user_input);

  // Create AST
  Node *node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

  // Generate assembly code for the AST
  gen(node);

  printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}

