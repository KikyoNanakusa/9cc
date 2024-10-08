#include "codegen.h"
#include "utils.h"

int labelseq = 0;
char *argreg_8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *argreg_4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
char *argreg_1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *funcname;

// To do pointer arithmetic, we need to scale the value to be added.
// by the size of the pointer type.
// e.g. int *p; p + 1 should be p + 4, not p + 1.
static void ajust_pointer_arithmetic(Node *lhs, Node *rhs) {
  Type *ptr_type = NULL;
  bool is_lhs_pointer = false;

  if (lhs->kind == ND_LVAR && is_ptr(lhs)) {
    ptr_type = lhs->var->type->ptr_to;  
    is_lhs_pointer = true;
  } else if (lhs->kind == ND_ADDR) {
    ptr_type = lhs->lhs->var->type;  
    is_lhs_pointer = true;
  } else if (lhs->kind == ND_LITERAL) {
    ptr_type = type_char();
    is_lhs_pointer = true;
  }

  // if lhs is not a pointer, try rhs
  if (ptr_type == NULL) {  
    if (rhs->kind == ND_LVAR && is_ptr(rhs)) {
      ptr_type = rhs->var->type->ptr_to;  
      is_lhs_pointer = false;
    } else if (rhs->kind == ND_ADDR) {
      ptr_type = rhs->lhs->var->type;  
      is_lhs_pointer = false;
    } else if (rhs->kind == ND_LITERAL) {
      ptr_type = type_char();
      is_lhs_pointer = false;
    }
  }

  if (ptr_type == NULL || ptr_type->size == 0) {
    error("Invalid pointer type for pointer addition.");
  }

  if (is_lhs_pointer) {
    printf("  imul rdi, %d\n", ptr_type->size);
  } else {
    printf("  imul rax, %d\n", ptr_type->size);
  }
}

void gen_load(Node *node) {
  int size = get_size(node);

  printf("  pop rax\n");
  if (size == 1) {
    printf("  movsx rax, byte ptr [rax]\n");
  } else if (size == 4) {
    printf("  movsxd rax, dword ptr [rax]\n");
  } else {
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
}

void gen_store(Node *node) {
  int size = get_size(node);

  if (size == 1) {
    printf("  mov [rax], dil\n");
  } else if (size == 4) {
    printf("  mov [rax], edi\n");
  } else {
    printf("  mov [rax], rdi\n");
  }
}

void gen_lval(Node *node) {
  if (node->kind == ND_DEREF) {
    gen(node->lhs);
  } else if (node->kind == ND_LVAR) {
    // global variable
    if (node->var->is_global) {
      gen_glval(node);
      return;
    }
    
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->var->offset);
    printf("  push rax\n");
  } else {
    error("Left values is not a variable: %d", node->kind);
  }
}

void gen_glval(Node *node) {
  // RIP-relative addressing
  printf("  lea rax, [rip + %s]\n", node->var->name);
  printf("  push rax\n");
  return;
}

void gen_init_list(Node *node) {
  if (node->rhs->init_list) {
    Node *init_list_node = node->rhs->init_list;
    int offset = 0;
    int size = node->lhs->var->type->ptr_to->size;

    for (Node *element = init_list_node; element; element = element->next) {
      gen_lval(node->lhs);
      printf("  pop rax\n");
      printf("  add rax, %d\n", offset);
      printf("  push rax\n");

      gen(element);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      gen_store(node->lhs);

      offset += size;
    }
  } else {
    error("cannot initialize array");
  }
}

