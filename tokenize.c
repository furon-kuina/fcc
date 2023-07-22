#include "fcc.h"

static char *current_input;

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

int token_cnt = 1;

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  fprintf(stderr, "\nトークン%d\n", token_cnt++);
  fprintf(stderr, "トークンの種類: %d\n", kind);

  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  tok->len = len;
  return tok;
}

Token *tokenize(char *p) {
  fprintf(stderr, "トークナイズ開始\n");
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字は読み飛ばす
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) || !strncmp(p, "!=", 2) ||
        !strncmp(p, "==", 2)) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '>' || *p == '<') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}