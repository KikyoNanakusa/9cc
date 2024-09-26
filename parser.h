#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "utils.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>


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
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_FUNCALL,
  ND_ADDR,
  ND_DEREF,
  ND_NULL,
  ND_PTR_ADD,
  ND_PTR_SUB,
} NodeKind;


typedef struct Node Node;

// Local variable
typedef struct LVar LVar;
struct LVar {
  char *name;
  Type *type;
  int offset; // offset from rbp
};

typedef struct LVarList LVarList;
struct LVarList {
  LVarList *next;
  LVar *var;
};

// Node type of AST
struct Node {
  NodeKind kind;
  Node *next;
  Node *lhs;
  Node *rhs;
  int val;    // val is used only if only kind is ND_NUM
  LVar *var;  // offset is used only if kind is ND_LVAR
              //
  Node *cond; // used only if kind is ND_IF or ND_WHILE
  Node *then; // used only if kind is ND_IF or ND_WHILE
  Node *els;  // used only if kind is ND_IF
              //
  Node *init; // user only if kind is ND_FOR
  Node *inc;  // user only if kind is ND_FOR
              //
  Node *body; // used only if kind is ND_BLOCK
              //
  char *funcname; // used only if kind is ND_FUNCALL
  Node *args;     // used only if kind is ND_FUNCALL
};

typedef struct Function Function;
struct Function  {
  Node *node;
  char *name;
  Function *next;
  LVarList *locals;
  int stack_size;
  LVarList *params;
};

Function *program();
Function *function();
Type *basetype();
Node *declaration(Type *type);
Node *func_args();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

bool is_ptr(Node *node);
bool is_array(Node *node);
Node *parse_deref(Node *node);
int get_size(Node *node);
#endif // PARSER_H
