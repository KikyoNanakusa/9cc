#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
} TokenKind;

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

typedef struct Token Token;

struct Token {
	TokenKind kind;
	Token *next;
	int val;		// if kind is TK_NUM, its value	
	char *str;
};

char *user_input;

Token *token;

Node *primary();
Node *mul();
Node *expr();

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

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


// If the next token is the expected symbol, read one token and return true.
// Otherwise, return false.
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		return false;
	}
	token = token->next;
	return true;
}

// If the next token is the expected symbol, read one token.
// Otherwise, it returns an error.
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		error_at(token->str, "expected '%c'", op);
	}
	token = token->next;
}

// If the next token is a number, read one token and return the value.
// Otherwise, it returns an error.
int expect_number() {
	if (token->kind != TK_NUM) {
		error_at(token->str, "expected_numner");
	}
	int val = token->val;
	token = token->next;
	return val;
}

// If the next token is a EOF, it returns true.
// Otherwise, it returns false.
bool at_eof() {
	return token->kind == TK_EOF;
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

Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// Tokenize the input p and return it.
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// Skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

    error_at(p, "Invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
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

