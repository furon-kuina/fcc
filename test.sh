#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  ./fcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 29 "19+10;"
assert 4 "19-15;"
assert 8 "8-0;"
assert 2 "1 + 1;"
assert 10 " 1 - 11 + 20   ;"
assert 19 "1+2*9;"
assert 255 "16*16 -1;"
assert 4 "20 / 5;"
assert 39 "10/ 2 + 17*2;"
assert 0 "0*2;"
assert 2 "39/13-1*1;"
assert 3 "(1+5)/2;"
assert 10 "(2+3) * (3-1);"
assert 1 "1==1;"
assert 0 "0==1;"
assert 1 " 20 == (5+15);"
assert 7 "((1 + 20)==(3*7))*7;"
assert 1 "20!=21;"
assert 0 "2!=2;"
assert 0 "((1 + 20)!=(3*7))*7;"
assert 1 "(1+20==21)*(34!=0);"
assert 0 "1<0;"
assert 0 "1<1;"
assert 1 "1<2;"
assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 0 "1<=0;"
assert 1 "1<=1;"
assert 1 "1<=2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"
assert 20 "-0+20;"
assert 5 "-1+6;"
assert 2 "-10/-5;"
assert 1 "0>-1;"
assert 0 "0<-1;"
assert 10 "20-10;"
assert 1 "-1==-1;"
assert 2 "-1*-2;"
assert 1 "-1>-2;"
assert 1 "a=1;a;"
assert 1 "aa=-1*-1;aa;"
assert 6 "ab=2+1;ab+3;"
assert 4 "aa=1;bb=2;cc=1;aa+bb+cc;"
assert 1 "abaidfhaofjdsoa=2;fafdhaslfhaofjas=3;abaidfhaofjdsoa!=fafdhaslfhaofjas;"
assert 6 "a1=1;a3343=5;a1+a3343;"
assert 1 "_=1;_;"
assert 4 "__=1;___=3 ;___+__;"
assert 2 "return 2;1;"
assert 3 "a=3;return a;b=1;"
echo OK