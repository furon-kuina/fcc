#include "fcc.h"

// グローバル変数

Token *token;  // パーサに入力として与えられるトークン列
LVar *locals;  // ローカル変数を表す連結リスト
Node *functions[100];  // ";"で区切られたコード

int sf_size;

int node_cnt = 0;  // デバッグ用: ノードの数

// 次のトークンがopの場合は読み進めてtrueを返す
// そうでない場合はfalseを返す
// 演算子にだけ使う
bool consume_str(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

bool consume_token(TokenKind kind) {
  if (token->kind != kind) {
    return false;
  }
  token = token->next;
  return true;
}

bool equal(char *op) {
  return token->kind != TK_RESERVED || strlen(op) != token->len ||
         memcmp(token->str, op, token->len);
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
  if (token->kind != TK_NUM) error_at(token->str, "Expected number");
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
  node->type = calloc(1, sizeof(Type));
  node->type->ty = INT;
  node->type->ptr_to = NULL;
  node->id = node_cnt++;
  return node;
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  error_at(token->str, "Undefined identifier\n");
  return NULL;
}

size_t allocation_size(Type *ty) {
  if (ty->ty == ARRAY) {
    return 8 * ty->array_size;
  } else {
    return 8;
  }
}

Node *new_var(Token *tok, Type *ty) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->type = ty;
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  if (locals) {
    lvar->offset = locals->offset + allocation_size(ty);
  } else {
    lvar->offset = allocation_size(ty);
  }
  node->offset = lvar->offset;
  node->type = lvar->type;
  locals = lvar;
  sf_size += allocation_size(ty);
  return node;
}

Type *pointer_to(Type *type) {
  Type *res = calloc(1, sizeof(Type));
  res->ptr_to = type;
  res->ty = PTR;
  return res;
}

Type *base_type_of(Type *type) {
  if (type->ptr_to == NULL) {
    error_at(token->str, "Expected pointer type");
  }
  return type->ptr_to;
}

Type *array_of(Type *ty, size_t size) {
  Type *res = calloc(1, sizeof(Type));
  res->ty = ARRAY;
  res->ptr_to = ty;
  res->array_size = size;
  return res;
}

size_t size_of(Type *type) {
  if (type->ty == INT) {
    return 4;
  }
  if (type->ty == PTR) {
    return 8;
  }
  if (type->ty == ARRAY) {
    return size_of(type->ptr_to);
  }
}

Node *function();
Node *stmt();
Node *expr();
Node *assign();
Node *mul();
Node *primary();
Node *equality();
Node *relational();
Node *add();
Node *unary();
Type *type();

void program() {
  int i = 0;
  while (!at_eof()) {
    functions[i] = function();
    i++;
  }
  functions[i] = NULL;
}

Node *function() {
  Type *ty = type();
  if (token->kind != TK_IDENT) {
    error_at(token->str, "Expected identifier");
  }
  Node *node = calloc(1, sizeof(Node));
  node->fname = token->str;
  node->fname_len = token->len;
  node->type = ty;
  token = token->next;
  expect("(");
  {
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume_str(")")) {
      ty = type();
      Node *new = new_var(token, ty);
      new->type = ty;
      token = token->next;
      cur->next = new;
      cur = cur->next;
      consume_str(",");
    }
    node->args = head.next;
    node->kind = ND_FUNC;
  }

  {
    expect("{");
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume_str("}")) {
      Node *new = stmt();
      cur->next = new;
      cur = cur->next;
    }
    node->stmts = head.next;
  }
  node->sf_size = sf_size;
  sf_size = 0;
  locals = NULL;
  return node;
}

