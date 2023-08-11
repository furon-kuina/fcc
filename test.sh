#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./fcc "$input" > tmp.s
  cc -static -o tmp tmp.s linked/callee.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 2 "int main() {int x; int *y; y = &x; *y = 2; return x;}"
assert 1 "int main() {int a[1]; *a = 1; int *p; p = a; return *p;}"
assert 2 "int main() {int a[2];*(a + 1) = 2;int *p; p = a; return *(a + 1);}"
assert 3 "int main() {int a[2]; *a = 1; *(a + 1) = 2;int *p; p = a; return *p + *(p + 1);}"

assert 1 "int main(){int x; x = 1; return x;}"
assert 8 "int main(){int x; return sizeof(**&&&x);}"
assert 4 "int main(){int x; return sizeof(**&&x);}"
assert 4 "int main(){int x; return sizeof(*&x);}"
assert 8 "int main(){int **x; return sizeof(8 + x);}"
assert 8 "int main(){int *x; return sizeof(8 + x);}"
assert 8 "int main(){int *x; return sizeof(x + 8);}"
assert 4 "int main(){int x; return sizeof(sizeof x);}"
assert 8 "int main(){int x; x = 3; int *y; y = &x; int **z; z = &y; return sizeof(z);}"
assert 8 "int main(){int x; x = 3; int *y; y = &x; return sizeof(y);}"
assert 4 "int main(){int x; return sizeof x;}"
assert 4 "int main(){int x;return sizeof(x);}"
assert 4 "int main(){return sizeof(1);}"
assert 4 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = p + 2; return *q;}"
assert 3 'int main(){int x; x = 3;int *y; y=&x; int **z; z=&y; return **z; }'
assert 8 "int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q;q = p + 3; return *q;}"
assert 3 'int main(){int x; x=3; return *&x; }'
assert 5 'int main(){int x; x=3;int *y; y=&x; *y=5; return x; }'

# assert 1  "int *echo(int *x) { return x; } int main(){ int x; int *y; x = 1; return *echo(y); }"

assert 5 "int main() {
  int x;
  int *y;
  int **z;
  y = &x;
  z = &y;
  **z = 5;
  return x;
}"

assert 3 "int main() {
  int x;
  int *y;
  y = &x;
  *y = 3;
  return x;
}"

assert 3 "int main() {
  return 3;
}"
assert 5 "int main() {
  int x; 
  x = 3; 
  x = 5; 
  return x;
}"
assert 3 "int main() {
  int x; 
  x = 3; 
  return x;
}"
assert 10 "int add(int x, int y){
  return x+y;
}
int main(){
  return add(3,7);
}"
assert 233 "
int f(int x) {
  if (x <= 2) {
    return 1;
  } else {
    return f(x - 1) + f(x - 2);
  }
}
int main() {
  return f(13);
}
"
# assert 3 "main(){x=3;y=&x;return *y;}"
# assert 1 "f(x){{if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}}main(){return f(1);}"
# assert 1 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(2);}"
# assert 2 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(3);}"
# assert 3 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(4);}"
# assert 5 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(5);}"
# assert 55 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(10);}"
# assert 233 "f(x){if(x<=2){return 1;} else {return f(x-1)+f(x-2);}}main(){return f(13);}"


