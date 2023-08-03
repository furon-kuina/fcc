#include "fcc.h"

void print_token_list(Token *token) {
  fprintf(stderr, "Input tokenized as:\n");
  fprintf(stderr, "[");
  while (token) {
    if (token->kind == TK_NUM) {
      fprintf(stderr, "\"%i\"", token->val);
      if (token->next == NULL) break;
      fprintf(stderr, ", ");
      token = token->next;
    } else {
      char *buffer = calloc(token->len + 1, sizeof(char));
      strncpy(buffer, token->str, token->len);
      buffer[token->len] = '\0';
      fprintf(stderr, "\"%s\"", buffer);
      free(buffer);
      if (token->next == NULL) break;
      fprintf(stderr, ", ");
      token = token->next;
    }
  }
  fprintf(stderr, "]\n");
}

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
    case ND_LVAR:
      return "LVAR";
    case ND_ASSIGN:
      return "=";
    case ND_RETURN:
      return "return";
    case ND_WHILE:
      return "while";
    case ND_IF:
      return "if";
    case ND_FOR:
      return "for";
    case ND_BLOCK:
      return "block";
    case ND_CALL:
      return "function call";
    case ND_FUNC:
      return "function definition";
  }
}

void print_node(Node *node) {
  fprintf(stderr, "\n");
  fprintf(stderr, "ノード番号: %i\n", node->id);
  fprintf(stderr, "ノード種類: %s\n", node_dbg(node));
  if (node->lhs) {
    fprintf(stderr, "左の子: %i\n", node->lhs->id);
  }
  if (node->rhs) {
    fprintf(stderr, "右の子: %i\n", node->rhs->id);
  }
  if (node->lhs) {
    print_node(node->lhs);
  }
  if (node->rhs) {
    print_node(node->rhs);
  }
}

void print_functions(Node **functions) {
  int i = 0;
  while (functions[i]) {
    fprintf(stderr, "\n%i番目のコード: ", i);
    print_node(functions[i++]);
  }
}

int main(int argc, char **argv) {
  fprintf(stderr, "コンパイル開始\n");
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  char *user_input = argv[1];
  fprintf(stderr, "入力を受け取りました: %s\n\n", user_input);

  Token *token = tokenize(user_input);
  print_token_list(token);
  Node **functions = parse(token);
  print_functions(functions);
  codegen(functions);

  return 0;
}