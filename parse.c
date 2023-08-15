#include "fcc.h"

// グローバル変数

Token *token;  // パーサに入力として与えられるトークン列
LVar *locals;  // ローカル変数を表す連結リスト
GVar *globals;
Func *funcs;             // 関数を表す連結リスト
Node *definitions[100];  // ";"で区切られたコード

int sf_size;

int node_cnt = 1;  // デバッグ用: ノードの数

void advance() { token = token->next; }

// 次のトークンがopの場合は読み進めてtrueを返す
// そうでない場合はfalseを返す
// 演算子にだけ使う
bool consume_str(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  advance();
  return true;
}

bool consume_token(TokenKind kind) {
  if (token->kind != kind) {
    return false;
  }
  advance();
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
    error_at(token->str, "Expected \"%s\"", op);
  advance();
}

int expect_number() {
  if (token->kind != TK_NUM) error_at(token->str, "Expected number");
  int val = token->val;
  advance();
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  return NULL;
}

GVar *find_gvar(Token *tok) {
  for (GVar *var = globals; var; var = var->next) {
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
      return var;
    }
  }
  error_at(token->str, "Undefined identifier\n");
  return NULL;
}

Func *find_func(Token *tok) {
  for (Func *f = funcs; f; f = f->next) {
    if (f->len == tok->len && !memcmp(tok->str, f->name, f->len)) {
      return f;
    }
  }
  error_at(token->str, "Undeclared function\n");
  return NULL;
}

size_t allocation_size(Type *ty) {
  if (ty->ty == ARRAY) {
    return 8 * ty->array_size;
  } else {
    return 8;
  }
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
  switch (type->ty) {
    case CHAR:
      return 1;
    case INT:
      return 4;
    case PTR:
      return 8;
    case ARRAY:
      return size_of(type->ptr_to) * type->array_size;
  }
}

// Type *add_type(Node *node) {
//   switch (node->kind) {
//     case ND_ADD:
//     case ND_SUB:
//     case ND_MUL:
//     case ND_DIV:
//     case ND_LT:
//     case ND_LE:
//     case ND_EQ:
//     case ND_NEQ:
//     case ND_NUM:
//     return case ND_ASSIGN:
//       return node->rhs->type;
//     case ND_LVAR:
//     case ND_ADDR:
//       return pointer_to(node->type);
//     case ND_DEREF:
//       return base_type_of(node->type);
//   }
// }

Node *func_def();
Node *gvar_def();
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

Node *new_node_char(char val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CHAR;
  node->val = val;
  node->type = calloc(1, sizeof(Type));
  node->type->ty = CHAR;
  node->type->ptr_to = NULL;
  node->id = node_cnt++;
  return node;
}

Node *new_node_lvar(Token *ident, Type *ty) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_LVAR;
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->type = ty;
  lvar->next = locals;
  lvar->name = ident->str;
  lvar->len = ident->len;
  if (locals) {
    lvar->offset = locals->offset + allocation_size(ty);
  } else {
    lvar->offset = allocation_size(ty);
  }
  node->offset = lvar->offset;
  node->type = lvar->type;
  node->id = node_cnt++;
  locals = lvar;
  sf_size += allocation_size(ty);
  return node;
}

Node *new_node_gvar_def(Token *ident, Type *ty) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_GVAR_DEF;
  node->name = ident->str;
  node->len = ident->len;
  GVar *gvar = calloc(1, sizeof(GVar));
  gvar->type = ty;
  gvar->next = globals;
  gvar->name = ident->str;
  gvar->len = ident->len;
  if (globals) {
    gvar->offset = globals->offset + allocation_size(ty);
  } else {
    gvar->offset = allocation_size(ty);
  }
  node->offset = gvar->offset;
  node->type = gvar->type;
  node->id = node_cnt++;
  globals = gvar;
  return node;
}

Node *new_node_func(Token *ident, Type *ty, Node *args, Node *stmts) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->id = node_cnt++;
  node->name = ident->str;
  node->len = ident->len;
  node->type = ty;
  node->args = args;
  node->stmts = stmts;
  node->sf_size = sf_size;
  sf_size = 0;
  locals = NULL;
  return node;
}

void register_func(Token *ident, Type *type) {
  Func *func = calloc(1, sizeof(Func));
  func->return_type = type;
  func->name = ident->str;
  func->len = ident->len;
  func->next = funcs;
  funcs = func;
}

