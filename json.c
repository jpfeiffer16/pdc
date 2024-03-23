#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/*
  RFC: https://datatracker.ietf.org/doc/html/rfc8259#section-2
*/

#define MAX_JSON_ERROR_SIZE 512

typedef enum {
  JSON_UNKOWN,
  JSON_ERROR,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_NUMBER,
  JSON_STRING,
  JSON_BOOL,
  JSON_NULL
} node_type;

char* get_ast_node_type_name(node_type token) {
  switch (token) {
    case JSON_UNKOWN: return "JSON_UNKOWN";
    case JSON_ERROR:  return "JSON_ERROR";
    case JSON_OBJECT: return "JSON_OBJECT";
    case JSON_ARRAY:  return "JSON_ARRAY";
    case JSON_NUMBER: return "JSON_NUMBER";
    case JSON_STRING: return "JSON_STRING";
    case JSON_BOOL:   return "JSON_BOOL";
    case JSON_NULL:   return "JSON_NULL";
    default:          return "UNKNOWN!";
  }
}

typedef struct node {
  node_type type;
  struct node *children;
} node;

typedef enum {
  TK_NONE,
  TK_LBRACE,
  TK_RBRACE,
  TK_LBRACKET,
  TK_RBRACKET,
  TK_IDENTIFIER,
  TK_STRING,
  TK_COLON,
  TK_COMMA
} token_type;

char* get_token_type_name(token_type token) {
  switch (token) {
    case TK_NONE:       return "NONE";
    case TK_LBRACE:     return "LBRACE";
    case TK_RBRACE:     return "RBRACE";
    case TK_LBRACKET:   return "LBRACKET";
    case TK_RBRACKET:   return "RBRACKET";
    case TK_IDENTIFIER: return "IDENTIFIER";
    case TK_STRING:     return "STR";
    case TK_COLON:      return "COLON";
    case TK_COMMA:      return "COMMA";
    default:            return "UNKNOWN!";
  }
}

typedef struct {
  token_type type;
  char *value;
  int line;
  int column;
} token;

typedef struct token_node {
  struct token_node *next;
  token token;
} token_node;

typedef struct {
  token_node *tail;
  token_node *head;
} token_list;

token_node* new_token(token_list *list) {
  token_node *tk_l = malloc(sizeof(token_node));
  if (!list->tail) {
    list->tail = tk_l;
    list->head = tk_l;
  } else {
    list->head->next = tk_l;
    list->head = tk_l;
  }

  return tk_l;
}

typedef struct {
  token_list *tokens;
  enum {
    SUCCESS, ERROR
  } status;
  char *error;
} tokenize_result;

tokenize_result tokenize(char *buf, long buf_len, int idx) {
  token_list *tlist = malloc(sizeof(token_list));
  // TODO: Make this check more robust.
  tokenize_result error = { .status = ERROR, .error = "emptly list" };
  if (!buf_len) return error;
  char ch = buf[idx];
  int last_newline = 0;
  int line = 0;

  do {
    if (ch == '\n') {
      last_newline = idx;
      line++;
    }
    switch (ch) {
      case '{': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_LBRACE, .value = "{", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '}': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_RBRACE, .value = "}", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '[': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_LBRACKET, .value = "[", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ']': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_RBRACKET, .value = "]", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ',': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_COMMA, .value = ",", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ':': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = TK_COLON, .value = ":", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '"': {
        token_node *tk_l = new_token(tlist);
        int begin = idx;
        token tk = { .type = TK_STRING, .line = line, .column = idx - last_newline };
        while (buf[++idx] != '"') {
          if (buf[idx] == '\\') {
            idx++;
          }
        }
        int val_size = ++idx - (begin + 1);
        --idx;
        char *val = malloc(val_size + 1);
        strncpy(val, (char*)buf + begin + 1, val_size - 1);
        val[val_size] = '\0';
        //TODO: Is there a better way to do with that is both space and time efficient?
        int shift = 0;
        for (int i = 0; i < (val_size); i++) {
          // TODO: Check the RFC. Do we need to allow only certain
          // escape sequences, or can we safely assume we can allow
          // any char to be escaped.
          if (val[i] == '\\') shift++;
          else if (shift) val[i - shift] = val[i];
        }
        tk.value = val;
        // TODO: Don't do a copy here
        tk_l->token = tk;
        break;
      }
      default: {
        if (ch > 32 && ch < 127) {
          token_node *tk_l = new_token(tlist);
          token tk = { .type = TK_IDENTIFIER, .line = line, .column = idx - last_newline };
          int begin = idx;
          while (buf[++idx] != '{'
              && buf[idx]   != '}'
              && buf[idx]   != '['
              && buf[idx]   != ']'
              && buf[idx]   != ':'
              && buf[idx]   != ',') { }
          int len = idx - begin;
          --idx;
          char *val = malloc(len + 1);
          strncpy(val, (char*)buf + begin, len);
          val[len] = '\0';
          tk.value = val;
          // TODO: don't do a copy here.
          tk_l->token = tk;
        }
        break;
      }
    }
  } while ((ch = buf[++idx]) != EOF);

  tokenize_result res;
  res.status = SUCCESS;
  res.tokens = tlist;
  return res;
}

