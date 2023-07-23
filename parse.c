#include "fcc.h"

// 次のトークンがopの場合は読み進めてtrueを返す
// そうでない場合はfalseを返す
// 演算子にだけ使う
bool equal(Token *token, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  return true;
}

// 次のトークンがopの場合は読み進める
// そうでない場合はerrorを呼ぶ
Token *expect(Token *token, char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%C'ではありません", op);
  return token->next;
}

int expect_number(Token *token) {
  if (token->kind != TK_NUM) error_at(token->str, "数ではありません");
  int val = token->val;
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

Node *mul(Token **rest, Token *tok);
Node *primary(Token **rest, Token *tok);
Node *equality(Token **rest, Token *tok);
Node *relational(Token **rest, Token *tok);
Node *add(Token **rest, Token *tok);
Node *unary(Token **rest, Token *tok);

Node *expr(Token **rest, Token *tok) {
  Node *node = equality(rest, tok);
  return node;
}

Node *equality(Token **rest, Token *tok) {
  Node *node = relational(&tok, tok);
  for (;;) {
    if (equal(tok, "==")) {
      node = new_node(ND_EQ, node, relational(&tok, tok->next));
    } else if (equal(tok, "!=")) {
      node = new_node(ND_NEQ, node, relational(&tok, tok->next));
    } else {
      *rest = tok;
      return node;
    }
  }
}

Node *relational(Token **rest, Token *tok) {
  Node *node = add(&tok, tok);
  for (;;) {
    if (equal(tok, "<")) {
      node = new_node(ND_LT, node, add(&tok, tok->next));
    } else if (equal(tok, ">")) {
      node = new_node(ND_LT, add(&tok, tok->next), node);
    } else if (equal(tok, "<=")) {
      node = new_node(ND_LE, node, add(&tok, tok->next));
    } else if (equal(tok, ">=")) {
      node = new_node(ND_LE, add(&tok, tok->next), node);
    } else {
      *rest = tok;
      return node;
    }
  }
}

Node *add(Token **rest, Token *tok) {
  Node *node = mul(&tok, tok);
  for (;;) {
    if (equal(tok, "+")) {
      node = new_node(ND_ADD, node, mul(&tok, tok->next));
    } else if (equal(tok, "-")) {
      node = new_node(ND_SUB, node, mul(&tok, tok->next));
    } else {
      *rest = tok;
      return node;
    }
  }
}

Node *mul(Token **rest, Token *tok) {
  Node *node = unary(&tok, tok);
  for (;;) {
    if (equal(tok, "*")) {
      node = new_node(ND_MUL, node, unary(&tok, tok->next));
    } else if (equal(tok, "/")) {
      node = new_node(ND_DIV, node, unary(&tok, tok->next));
    } else {
      *rest = tok;
      return node;
    }
  }
}

Node *unary(Token **rest, Token *tok) {
  if (equal(tok, "+")) {
    return primary(rest, tok->next);
  } else if (equal(tok, "-")) {
    return new_node(ND_SUB, new_node_num(0), primary(rest, tok->next));
  }
  return primary(rest, tok);
}

// primary = "(" expr ")" | num

Node *primary(Token **rest, Token *tok) {
  if (equal(tok, "(")) {
    Node *node = expr(&tok, tok->next);
    *rest = expect(tok, ")");
    return node;
  }
  Node *node = new_node_num(expect_number(tok));
  *rest = tok->next;
  return node;
}

Node *parse(Token *tok) {
  Node *node = expr(&tok, tok);
  return node;
}