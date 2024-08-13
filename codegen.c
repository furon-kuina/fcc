#include "fcc.h"

// nodeに対応する変数のアドレスをスタックトップにpushする
void gen_lval(Node* node) {
  if (node->kind != ND_LVAR && node->kind != ND_GVAR) {
    error("代入の左辺値が変数ではありません");
  }
  switch (node->kind) {
    case ND_LVAR:
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", node->offset);
      printf("  push rax\n");
      break;
    case ND_GVAR:
      printf("  lea rax, %.*s[rip]\n", node->len, node->name);
      printf("  push rax\n");
      break;
    default:
      error("代入の左辺値が変数ではありません");
  }
}

void gen(Node* node);

void gen_stmts(Node* stmt) {
  Node* cur = stmt;
  while (cur) {
    gen(cur);
    printf("  pop rax\n");
    cur = cur->next;
  }
}

int xxx = 0;

char* node_kind_str(NodeKind kind) {
  switch (kind) {
    case ND_ADD:
      return "ND_ADD";
    case ND_SUB:
      return "ND_SUB";
    case ND_MUL:
      return "ND_MUL";
    case ND_DIV:
      return "ND_DIV";
    case ND_LT:
      return "ND_LT";
    case ND_LE:
      return "ND_LE";
    case ND_EQ:
      return "ND_EQ";
    case ND_NEQ:
      return "ND_NEQ";
    case ND_NUM:
      return "ND_NUM";
    case ND_ASSIGN:
      return "ND_ASSIGN";
    case ND_LVAR:
      return "ND_LVAR";
    case ND_RETURN:
      return "ND_RETURN";
    case ND_WHILE:
      return "ND_WHILE";
    case ND_IF:
      return "ND_IF";
    case ND_FOR:
      return "ND_FOR";
    case ND_BLOCK:
      return "ND_BLOCK";
    case ND_CALL:
      return "ND_CALL";
    case ND_FUNC:
      return "ND_FUNC";
    case ND_ADDR:
      return "ND_ADDR";
    case ND_DEREF:
      return "ND_DEREF";
    case ND_GVAR_DEF:
      return "ND_GVAR_DEF";
    case ND_GVAR:
      return "ND_GVAR";
    case ND_CHAR:
      return "ND_CHAR";
  }
}

void print_gen_start(Node* node) {
  printf("# start generating node #%i of kind %s\n", node->id,
         node_kind_str(node->kind));
}

void print_gen_end(Node* node) {
  printf("# finish generating node #%i of kind %s\n", node->id,
         node_kind_str(node->kind));
}

void gen(Node* node) {
  print_gen_start(node);
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      print_gen_end(node);
      return;
    case ND_LVAR:
      // nodeに対応する変数のアドレスをスタックトップにpushする
      gen_lval(node);
      // スタックトップにある値をpopし、それをアドレスとみなしたときの
      // アドレスの値をスタックにpushする
      if (node->type->ty != ARRAY) {
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
      }
      print_gen_end(node);
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
      print_gen_end(node);
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      print_gen_end(node);
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
      print_gen_end(node);
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
      print_gen_end(node);
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
      print_gen_end(node);
      return;
    }
    case ND_BLOCK: {
      gen_stmts(node->stmts);
      print_gen_end(node);
      return;
    }
    case ND_CALL: {
      Node* args = node->args;
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
      int id = xxx;
      xxx++;
      printf("  mov rax, rsp\n");
      printf("  and rax, 15\n");
      printf("  jnz .Lcall%d\n", id);
      printf("  mov rax, 0\n");
      printf("  call %.*s\n", node->len, node->name);
      printf("  jmp .Lend%d\n", id);
      printf(".Lcall%d:\n", id);
      printf("  sub rsp, 8\n");
      printf("  mov rax, 0\n");
      printf("  call %.*s\n", node->len, node->name);
      printf("  add rsp, 8\n");
      printf(".Lend%d:\n", id);
      printf("  push rax\n");
      print_gen_end(node);
      return;
    }
    case ND_FUNC: {
      printf("%.*s:\n", node->len, node->name);

      // prologue
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      printf("  sub rsp, %i\n", node->sf_size);

      Node* args = node->args;

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
      print_gen_end(node);
      return;
    }
    case ND_ADDR:
      gen_lval(node->lhs);
      print_gen_end(node);
      return;
    case ND_DEREF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      print_gen_end(node);
      return;
    case ND_GVAR_DEF:
      printf("  .comm %.*s, %i\n", node->len, node->name, node->offset);
      print_gen_end(node);
      return;
    case ND_GVAR:
      // nodeに対応する変数のアドレスをスタックトップにpushする
      gen_lval(node);
      // スタックトップにある値をpopし、それをアドレスとみなしたときの
      // アドレスの値をスタックにpushする
      if (node->type->ty != ARRAY) {
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
      }
      print_gen_end(node);
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
      // assuming that at least one of the operands is of int type
      if (node->lhs->kind == ND_LVAR && node->lhs->type->ty != INT) {
        if (node->lhs->type->ptr_to->ty == INT) {
          printf("  imul rdi, 4\n");
        } else {
          printf("  imul rdi, 8\n");
        }
      } else if (node->rhs->kind == ND_LVAR && node->rhs->type->ty != INT) {
        if (node->rhs->type->ptr_to->ty == INT) {
          printf("  imul rax, 4\n");
        } else {
          printf("  imul rax, 8\n");
        }
      }
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
  print_gen_end(node);
}

void codegen(Node** definitions) {
  fprintf(stderr, "アセンブリ生成開始\n");
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");

  for (int i = 0; definitions[i]; i++) {
    gen(definitions[i]);
    if (definitions[i]->kind == ND_FUNC) {
      printf("  pop rax\n");
    }
  }
}