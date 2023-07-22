#include "fcc.h"

// 次のトークンがopの場合は読み進めてtrueを返す
// そうでない場合はfalseを返す
// 演算子にだけ使う
bool consume(Token *token, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 次のトークンがopの場合は読み進める
// そうでない場合はerrorを呼ぶ
void expect(Token *token, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%C'ではありません", op);
  token = token->next;
}

int expect_number(Token *token) {
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

Node *mul(Token *tok);
Node *primary(Token *tok);
Node *equality(Token *tok);
Node *relational(Token *tok);
Node *add(Token *tok);
Node *unary(Token *tok);

Node *expr(Token *tok) {
  Node *node = equality(tok);
  return node;
}

Node *equality(Token *tok) {
  Node *node = relational(tok);
  for (;;) {
    if (consume(tok, "==")) {
      node = new_node(ND_EQ, node, relational(tok));
    } else if (consume(tok, "!=")) {
      node = new_node(ND_NEQ, node, relational(tok));
    } else {
      return node;
    }
  }
}

Node *relational(Token *tok) {
  Node *node = add(tok);
  for (;;) {
    if (consume(tok, "<")) {
      node = new_node(ND_LT, node, add(tok));
    } else if (consume(tok, ">")) {
      node = new_node(ND_LT, add(tok), node);
    } else if (consume(tok, "<=")) {
      node = new_node(ND_LE, node, add(tok));
    } else if (consume(tok, ">=")) {
      node = new_node(ND_LE, add(tok), node);
    } else {
      return node;
    }
  }
}

Node *add(Token *tok) {
  Node *node = mul(tok);
  for (;;) {
    if (consume(tok, "+")) {
      node = new_node(ND_ADD, node, mul(tok));
    } else if (consume(tok, "-")) {
      node = new_node(ND_SUB, node, mul(tok));
    } else {
      return node;
    }
  }
}

Node *mul(Token *tok) {
  Node *node = unary(tok);
  for (;;) {
    if (consume(tok, "*")) {
      node = new_node(ND_MUL, node, unary(tok));
    } else if (consume(tok, "/")) {
      node = new_node(ND_DIV, node, unary(tok));
    } else {
      return node;
    }
  }
}

Node *unary(Token *tok) {
  if (consume(tok, "+")) {
    return primary(tok);
  } else if (consume(tok, "-")) {
    return new_node(ND_SUB, new_node_num(0), primary(tok));
  }
  return primary(tok);
}

Node *primary(Token *tok) {
  if (consume(tok, "(")) {
    Node *node = expr(tok);
    expect(tok, ")");
    return node;
  }
  return new_node_num(expect_number(tok));
}

Node *parse(Token *tok) {
  Node *node = expr(tok);
  return node;
}