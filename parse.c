#include "fcc.h"

Token *token;

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
    case ND_LE:
      return "<=";
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
Node *unary();

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
  } else if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  return primary();
}

// primary = "(" expr ")" | num

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  Node *node = new_node_num(expect_number());
  return node;
}

Node *parse(Token *tok) {
  fprintf(stderr, "パース開始\n");
  token = tok;
  Node *node = expr();
  return node;
}