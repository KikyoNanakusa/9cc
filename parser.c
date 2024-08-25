#include "parser.h"
#include "tokenizer.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


// global variables
Node *code[100];
LVar *locals = NULL;

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

// Find a declared local variable
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }

  return NULL;
}


Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }

  return node;
}

Node *expr() {
  return assign();
}


Node *stmt() {
  Node *node;

  if (consume_return()) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();

    if (!consume(";")) {
      error("';' is expected");
    }
    return node;
  }
    
  if (consume("if")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume("else")) {
      node->els = stmt();
    }
    return node;
  }

  if (consume("while")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;

    expect("(");
    node->cond = expr();
    expect(")");
    node->then = stmt();

    return node;
  }

  if (consume("for")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;

    expect("(");
    if(!consume(";")) {
      node->init = expr();
      expect(";");
    } else {
      node->init = NULL;
    }

    if (!consume(";")) {
      node->cond = expr(); 
      expect(";");
    } else {
      node->cond = NULL;
    }

    if (!consume(")")) {
      node->inc = expr();
      expect(")");
    } else {
      node->inc = NULL;
    }
    
    node->then = stmt();
    return node;
  }

  node = expr();

  if (!consume(";")) {
    error("';' is expected");
  }
   
  return node;
}

void program() {
  int i = 0;
  while(!at_eof()) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

Node *equality() {
  Node *node = relational();
  for(;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    }
    else if (consume("!=")) {
      node = new_node(ND_NE, node, relational());
    }
    else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for(;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for(;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    }
    else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    }
    else {
      return node;
    }
  }
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);

    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;

      if (locals == NULL) {
        lvar->offset = 8;
      } else {
        lvar->offset = locals->offset + 8;
      }

      node->offset = lvar->offset;
      locals = lvar;
    }

    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+")) {
    return primary();
  }

  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }

  return primary();
}

Node *mul() {
  Node *node = unary();
  for(;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    }
    else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    }
    else {
      return node;
    }
  }
}
