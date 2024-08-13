#include "parser.h"
#include "tokenizer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

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