Node *stmt() {
  Node *node;
  if (consume_str("{")) {
    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume_str("}")) {
      Node *new = stmt();
      cur->next = new;
      cur = cur->next;
    }
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->stmts = head.next;
  } else if (consume_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
  } else if (consume_token(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_WHILE;
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
  } else if (consume_token(TK_IF)) {
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
  } else if (consume_token(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node_cnt++;
    node->kind = ND_FOR;
    expect("(");
    if (!consume_str(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume_str(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume_str(")")) {
      node->update = expr();
      expect(")");
    }
    node->lhs = stmt();

  } else if (token->kind == TK_INT) {
    Type *ty = type();
    if (token->kind != TK_IDENT) {
      error_at(token->str, "Expected identifier\n");
    }
    Token *identifier = token;
    token = token->next;
    if (consume_str("[")) {
      node = new_var(identifier, array_of(ty, token->val));
      expect("]");
    } else {
      node = new_var(identifier, ty);
    }
    expect(";");
  } else {
    node = expr();
    expect(";");
  }
  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume_str("=")) {
    node = new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume_str("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume_str("!=")) {
      node = new_node(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume_str("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume_str(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume_str("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume_str(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();
  bool is_add_or_sub = false;
  for (;;) {
    if (consume_str("+")) {
      is_add_or_sub = true;
      node = new_node(ND_ADD, node, mul());
    } else if (consume_str("-")) {
      is_add_or_sub = true;
      node = new_node(ND_SUB, node, mul());
    } else if (is_add_or_sub) {
      node->type = calloc(1, sizeof(Type));
      if (node->lhs->type->ty == PTR) {
        node->type->ty = PTR;
        node->type->ptr_to = node->lhs->type->ptr_to;
      } else if (node->rhs->type->ty == PTR) {
        node->type->ty = PTR;
        node->type->ptr_to = node->rhs->type->ptr_to;
      } else {
        node->type->ty = INT;
      }
      return node;
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume_str("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume_str("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

Node *unary() {
  if (consume_str("+")) {
    return primary();
  }
  if (consume_str("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  }
  if (consume_str("&")) {
    Node *operand = unary();
    Node *node = new_node(ND_ADDR, operand, NULL);
    node->type = pointer_to(operand->type);
    return node;
  }
  if (consume_str("*")) {
    Node *operand = unary();
    Node *node = new_node(ND_DEREF, operand, NULL);
    node->type = base_type_of(operand->type);
    return node;
  }
  if (consume_token(TK_SIZEOF)) {
    Node *node = unary();
    return new_node_num(size_of(node->type));
  }
  return primary();
}

Node *primary() {
  if (consume_str("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  if (token->kind == TK_IDENT) {
    Node *node = calloc(1, sizeof(Node));
    Token *tmp = token;
    token = token->next;
    if (consume_str("(")) {
      // 関数名だった場合
      Node head;
      head.next = NULL;
      Node *cur = &head;
      while (!consume_str(")")) {
        Node *new = expr();
        new->next = NULL;
        cur->next = new;
        cur = cur->next;
        consume_str(",");
      }
      node->args = head.next;
      node->kind = ND_CALL;
      node->type = calloc(1, sizeof(Type));
      // assuming that the function return type is int
      // TODO: attach the appropriate type
      node->type->ty = INT;
      node->type->ptr_to = NULL;
      node->fname = tmp->str;
      node->fname_len = tmp->len;
      return node;
    }
    token = tmp;

    node->kind = ND_LVAR;
    node_cnt++;

    LVar *lvar = find_lvar(token);
    node->offset = lvar->offset;
    node->type = lvar->type;
    token = token->next;
    return node;
  }
  Node *node = new_node_num(expect_number());
  return node;
}

Type *type() {
  if (token->kind != TK_INT) {
    error_at(token->str, "Expected typename\n");
  }
  token = token->next;
  Type *head = calloc(1, sizeof(Type));
  head->ty = INT;
  head->ptr_to = NULL;

  while (consume_str("*")) {
    Type *new = calloc(1, sizeof(Type));
    new->ty = PTR;
    new->ptr_to = head;
    head = new;
  }
  return head;
}

Node **parse(Token *tok) {
  fprintf(stderr, "パース開始\n");
  token = tok;
  program();
  return functions;
}