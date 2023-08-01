#include <stdio.h>
void foo() { printf("OK\n"); }

void f1(int x) { printf("Argument: %i\n", x); }
void f2(int x1, int x2) { printf("Arguments: %i, %i\n", x1, x2); }
void f3(int x1, int x2, int x3) {
  printf("Arguments: %i, %i, %i\n", x1, x2, x3);
}
void f4(int x1, int x2, int x3, int x4) {
  printf("Arguments: %i, %i, %i, %i\n", x1, x2, x3, x4);
}
void f5(int x1, int x2, int x3, int x4, int x5) {
  printf("Arguments: %i, %i, %i, %i, %i\n", x1, x2, x3, x4, x5);
}
void f6(int x1, int x2, int x3, int x4, int x5, int x6) {
  printf("Arguments: %i, %i, %i, %i, %i, %i\n", x1, x2, x3, x4, x5, x6);
}

int g1(int x) { return x; }