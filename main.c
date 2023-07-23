#include "fcc.h"

char *user_input;

int main(int argc, char **argv) {
  fprintf(stderr, "コンパイル開始\n");
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  user_input = argv[1];
  fprintf(stderr, "入力を受け取りました: %s\n\n", user_input);

  Token *token = tokenize(user_input);
  Node *node = parse(token);
  codegen(node);

  return 0;
}