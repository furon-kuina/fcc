#include "fcc.h"

// グローバル変数

Token *token;      // パーサに入力として与えられるトークン列
LVar *locals;      // ローカル変数を表す連結リスト
Node *code[100];   // ";"で区切られたコード
int node_cnt = 0;  // デバッグ用: ノードの数

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

bool expect_ident() {
  if (token->kind != TK_IDENT) return NULL;
  return token->str[0];
}

// 次のトークンがopの場合は読み進める
// そうでない場合はerror
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

bool at_eof() { return token->kind == TK_EOF; }

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->id = node_cnt++;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->id = node_cnt++;
  return node;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

Node *stmt();
Node *expr();
Node *assign();
Node *mul();
Node *primary();
Node *equality();
Node *relational();
Node *add();
Node *unary();

void program() {
  int i = 0;
  while (!at_eof()) {
    code[i] = stmt();
    i++;
  }
  code[i] = NULL;
}

Node *stmt() {
  Node *node;
  if (consume("{")) {
    Stmt head;
    head.next = NULL;
    Stmt *cur = &head;
    while (!consume("}")) {
      Stmt *new = calloc(1, sizeof(Stmt));
      new->node = stmt();
      cur->next = new;
      cur = cur->next;
    }
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = head.next;
  } else if (token->kind == TK_RETURN) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (token->kind == TK_WHILE) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_WHILE;
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
  } else if (token->kind == TK_IF) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->lhs = stmt();
    if (token->kind == TK_ELSE) {
      token = token->next;
      node->rhs = stmt();
    }
  } else if (token->kind == TK_FOR) {
    token = token->next;
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_FOR;
    expect("(");
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->update = expr();
      expect(")");
    }
    node->lhs = stmt();

  } else {
    node = expr();
    expect(";");
  }
  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
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
      node = new_node(ND_LT, add(), node);
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
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
  Node *node = unary();
  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
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

// primary = num | ident | "(" expr ")"

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  if (token->kind == TK_IDENT) {
    Node *node = calloc(1, sizeof(Node));
    Token *tmp = token;
    token = token->next;
    if (consume("(")) {
      // 関数名だった場合
      Arg head;
      head.next = NULL;
      Arg *cur = &head;
      while (!consume(")")) {
        Arg *arg = calloc(1, sizeof(Arg));
        arg->node = expr();
        arg->next = NULL;
        cur->next = arg;
        cur = cur->next;
        consume(",");
      }
      node->args = head.next;
      node->kind = ND_CALL;
      node->fname = tmp->str;
      node->fname_len = tmp->len;
      return node;
    }
    token = tmp;

    node->kind = ND_LVAR;
    node_cnt++;

    LVar *lvar = find_lvar(token);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = token->str;
      lvar->len = token->len;
      lvar->offset = locals ? locals->offset + 8 : 8;
      node->offset = lvar->offset;
      locals = lvar;
    }
    token = token->next;

    return node;
  }
  Node *node = new_node_num(expect_number());
  return node;
}

Node **parse(Token *tok) {
  fprintf(stderr, "パース開始\n");
  token = tok;
  program();
  return code;
}