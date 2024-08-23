#ifndef PARSER_H
#define PARSER_H

// Node type of AST
typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,
  ND_LVAR,
} NodeKind;

typedef struct Node Node;

extern Node *code[100];

// Node type of AST
struct Node {
  NodeKind kind;
  Node *next;
  Node *lhs;
  Node *rhs;
  int val;    // val is used only if only kind is ND_NUM
  int offset; // offset is used only if kind is ND_LVAR
};

void program();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

#endif // PARSER_H
