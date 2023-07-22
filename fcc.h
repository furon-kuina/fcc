#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(Token *token, char *op);
void expect(Token *tok, char *op);

Token *tokenize(char *p);

// expr       = equality
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = primary ("*" primary | "/" primary)*
// unary      = ( "+" | "-" )? primary
// primary    = num | "(" expr ")"

typedef enum {
  ND_ADD,  // +
  ND_SUB,  // -
  ND_MUL,  // *
  ND_DIV,  // /
  ND_LT,   // <
  ND_LE,   // <=
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

Node *parse(Token *token);

// codegen

void gen(Node *node);