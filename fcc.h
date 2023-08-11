#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// tokenize

typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_RETURN,    // return
  TK_WHILE,     // while
  TK_IF,        // if
  TK_ELSE,      // else
  TK_FOR,       // for
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
  TK_INT,       // int
  TK_SIZEOF,    // sizeof
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;  // トークンの種類
  Token *next;     // 次の入力トークン
  int val;         // TK_NUMの数値
  char *str;       // トークン文字列
  int len;         // トークン長
};

// parse

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool at_eof();
Token *tokenize(char *p);

// program    = function*
// function   = type ident "(" (type ident)? ("," "type ident)* ")"
//              "{" stmt* "}"
// stmt       = expr ";"
//            | "{" stmt* "}"
//            | "return" expr ";"
//            | "while" "(" expr ")" stmt
//            | "if" "(" expr ")" stmt ("else" stmt)?
//            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//            | type ident ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = primary ("*" primary | "/" primary)*
// unary      = ( "+" | "-" )? primary
//            | ("&" | "*") unary
//            | "sizeof" unary
// primary    = num
//            | ident ("(" expr? ")")?
//            | "(" expr ")"
// type       = "int" "*"*

typedef enum {
  ND_ADD,     // +
  ND_SUB,     // -
  ND_MUL,     // *
  ND_DIV,     // /
  ND_LT,      // <
  ND_LE,      // <=
  ND_EQ,      // ==
  ND_NEQ,     // !=
  ND_NUM,     // 整数
  ND_ASSIGN,  // 代入
  ND_LVAR,    // 左辺値
  ND_RETURN,  // return
  ND_WHILE,   // while
  ND_IF,      // if
  ND_FOR,     // for
  ND_BLOCK,   // ブロック
  ND_CALL,    // 関数呼び出し
  ND_FUNC,    // 関数定義
  ND_ADDR,    // アドレス
  ND_DEREF,   // dereference
} NodeKind;

typedef struct Type Type;
struct Type {
  enum { INT, PTR, ARRAY } ty;
  struct Type *ptr_to;
  size_t array_size;
};

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  Type *type;
  int len;
  int offset;
};

typedef struct Node Node;

struct Node {
  NodeKind kind;  // ノードの種類
  Node *lhs;      // 左オペランド
  Node *rhs;      // 右オペランド
  Node *init;     // kind == ND_FOR
  Node *cond;     // kind == ND_IF, ND_FOR
  Node *update;   // kind == ND_FOR
  Node *stmts;    // kind == ND_BLOCK
  Node *args;     // kind == ND_CALL, ND_FUNC
  Node *next;     // kind == ND_FUNC
  Type *type;     // kind == ND_FUNC, ND_LVAR
  int val;        // kind == ND_NUM
  int offset;     // kind == ND_LVAR
  int sf_size;    // kind == ND_FUNC
  char *fname;    // 関数名 kind == ND_CALL
  int fname_len;  // 関数名 kind == ND_CALL
  int id;         // デバッグ用
};

Node **parse(Token *tok);

// codegen

void codegen(Node **code);