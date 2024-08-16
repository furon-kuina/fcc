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

char *print_nodekind(Node *node) {
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
      return "int literal";
    case ND_LVAR:
      return "local variable";
    case ND_ASSIGN:
      return "assign";
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
    case ND_ADDR:
      return "address";
    case ND_DEREF:
      return "dereference";
    case ND_CHAR:
      return "char";
    case ND_GVAR:
      return "global variable";
    case ND_GVAR_DEF:
      return "global variable definition";
  }
}

void print_indent(char *str, int indent) {
  for (int i = 0; i < indent; i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "%s", str);
}

void print_node(Node *node, int indent) {
  for (int i = 0; i < indent; i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "node #%i, kind: \"%s\", ", node->id, print_nodekind(node));
  switch (node->kind) {
    case ND_ADD:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_SUB:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_MUL:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_DIV:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_LT:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_LE:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_EQ:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_NEQ:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_NUM:
      fprintf(stderr, "value: %d\n", node->val);
      break;
    case ND_LVAR:
      fprintf(stderr, "name: %.*s\n", node->len, node->name);
      break;
    case ND_ASSIGN:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_RETURN:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      break;
    case ND_WHILE:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      print_node(node->rhs, indent + 2);
      break;
    case ND_IF:
      fprintf(stderr, "\n");

      print_node(node->cond, indent + 2);
      print_node(node->lhs, indent + 2);
      if (node->rhs) {
        // else
        print_node(node->rhs, indent + 2);
      }
      break;
    case ND_FOR:
      fprintf(stderr, "\n");
      print_indent("initialization:\n", indent + 2);
      if (node->init) {
        print_node(node->init, indent + 2);
      }
      print_indent("condition:\n", indent + 2);
      if (node->cond) {
        print_node(node->cond, indent + 2);
      }
      print_indent("update:\n", indent + 2);
      if (node->update) {
        print_node(node->update, indent + 2);
      }
      print_node(node->lhs, indent + 2);
      break;
    case ND_BLOCK:
      fprintf(stderr, "\n");
      for (Node *stmt = node->stmts; stmt; stmt = stmt->next) {
        print_node(stmt, indent + 2);
      }
      break;
    case ND_CALL:
      fprintf(stderr, "name: %.*s\n", node->len, node->name);
      print_indent("arguments:\n", indent + 2);
      if (node->args) {
        print_node(node->args, indent + 2);
      }
      break;
    case ND_FUNC:
      fprintf(stderr, "name: %.*s\n", node->len, node->name);
      print_node(node->stmts, indent + 2);
      break;
    case ND_ADDR:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      break;
    case ND_DEREF:
      fprintf(stderr, "\n");
      print_node(node->lhs, indent + 2);
      break;
    case ND_CHAR:
      break;
    case ND_GVAR:
      fprintf(stderr, "name: %.*s\n", node->len, node->name);
      break;
    case ND_GVAR_DEF:
      fprintf(stderr, "name: %.*s\n", node->len, node->name);
      break;
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
  Program *program = parse(token);
  for (int i = 0; program->definitions[i]; i++) {
    print_node(program->definitions[i], 0);
  }
  program = annotate_type(program);
  codegen(program->definitions);

  return 0;
}