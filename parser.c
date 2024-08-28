#include "parser.h"

// global variables
LVarList *locals = NULL;


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

LVar *push_lvar(char *name) {
  LVar *var = calloc(1, sizeof(LVar));
  var->name = name;

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

  LVarList *head = calloc(1, sizeof(LVarList));
  head->var = push_lvar(expect_ident());
  LVarList *cur = head;

  while(consume(")")) {
    expect(",");
    LVarList *param = calloc(1, sizeof(LVarList));
    param->var = push_lvar(expect_ident());
    cur->next = param;
    cur = cur->next;
  }

  return head;
}

Function *function() {
  locals = NULL;

  Function *fn = calloc(1, sizeof(Function));
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

    // if the token is a variable
    LVar *lvar = find_lvar(tok);

    if (!lvar) {
      lvar = push_lvar(strndup(tok->str, tok->len));
    }

    node->kind = ND_LVAR;
    node->var = lvar;
    return node;
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+")) {
    return unary();
  }

  if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), unary());
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
