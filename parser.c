#include "parser.h"

// global variables
LVarList *locals = NULL;

// Parse the dereference node
Node *parse_deref(Node *node) {
  if (node->kind == ND_DEREF) {
    parse_deref(node->lhs);
  }
  return node;
}

bool is_ptr(Node *node) {
  return node->var->type->kind == TY_PTR || node->var->type->kind == TY_ARRAY;
}

bool is_array(Node *node) {
  if (!node->var) {
    return false;
  }

  return node->var->type->kind == TY_ARRAY;
}

// get the size of the node
int get_size(Node *node) {
  // At this time, ND_NUM is the only node that has a size of 4.
  if (node->kind == ND_NUM) {
    return 4;
  }

  if (node->kind == ND_ADDR) {
    return 8;
  }

  if (node->kind == ND_DEREF) {
    if (node->lhs->kind == ND_PTR_ADD || node->lhs->kind == ND_PTR_SUB) {
      if (is_array(node->lhs->lhs)) {
        return node->lhs->lhs->var->type->ptr_to->size;
      }

      return get_size(node->lhs);
    } else if (node->lhs->var->type->kind == TY_PTR) {
      return node->lhs->var->type->ptr_to->size;
    } else {
      return get_size(node->lhs);
    }
  }

  if (node->kind == ND_PTR_ADD || node->kind == ND_PTR_SUB) {
    return get_size(node->lhs);
  }

  if (node->kind == ND_LVAR && node->var->type->kind == TY_ARRAY) {
    return node->var->type->array_size * node->var->type->ptr_to->size;
  }

  return node->var->type->size;
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

// Find a declared local variable
LVar *find_lvar(Token *tok) {
  for (LVarList *varList = locals; varList; varList = varList->next) {
    LVar *var = varList->var;
    if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len)) {
      return var;
    }
  }

  return NULL;
}

LVar *push_lvar(char *name, Type *type) {
  LVar *var = calloc(1, sizeof(LVar));
  var->name = name;
  var->type = type;

  LVarList *varList = calloc(1, sizeof(LVarList));
  varList->var = var;
  varList->next = locals;
  locals = varList;

  return var;
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

Node *declaration(Type *type) {
  char *name = expect_ident();
  if (consume("[")) {
    int array_size = expect_number();
    expect("]");
    type = array_of(type, array_size);
  }

  LVar *var = push_lvar(name, type);

  if (consume(";")) {
    Node *null_node = calloc(1, sizeof(Node));
    null_node->kind = ND_NULL;
    return null_node;
  }

  expect("=");

  Node *lhs = calloc(1, sizeof(Node));
  lhs->kind = ND_LVAR;
  lhs->var = var;

  Node *rhs = expr();

  expect(";");
  Node *node = new_node(ND_ASSIGN, lhs, rhs);
  return node;
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

  if (consume("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while(!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }

    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->body = head.next;
    return node;
  }

  if (consume("int")) {
    Type *type = type_int();
    while (consume("*")) {
      type = pointer_to(type);
    }
    return declaration(type);
  }

  node = expr();

  if (!consume(";")) {
    error("';' is expected");
  }
   
  return node;
}

Function *program() {
  Function head;
  head.next = NULL;
  Function *cur = &head;

  while (!at_eof()) {
    cur->next = function();
    cur = cur->next;
  }
  
  return head.next;
}

LVarList *read_func_params() {
  if (consume(")")) {
    return NULL;
  }

  expect("int");
  LVarList *head = calloc(1, sizeof(LVarList));
  head->var = push_lvar(expect_ident(), type_int());
  LVarList *cur = head;

  while(!consume(")")) {
    expect(",");
    expect("int");
    LVarList *param = calloc(1, sizeof(LVarList));

    param->var = push_lvar(expect_ident(), type_int());
    cur->next = param;
    cur = cur->next;
  }

  return head;
}

Function *function() {
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));

  // TODO: impelement type
  expect("int");

  fn->name = expect_ident();
  expect("(");
  fn->params = read_func_params();
  expect("{");

  Node head;
  head.next = NULL;
  Node *cur = &head;

  while(!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }

  fn->node = head.next;
  fn->locals = locals;

  return fn;
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

bool is_calc_ptr(Node *lhs, Node *rhs) {
  return (
          (lhs->kind == ND_LVAR && lhs->var->type->kind == TY_PTR) ||
          (lhs->kind == ND_LVAR && lhs->var->type->kind == TY_ARRAY) ||
          lhs->kind == ND_ADDR ||
          (rhs->kind == ND_LVAR && rhs->var->type->kind == TY_PTR) ||
          (rhs->kind == ND_LVAR && rhs->var->type->kind == TY_ARRAY) ||
          rhs->kind == ND_ADDR);
}

Node *add() {
  Node *node = mul();

  for(;;) {
    if (consume("+")) {
      Node *rhs = mul();
      if (is_calc_ptr(node, rhs)) {
        node = new_node(ND_PTR_ADD, node, rhs);
      } else {
        node = new_node(ND_ADD, node, rhs);
      }
    } else if (consume("-")) {
      Node *rhs = mul();
      if (is_calc_ptr(node, rhs)) {
        node = new_node(ND_PTR_SUB, node, rhs);
      } else {
        node = new_node(ND_SUB, node, rhs);
      }
    }
    else {
      return node;
    }
  }
}

Node *func_args() {
  if (consume(")")) {
    return NULL;
  }
  Node *head = assign();
  Node *cur = head;
  while(consume(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
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
    
    // if the token is a no argument function call
    if (consume("(")) {
      node->kind = ND_FUNCALL;
      node->funcname = strndup(tok->str, tok->len);
      node->args = func_args();
      return node;
    }

    LVar *lvar = find_lvar(tok);

    if (!lvar) {
      error("undefined variable");
    }
    
    if (consume("[")) {
      Node *index = expr();
      node->kind = ND_LVAR;
      node->var = lvar;
      Node *ptr_add = new_node(ND_PTR_ADD, node, index);
      node = new_node(ND_DEREF, ptr_add, NULL);
      expect("]");
      return node;
    }

    node->kind = ND_LVAR;
    node->var = lvar;
    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume_sizeof()) {
    Node *node = unary();
    return new_node_num(get_size(node));
  }

  if (consume("+")) {
    return unary();
  }

  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), unary());
  }

  if (consume("&")) {
    return new_node(ND_ADDR, unary(), NULL);
  }

  if (consume("*")) {
    return new_node(ND_DEREF, unary(), NULL);
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
