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

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 29 "19+10"
assert 4 "19-15"
assert 8 "8-0"
assert 2 "1 + 1"
assert 10 " 1 - 11 + 20   "
assert 19 "1+2*9"
assert 255 "16*16 -1"
assert 4 "20 / 5"
assert 39 "10/ 2 + 17*2"
assert 0 "0*2"
assert 2 "39/13-1*1"
assert 3 "(1+5)/2"
assert 10 "(2+3) * (3-1)"
echo OK