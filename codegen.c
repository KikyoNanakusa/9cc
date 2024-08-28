#include "codegen.h"
#include "utils.h"

int labelseq = 0;
char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *funcname;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("Left values is not a variable");
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->var->offset);
  printf("  push rax\n");
}

// Generate assembly code
void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
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
        printf("  pop %s\n", argreg[i]);
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

void codegen(Function *prog) {
  printf(".intel_syntax noprefix\n");

  for (Function *fn = prog; fn; fn = fn->next) {
    printf(".global %s\n", fn->name);
    printf("%s:\n", fn->name);
    funcname = fn->name;

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", fn->stack_size);

    for (Node *node = fn->node; node; node = node->next) {
      gen(node);
    }

    // epilogue
    printf(".L.return.%s:\n", fn->name);
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }
}