// reads "(" type ident ("," type ident)* ")" | "(" ")"
Node *func_arg_list() {
  expect("(");
  Node args_head;
  args_head.next = NULL;
  Node *cur = &args_head;
  while (!consume_str(")")) {
    Type *ty = type();
    Node *new = new_node_lvar(token, ty);
    new->type = ty;
    advance();
    cur->next = new;
    cur = cur->next;

    consume_str(",");
  }
  return args_head.next;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    Token *tmp = token;
    type();
    if (token->kind != TK_IDENT) {
      error_at(token->str, "Expected identifier\n");
    }
    advance();
    if (*(token->str) == '(') {
      // function definition
      token = tmp;
      definitions[i] = func_def();
    } else {
      // global variable definition
      token = tmp;
      definitions[i] = gvar_def();
    }
    i++;
  }
  definitions[i] = NULL;
}

Node *func_def() {
  Type *ty = type();
  if (token->kind != TK_IDENT) {
    error_at(token->str, "Expected identifier");
  }
  Token *ident = token;
  register_func(ident, ty);
  advance();
  Node *args = func_arg_list();
  Node *stmts = stmt();
  return new_node_func(ident, ty, args, stmts);
}

Node *gvar_def() {
  Type *ty = type();
  Token *ident = token;
  advance();
  if (consume_str("[")) {
    ty->ptr_to = ty;
    ty->ty = ARRAY;
    ty->array_size = expect_number();
    expect("]");
  }
  expect(";");
  return new_node_gvar_def(ident, ty);
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
      advance();
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

  } else if (token->kind == TK_INT || token->kind == TK_CHAR) {
    // 変数定義
    Type *ty = type();
    if (token->kind != TK_IDENT) {
      error_at(token->str, "Expected identifier\n");
    }
    Token *identifier = token;
    advance();
    if (consume_str("[")) {
      if (token->kind != TK_NUM) {
        expect_number();
      }
      node = new_node_lvar(identifier, array_of(ty, token->val));
      advance();
      expect("]");
    } else {
      node = new_node_lvar(identifier, ty);
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
      if (node->lhs->type->ty == PTR || node->lhs->type->ty == ARRAY) {
        node->type->ty = PTR;
        node->type->ptr_to = node->lhs->type->ptr_to;
      } else if (node->rhs->type->ty == PTR || node->rhs->type->ty == ARRAY) {
        node->type->ty = PTR;
        node->type->ptr_to = node->rhs->type->ptr_to;
      } else {
        node->type->ty = node->lhs->type->ty;
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
    Token *ident = token;
    advance();

    if (consume_str("[")) {
      // TODO: support array element access in
      // a format like 3[a]
      Node *num_node = new_node_num(expect_number());
      Node *ptr_node = calloc(1, sizeof(Node));
      ptr_node->kind = ND_LVAR;
      LVar *lvar = find_lvar(ident);
      ptr_node->id = node_cnt++;
      ptr_node->offset = lvar->offset;
      ptr_node->type = lvar->type;
      Node *add_node = new_node(ND_ADD, ptr_node, num_node);
      add_node->type = ptr_node->type;
      node = new_node(ND_DEREF, add_node, NULL);
      node->type = base_type_of(ptr_node->type);
      expect("]");
      return node;
    }

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
      node->type->ptr_to = NULL;
      node->type->ty = INT;
      node->name = ident->str;
      node->len = ident->len;
      return node;
    }
    // 変数名だった場合
    token = ident;
    LVar *lvar = find_lvar(token);
    if (lvar) {
      node->kind = ND_LVAR;
      node->id = node_cnt++;
      node->offset = lvar->offset;
      node->type = lvar->type;
    } else {
      GVar *gvar = find_gvar(token);
      node->kind = ND_GVAR;
      node->id = node_cnt++;
      node->offset = gvar->offset;
      node->type = gvar->type;
      node->name = gvar->name;
      node->len = gvar->len;
    }

    advance();
    return node;
  }
  Node *node = new_node_num(expect_number());
  return node;
}

Type *type() {
  if (token->kind != TK_INT && token->kind != TK_CHAR) {
    error_at(token->str, "Expected typename\n");
  }
  Type *head = calloc(1, sizeof(Type));
  if (token->kind == TK_INT) {
    head->ty = INT;
  } else {
    head->ty = CHAR;
  }
  head->ptr_to = NULL;
  advance();

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
  return definitions;
}