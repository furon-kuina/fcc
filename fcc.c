#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *user_input;

typedef enum {
  TK_RESERVED,  // 記号
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // トークンの種類
  Token *next;     // 次の入力トークン
  int val;         // TK_NUMの数値
  char *str;       // トークン文字列
  int len;         // トークン長
};                 // Tokenの連結リストにトークナイズする

Token *token;  // 現在注目してるトークン

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool at_eof() { return token->kind == TK_EOF; }

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

    error_at(token->str, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// 次のトークンがopの場合は読み進めてtrueを返す
// そうでない場合はfalseを返す
// 演算子にだけ使う
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンがopの場合は読み進める
// そうでない場合はerrorを呼ぶ
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%C'ではありません", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// expr       = equality
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = primary ("*" primary | "/" primary)*
// primary    = num | "(" expr ")"

typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_LT,   // <
  ND_GT,   // >
  ND_LE,   // <=
  ND_GE,   // >=
  ND_EQ,   // ==
  ND_NEQ,  // !=
  ND_NUM,  // 整数
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // ノードの種類
  Node *lhs;      // 左オペランド
  Node *rhs;      // 右オペランド
  int val;        // kind == ND_NUMのときのみ
};

int node_cnt = 1;

char *node_dbg(Node *node) {
  switch (node->kind) {
    case ND_ADD:
      return "+";
    case ND_SUB:
      return "-";
    case ND_MUL:
      return "*";
    case ND_DIV:
      return "/";
    case ND_LT:
      return "<";
    case ND_GT:
      return ">";
    case ND_LE:
      return "<=";
    case ND_GE:
      return ">=";
    case ND_EQ:
      return "==";
    case ND_NEQ:
      return "!=";
    case ND_NUM:
      return "Number";
  }
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  fprintf(stderr, "ノード%d\n", node_cnt++);
  fprintf(stderr, "ノードの種類: %s\n", node_dbg(node));
  fprintf(stderr, "左の子: %s, 右の子: %s\n", node_dbg(node->lhs),
          node_dbg(node->rhs));
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  fprintf(stderr, "ノード%d\n", node_cnt++);
  fprintf(stderr, "ノードの種類: %s, 値%d\n", node_dbg(node), node->val);
  return node;
}

Node *mul();
Node *primary();
Node *equality();
Node *relational();
Node *add();

Node *expr() {
  Node *node = equality();
  return node;
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume(">")) {
      node = new_node(ND_GT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_GE, node, add());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = primary();
  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, primary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, primary());
    } else {
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
  return new_node_num(expect_number());
}

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
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
  }
  printf("  push rax\n");
}

int main(int argc, char **argv) {
  fprintf(stderr, "\n");
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  user_input = argv[1];
  fprintf(stderr, "入力を受け取りました: %s\n\n", user_input);
  token = tokenize(user_input);
  fprintf(stderr, "\nトークナイズ終了\n\n");

  Node *node = expr();

  fprintf(stderr, "パースしました\n");

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}