# assert 4 "f(x){if(x == 0){return 0;} else {return f(x-1)+1;}}main(){return f(4);}"
# assert 2 "f(x){return x+1;}main(){return f(1);}"
# assert 2 "f(){return 1;}main(){return f()+f();}"
# assert 1 "main(){return 1;}"
# assert 124 "main(){x=1;return g1(123)+x;}"
# assert 123 "main(){return g1(123);}"
# assert 2 "main(){f6(1,2,3,4,5,6);return 2;}"
# assert 2 "main(){f5(1,2,3,4,5);return 2;}"
# assert 2 "main(){f4(1,2,3,4);return 2;}"
# assert 2 "main(){f3(1,2,3);return 2;}"
# assert 1 "main(){foo();return 1;}"
# assert 2 "main(){f2(1,2);return 2;}"
# assert 2 "main(){f1(1);return 2;}"
# assert 1 "main(){i=1;{}return i;}"
# assert 2 "main(){i=1;for(;;){}return i+1;}"
# assert 1 "main(){i=0;while(i<10)if(i==0)return 1;return 0;}"
# assert 11 "main(){i=0;for(i=0;i<10;){if(i<5){i=i+1;}else{i=i+1;i=i+1;}}return i;}"
# assert 10 "main(){i=0;for(i=0;i<10;i=i+1)1+1;return i;}"
# assert 1 "main(){i=1;if(i!=0)return 1;else return 2;}"
# assert 1 "main(){i=0;while(i<10)if(i==0)return 1;else return 0;}"
# assert 11 "main(){i=0;while(i<10)if(i<5)i=i+1;else i=i+2;return i;}"
# assert 1 "main(){i=0;i=i+1;return i;}"
# assert 10 "main(){ i=0;while(i<10)i=i+1;return i;}"
# assert 128 "main(){ i=2;while(i<100)i=2*i;return i;}"
# assert 0 "main(){return 0;}"
# assert 42 "main(){return 42;}"
# assert 21 "main(){return 5+20-4;}"
# assert 29 "main(){return 19+10;}"
# assert 4 "main(){return 19-15;}"
# assert 8 "main(){return 8-0;}"
# assert 2 "main(){return 1 + 1;}"
# assert 10 "main(){return  1 - 11 + 20   ;}"
# assert 19 "main(){return 1+2*9;}"
# assert 255 "main(){return 16*16 -1;}"
# assert 4 "main(){return 20 / 5;}"
# assert 39 "main(){return 10/ 2 + 17*2;}"
# assert 0 "main(){return 0*2;}"
# assert 2 "main(){return 39/13-1*1;}"
# assert 3 "main(){return (1+5)/2;}"
# assert 10 "main(){return (2+3) * (3-1);}"
# assert 1 "main(){return 1==1;}"
# assert 0 "main(){return 0==1;}"
# assert 1 "main(){return  20 == (5+15);}"
# assert 7 "main(){return ((1 + 20)==(3*7))*7;}"
# assert 1 "main(){return 20!=21;}"
# assert 0 "main(){return 2!=2;}"
# assert 0 "main(){return ((1 + 20)!=(3*7))*7;}"
# assert 1 "main(){return (1+20==21)*(34!=0);}"
# assert 0 "main(){return 1<0;}"
# assert 0 "main(){return 1<1;}"
# assert 1 "main(){return 1<2;}"
# assert 1 "main(){return 1>0;}"
# assert 0 "main(){return 1>1;}"
# assert 0 "main(){return 1>2;}"
# assert 0 "main(){return 1<=0;}"
# assert 1 "main(){return 1<=1;}"
# assert 1 "main(){return 1<=2;}"
# assert 1 "main(){return 1>=0;}"
# assert 1 "main(){return 1>=1;}"
# assert 0 "main(){return 1>=2;}"
# assert 20 "main(){return -0+20;}"
# assert 5 "main(){return -1+6;}"
# assert 2 "main(){return -10/-5;}"
# assert 1 "main(){return 0>-1;}"
# assert 0 "main(){return 0<-1;}"
# assert 10 "main(){return 20-10;}"
# assert 1 "main(){return -1==-1;}"
# assert 2 "main(){return -1*-2;}"
# assert 1 "main(){return -1>-2;}"
# assert 1 "main(){a=1; return a;}"
# assert 1 "main(){aa=-1*-1;return aa;}"
# assert 6 "main(){ab=2+1;return ab+3;}"
# assert 4 "main(){aa=1;bb=2;cc=1;return aa+bb+cc;}"
# assert 1 "main(){abaidfhaofjdsoa=2;fafdhaslfhaofjas=3;return abaidfhaofjdsoa!=fafdhaslfhaofjas;}"
# assert 6 "main(){a1=1;a3343=5;return a1+a3343;}"
# assert 1 "main(){_=1;return _;}"
# assert 4 "main(){__=1;___=3 ;return ___+__;}"
# assert 2 "main(){return 2;1;}"
# assert 3 "main(){a=3;return a;b=1;}"
# assert 2 "main(){a=1;a=2;return a;}"
# assert 5 "main(){a=3;b=2;return a+b;}"
echo OK