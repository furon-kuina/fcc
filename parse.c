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

// type-related functions

Type *array_of(Type *ty, size_t size) {
  Type *res = calloc(1, sizeof(Type));
  res->ty = ARRAY;
  res->ptr_to = ty;
  res->array_size = size;
  return res;
}

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
Node *postfix();
Type *type();

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs) {
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

Node *new_node_char(char val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CHAR;
  node->val = val;
  node->id = node_cnt++;
  return node;
}

Node *new_node_lvar_def(Token *ident, Type *ty) {
  Node *node = calloc(1, sizeof(Node));
  node->type = calloc(1, sizeof(Type));
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

Node *new_node_lvar(LVar *lvar) {
  Node *node = calloc(1, sizeof(Node));
  node->type = calloc(1, sizeof(Type));
  node->kind = ND_LVAR;
  node->id = node_cnt++;
  node->offset = lvar->offset;
  node->name = lvar->name;
  node->len = lvar->len;
  return node;
}

Node *new_node_gvar(Token *ident) {
  GVar *gvar = find_gvar(ident);
  if (!gvar) {
    error_at(ident->str, "undeclared identifier: %.*s", ident->len, ident->str);
  }
  Node *node = calloc(1, sizeof(Node));
  node->type = calloc(1, sizeof(Type));
  node->kind = ND_GVAR;
  node->id = node_cnt++;
  node->offset = gvar->offset;
  node->name = gvar->name;
  node->len = gvar->len;
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
  globals = gvar;
  node->id = node_cnt++;
  return node;
}

Node *new_node_func(Token *ident, Type *ty, Node *args, Node *stmts) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNC;
  node->name = ident->str;
  node->len = ident->len;
  node->type = ty;
  node->args = args;
  node->stmts = stmts;
  node->sf_size = sf_size;
  sf_size = 0;
  locals = NULL;
  node->id = node_cnt++;
  return node;
}

Node *new_node_call(Token *ident, Node *args) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CALL;
  node->args = args;
  node->name = ident->str;
  node->len = ident->len;
  node->id = node_cnt++;
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

// func-arg-list =  "(" type ident ("," type ident)* ")" | "(" ")"
Node *func_arg_list() {
  expect("(");
  Node args_head;
  args_head.next = NULL;
  Node *cur = &args_head;
  while (!consume_str(")")) {
    Type *ty = type();
    Node *new = new_node_lvar_def(token, ty);
    new->type = ty;
    advance();
    cur->next = new;
    cur = cur->next;

    consume_str(",");
  }
  return args_head.next;
}

// program = (global-var | function)*
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
    node->id = node_cnt++;
    node->stmts = head.next;
  } else if (consume_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->id = node_cnt++;
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
    // local variable definition
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
      node = new_node_lvar_def(identifier, array_of(ty, token->val));
      advance();
      expect("]");
    } else {
      node = new_node_lvar_def(identifier, ty);
    }
    expect(";");
  } else {
    node = expr();
    expect(";");
  }
  return node;
}

Node *expr() { return assign(); }

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume_str("=")) {
    node = new_node_binary(ND_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume_str("==")) {
      node = new_node_binary(ND_EQ, node, relational());
    } else if (consume_str("!=")) {
      node = new_node_binary(ND_NEQ, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();
  for (;;) {
    if (consume_str("<")) {
      node = new_node_binary(ND_LT, node, add());
    } else if (consume_str(">")) {
      node = new_node_binary(ND_LT, add(), node);
    } else if (consume_str("<=")) {
      node = new_node_binary(ND_LE, node, add());
    } else if (consume_str(">=")) {
      node = new_node_binary(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();
  for (;;) {
    if (consume_str("+")) {
      node = new_node_binary(ND_ADD, node, mul());
    } else if (consume_str("-")) {
      node = new_node_binary(ND_SUB, node, mul());
    } else {
      break;
    }
  }
  return node;
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();
  for (;;) {
    if (consume_str("*")) {
      node = new_node_binary(ND_MUL, node, unary());
    } else if (consume_str("/")) {
      node = new_node_binary(ND_DIV, node, unary());
    } else {
      break;
    }
  }
  return node;
}

// unary = ("+" | "-" | "*" | "&")? unary
//       | postfix
Node *unary() {
  if (consume_str("+")) {
    // TODO: fix to unary()
    return primary();
  }
  if (consume_str("-")) {
    return new_node_binary(ND_SUB, new_node_num(0), primary());
  }
  if (consume_str("&")) {
    Node *operand = unary();
    Node *node = new_node_binary(ND_ADDR, operand, NULL);
    return node;
  }
  if (consume_str("*")) {
    Node *operand = unary();
    Node *node = new_node_binary(ND_DEREF, operand, NULL);
    return node;
  }
  if (consume_token(TK_SIZEOF)) {
    Node *operand = unary();
    return new_node_binary(ND_SIZEOF, operand, NULL);
  }
  return postfix();
}

// postfix = primary ( "[" expr "]")*
// postfix: 後置演算子
Node *postfix() {
  Node *node = primary();
  // x[y] is equivalent to *(x + y)
  while (consume_str("[")) {
    node =
        new_node_binary(ND_DEREF, new_node_binary(ND_ADD, node, expr()), NULL);
    expect("]");
  }
  return node;
}

// func-args = "(" (assign ("," assign)*)? ")"
Node *func_args() {
  if (consume_str(")")) {
    return NULL;
  }
  Node *head = assign();
  Node *cur = head;
  while (consume_str(",")) {
    cur->next = assign();
    cur = cur->next;
  }
  expect(")");
  return head;
}

// primary = "(" expr ")" | ident func-args? | num
Node *primary() {
  if (consume_str("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  if (token->kind == TK_IDENT) {
    Token *ident = token;
    advance();
    if (consume_str("(")) {
      // the identifier is a function name
      return new_node_call(ident, func_args());
    }
    Node *node;
    // the identifier is a variable name
    token = ident;
    LVar *lvar = find_lvar(token);
    if (lvar) {
      // local vairable
      node = new_node_lvar(lvar);
    } else {
      // no local variable named ident
      // thus global variable
      node = new_node_gvar(ident);
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

Program *parse(Token *tok) {
  fprintf(stderr, "パース開始\n");
  token = tok;
  program();
  Program *program = calloc(1, sizeof(Program));
  program->functions = funcs;
  program->globals = globals;
  program->locals = locals;
  program->definitions = definitions;
  return program;
}