#include "fcc.h"

// nodeに対応する変数のアドレスをスタックトップにpushする
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR) {
    error("代入の左辺値が変数ではありません");
  }
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node);

void gen_stmts(Node *stmt) {
  Node *cur = stmt;
  while (cur) {
    gen(cur);
    printf("  pop rax\n");
    cur = cur->next;
  }
}

int xxx = 0;

void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      // スタックトップにある値をpopし、それをアドレスとみなしたときの
      // アドレスの値をスタックにpushする
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      if (node->lhs->kind == ND_DEREF) {
        gen(node->lhs->lhs);
      } else {
        gen_lval(node->lhs);
      }
      gen(node->rhs);
      // ここまでで、右辺の値, 左辺の変数のアドレスがスタックトップにある
      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_WHILE: {
      int id = xxx;
      xxx++;
      printf(".Lbegin%i:\n", id);
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%i\n", id);
      gen(node->rhs);
      printf("  jmp .Lbegin%i\n", id);
      printf(".Lend%i:\n", id);
      return;
    }
    case ND_IF: {
      int id = xxx;
      xxx++;
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lelse%i\n", id);
      gen(node->lhs);
      printf("  jmp .Lend%i\n", id);
      printf(".Lelse%i:\n", id);
      if (node->rhs) {
        gen(node->rhs);
      }
      printf(".Lend%i:\n", id);
      return;
    }
    case ND_FOR: {
      int id = xxx;
      xxx++;
      if (node->init) {
        gen(node->init);
      }
      printf(".Lbegin%i:\n", id);
      if (node->cond) {
        gen(node->cond);
      }
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%i\n", id);
      gen(node->lhs);
      if (node->update) {
        gen(node->update);
      }
      printf("  jmp .Lbegin%i\n", id);
      printf(".Lend%i:\n", id);
      return;
    }
    case ND_BLOCK: {
      gen_stmts(node->stmts);
      return;
    }
    case ND_CALL: {
      // よくわかってないのでrspを16の倍数に調整していない
      // 不都合が出るまでは放置
      Node *args = node->args;
      for (int i = 0; args != NULL && i < 6; ++i) {
        gen(args);
        if (i == 0) {
          printf("  pop rdi\n");
        } else if (i == 1) {
          printf("  pop rsi\n");
        } else if (i == 2) {
          printf("  pop rdx\n");
        } else if (i == 3) {
          printf("  pop rcx\n");
        } else if (i == 4) {
          printf("  pop r8\n");
        } else if (i == 5) {
          printf("  pop r9\n");
        }
        args = args->next;
      }
      printf("  call %.*s\n", node->fname_len, node->fname);
      printf("  push rax\n");
      return;
    }
    case ND_FUNC: {
      printf("%.*s:\n", node->fname_len, node->fname);

      // prologue
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      printf("  sub rsp, 208\n");

      Node *args = node->args;

      for (int i = 0; args != NULL && i < 6; i++) {
        gen_lval(args);
        if (i == 0) {
          printf("  push rdi\n");
        } else if (i == 1) {
          printf("  push rsi\n");
        } else if (i == 2) {
          printf("  push rdx\n");
        } else if (i == 3) {
          printf("  push rcx\n");
        } else if (i == 4) {
          printf("  push r8\n");
        } else if (i == 5) {
          printf("  push r9");
        }
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        args = args->next;
      }
      gen_stmts(node->stmts);
      printf("  pop rax\n");

      // epilogue

      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    }
    case ND_ADDR:
      gen_lval(node->lhs);
      return;
    case ND_DEREF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    default:
      break;
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
    case ND_NEQ:
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
    case ND_NUM:
      break;
  }
  printf("  push rax\n");
}

void codegen(Node **functions) {
  fprintf(stderr, "アセンブリ生成開始\n");
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  for (int i = 0; functions[i]; i++) {
    gen(functions[i]);
    printf("  pop rax\n");
  }
}