void print_list(token_list *list) {
  token_node *tokens = list->tail;
  while (tokens->next) {
    printf(
      "%d,%d:%s:\t\t%s\n",
      tokens->token.line,
      tokens->token.column,
      get_token_type_name(tokens->token.type),
      tokens->token.value);
    tokens = tokens->next;
  }
}

typedef struct {
  char *error;
  node ast_node;
  token_node *last_node;
} ast_result;

/* int next(token_type, ); */

ast_result parse_node(token_node *lexer_node) { 
  token tk = lexer_node->token;
  ast_result res = { .error = NULL };
  node n = { };
  switch(tk.type) {
    case TK_STRING: {
      n.type = JSON_STRING;
      break;
    }
    case TK_IDENTIFIER: {
      if (strcmp(tk.value, "null")) {
        n.type = JSON_NULL;
        break;
      }
      if (strcmp(tk.value, "false")) {
        n.type = JSON_BOOL;
        /* n.value = false; */
        break;
      }
      if (strcmp(tk.value, "true")) {
        n.type = JSON_BOOL;
        /* n.value = true; */
        break;
      }
    }
    case TK_LBRACE: {
      n.type = JSON_OBJECT;
      int nest_level = 1;
      while (nest_level) {
        lexer_node = lexer_node->next;

        switch (lexer_node->token.type) {
          case TK_RBRACE: {
            nest_level--;
            break;
          }
          case TK_STRING: {
            if (lexer_node->next->token.type != TK_COLON) {
              n.type = JSON_ERROR;
              res.error = malloc(MAX_JSON_ERROR_SIZE);
              res.error = sprintf(
                res.error,
                "Error parsing object. Colon expected: %d, %d",
                lexer_node->next->token.line,
                lexer_node->next->token.column);
              return res;
            }
            parse_node(lexer_node->next);
            res.ast_node = n;
            return res;
          }
          default: {
            n.type = JSON_ERROR;
            res.error = malloc(MAX_JSON_ERROR_SIZE);
            snprintf(
              res.error,
              MAX_JSON_ERROR_SIZE,
              "Error parsing object: %d, %d",
              lexer_node->token.line,
              lexer_node->token.column);
            res.ast_node = n;
            return res;
          }
        }
      }
    }
  }

  res.ast_node = n;
  return res;
};

void parse(token_list *tokens) {
  ast_result res = parse_node(tokens->tail);
  printf("AST: %s\n", get_ast_node_type_name(res.ast_node.type));
}









int main(int argv, char **argc) {
  FILE *input = fopen("./test.json", "r");
  if (!input) {
    fprintf(stderr, "Error opening file\n");
  }
  //TODO: erorr handle here
  fseek(input, 0, SEEK_END);
  int input_size = ftell(input);
  fseek(input, 0, 0);

  char *buffer = malloc(input_size);
  if (buffer == NULL) {
    fprintf(stderr, "Error allocating buffer\n");
  }

  int count = 0;
  char ch;

  do {
    ch = fgetc(input);
    buffer[count++] = ch;
  } while (ch != EOF);
  buffer[count] = '\0';

  tokenize_result res = tokenize(buffer, input_size, 0);
  if (res.status != SUCCESS) {
    fprintf(stderr, "Error parsing json:\n");
    fprintf(stderr, "%s", res.error);
    exit(1);
  }

  print_list(res.tokens);
  parse(res.tokens);
}
