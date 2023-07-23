.intel_syntax noprefix
.globl main
main:
  push 0
  push 2
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  push 0
  push 1
  pop rdi
  pop rax
  sub rax, rdi
  push rax
  pop rdi
  pop rax
  cmp rax, rdi
  setl al
  movzb rax, al
  push rax
  pop rax
  ret