// Generate assembly code
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      if (!is_array(node)) {
        gen_load(node);
      }
      return;
    case ND_ASSIGN:
      // list initializer (int a[3] = {1, 2, 3})
      if (node->rhs->kind == ND_INIT_LIST) {
        gen_init_list(node);
        return;
      }

      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      gen_store(node->lhs);
      printf("  push rdi\n");
      return;
    case ND_IF:
      int seq = labelseq++;
      if(node->els) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lelse%d\n", seq);
        gen(node->then);
        printf("  jmp .Lend%d\n", seq);
        printf(".Lelse%d:\n", seq);
        gen(node->els);
        printf(".Lend%d:\n", seq);
      } else {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", seq);
        gen(node->then);
        printf(".Lend%d:\n", seq);
      }
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  jmp .L.return.%s\n", funcname);
      return;
    case ND_WHILE:
      seq = labelseq++;
      printf(".Lbegin%d:\n", seq);
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%d\n", seq);
      gen(node->then);
      printf("  jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    case ND_FOR:
      seq = labelseq++; 
      if (node->init) {
        gen(node->init);
      }
      printf(".Lbegin%d:\n", seq);
      if (node->cond) {
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", seq);
      }
      gen(node->then);
      if (node->inc) {
        gen(node->inc);
      }
      printf("  jmp .Lbegin%d\n", seq);
      printf(".Lend%d:\n", seq);
      return;
    case ND_BLOCK:
      for (Node *n = node->body; n; n = n->next) {
        gen(n);
      }
      return;
    case ND_FUNCALL: {
      int nargs = 0;
      for (Node *arg = node->args; arg; arg = arg->next) {
        gen(arg);
        nargs++;
      }

      for (int i = nargs-1; i >= 0; i--) {
        printf("  pop %s\n", argreg_8[i]);
      }

      //check stack pointer is on 16bytes alignment
      int seq = labelseq++; 

      printf("  mov rax, rsp\n");
      printf("  and rax, 16\n");
      printf("  jnz .Lajust_rsp%d\n", seq);
      printf("  xor rax, rax\n");
      printf("  call %s\n", node->funcname);
      printf("  jmp .Lajust_rsp_end%d\n", seq);

      printf(".Lajust_rsp%d:\n", seq);
      printf("  sub rsp, 8\n");
      printf("  xor rax, rax\n");
      printf("  call %s\n", node->funcname);
      printf("  add rsp, 8\n");
      printf(".Lajust_rsp_end%d:\n", seq);
      printf("  push rax\n");
      return;
    }
    case ND_ADDR: {
      gen_lval(node->lhs);
      return;
    }

    case ND_DEREF: {
      gen(node->lhs);
      if (!is_array(node->lhs)) {
        gen_load(node->lhs);
      }
      return;
    }
    case ND_LITERAL: {
      printf("  lea rax, [rip+.LC%d]\n", node->literal->labelseq);
      printf("  push rax\n");
      return;
    }
    case ND_NULL: {
      return;
    }
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
    case ND_PTR_ADD: {
      ajust_pointer_arithmetic(node->lhs, node->rhs);

      printf("  add rax, rdi\n");
      break;
    }
    case ND_PTR_SUB: {
      ajust_pointer_arithmetic(node->lhs, node->rhs);

      printf("  sub rax, rdi\n");
      break;
    }
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void gen_func(Function *fn) {
  printf("  .globl %s\n", fn->name);
  printf("  .text\n");
  printf("%s:\n", fn->name);
  funcname = fn->name;

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", fn->stack_size);

  int i = 0;
  for (LVarList *varList = fn->params; varList; varList = varList->next) {
    LVar *var = varList->var;
    int size = var->type->size;
    if (size == 1) {
      printf("  mov [rbp-%d], %s\n", var->offset, argreg_1[i++]);
    } else if (size == 4) {
      printf("  mov [rbp-%d], %s\n", var->offset, argreg_4[i++]);
    } else {
      printf("  mov [rbp-%d], %s\n", var->offset, argreg_8[i++]);
    }
  }

  for (Node *node = fn->node; node; node = node->next) {
    gen(node);
  }

  // epilogue
  printf(".L.return.%s:\n", fn->name);
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen_gvar(Node *node) {
  printf("  .data\n");
  printf("  .globl %s\n", node->var->name);
  printf("%s:\n", node->var->name);

  if (node->list_element) {
    ListElement *element = node->list_element;
    while(element) {
      if(element->kind == LE_NUM) {
        printf("  .long %d\n", element->val);
      } else if(element->kind == LE_CHAR) {
        printf("  .byte %d\n", element->val);
      } else if(element->kind == LE_STRING) {
        printf("  .quad .LC%d\n", element->literal->labelseq);
      }
      element = element->next;
    }
    
    return;
  }

  if (node->init_literal) {
    printf("  .quad .LC%d\n", node->init_literal->labelseq);
    return;
  }

  if (node->init_val != 0) {
    if (node->var->type->size == 1) {
      printf("  .byte %d\n", node->init_val);
    } else if (node->var->type->size == 8) {
      printf("  .quad %d\n", node->init_val);
    } else {
      printf("  .long %d\n", node->init_val);
    }
  } else {
    printf("  .zero %d\n", node->var->type->size);
  }
}

void encode_literal(Literal *literal) {
  printf("  .data\n");
  printf(".LC%d:\n", labelseq);
  for(int i = 0; i < literal->len; i++) {
    printf("  .byte %d\n", literal->string[i]);
  }
  printf("  .byte 0\n");
  literal->labelseq = labelseq;
  labelseq++;
}

void codegen(Program *program) {
  printf(".intel_syntax noprefix\n");

  // encode all literal first in data segment
  for (LiteralList *literal = literals; literal; literal = literal->next) {
    encode_literal(literal->literal);  
  }

  for (Program *prog = program; prog; prog = prog->next) {
    if(prog->gvar) {
      gen_gvar(prog->gvar);
      continue; 
    }
  }

  for (Program *prog = program; prog; prog = prog->next) {
    if (prog->func) {
      gen_func(prog->func);
      continue;
    }
  }
}
