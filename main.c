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
  Node **code = parse(token);
  codegen(code);

  return 0;
}