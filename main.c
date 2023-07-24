#include "fcc.h"

int main(int argc, char **argv) {
  fprintf(stderr, "コンパイル開始\n");
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }
  char *user_input = argv[1];
  fprintf(stderr, "入力を受け取りました: %s\n\n", user_input);

  Token *token = tokenize(user_input);
  Node **code = parse(token);
  codegen(code);

  return 0;
}