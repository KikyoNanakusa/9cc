#ifndef PARSER_H
#define PARSER_H

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

Node *expr();
Node *mul();
Node *primary();
Node *unary();

#endif // PARSER_H
