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

// parse

void error(char *fmt, ...);

void error_at(char *loc, char *fmt, ...);

bool at_eof();

Token *tokenize(char *p);

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = primary ("*" primary | "/" primary)*
// unary      = ( "+" | "-" )? primary
// primary    = num | ident | "(" expr ")"

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
  ND_ASSIGN,  // =
  ND_LVAR,    // 左辺値
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;  // ノードの種類
  Node *lhs;      // 左オペランド
  Node *rhs;      // 右オペランド
  int val;        // kind == ND_NUMのときのみ
  int offset;     // kind == ND_LVARのときのみ
};

Node **parse(Token *tok);

// codegen

void codegen(Node **code);