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

typedef struct {
  char **properties;
} json_object;

typedef struct {
  struct node_list *items;
} json_array;

typedef struct {
  float value; // TODO: should this be a double? Check the RFC.
} json_number;

typedef struct {
  char *value;
} json_string;

typedef struct {
  bool value;
} json_bool;

typedef struct {
  void *value;
} json_null;

typedef union {
  json_object j_object;
  json_array j_array;
  json_number j_number;
  json_string j_string;
  json_bool j_bool;
  json_null j_null;
} json_item;

typedef struct node {
  struct node *next;
  node_type type;
  json_item *value;
} node;

typedef struct node_list {
  node *tail;
  node *head;
} node_list;

void new_node(node_list *list, node *nod) {
  if (!list->tail) {
    list->tail = nod;
    list->head = nod;
  } else {
    list->head->next = nod;
    list->head = nod;
  }
}

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
      last_newline = idx + 1;
      line++;
      continue;
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
              && buf[idx]   != ','
              && buf[idx]   != EOF
              && buf[idx]   != '\n') { }
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
  do {
    printf(
      "%d,%d:%s:\t\t%s\n",
      tokens->token.line,
      tokens->token.column,
      get_token_type_name(tokens->token.type),
      tokens->token.value);
    tokens = tokens->next;
  } while (tokens);
}

typedef struct {
  char *error;
  node *ast_node;
  token_node *last_node;
} ast_result;

/* int next(token_type, ); */

ast_result parse_node(token_node *lexer_node) { 
  token tk = lexer_node->token;
  ast_result res = { .error = NULL };
  node *n = malloc(sizeof(node));
  res.last_node = lexer_node;
  switch(tk.type) {
    case TK_STRING: {
      n->type = JSON_STRING;

      n->value = malloc(sizeof(json_string));
      n->value->j_string.value = tk.value;
      break;
    }
    case TK_IDENTIFIER: {
      if (strcmp(tk.value, "null") == 0) {
        n->type = JSON_NULL;
        n->value = malloc(sizeof(json_null));
        // TODO: NULL assignment considered harmful?
        n->value->j_null.value = NULL;
        break;
      }
      if (strcmp(tk.value, "false") == 0) {
        n->type = JSON_BOOL;
        n->value = malloc(sizeof(json_bool));
        n->value->j_bool.value = false;
        break;
      }
      if (strcmp(tk.value, "true") == 0) {
        n->type = JSON_BOOL;
        n->value = malloc(sizeof(json_bool));
        n->value->j_bool.value = true;
        break;
      }
      n->type = JSON_NUMBER;
      n->value = malloc(sizeof(json_number));
      n->value->j_number.value = atof(tk.value);
      break;
    }
    case TK_LBRACE: {
      n->type = JSON_OBJECT;
      int nest_level = 1;
      while (nest_level) {
        lexer_node = lexer_node->next;

        switch (lexer_node->token.type) {
          case TK_RBRACE: {
            nest_level--;
            break;
          }
          /* case TK_STRING: { */
          /*   if (lexer_node->next->token.type != TK_COLON) { */
          /*     n.type = JSON_ERROR; */
          /*     res.error = malloc(MAX_JSON_ERROR_SIZE); */
          /*     sprintf( */
          /*       res.error, */
          /*       "Error parsing object. Colon expected: %d, %d", */
          /*       lexer_node->next->token.line, */
          /*       lexer_node->next->token.column); */
          /*     return res; */
          /*   } */
          /*   // Print the type of the next next lexer node. */
          /*   printf("Next next: %s\n", get_token_type_name(lexer_node->next->next->token.type)); */
          /*   ast_result nod = parse_node(lexer_node->next->next); */
          /*   #<{(| lexer_node = nod.last_node; |)}># */
          /*   printf("Next node: %s\n", get_token_type_name(nod.last_node->token.type)); */
          /*   printf("AST: %s\n", get_ast_node_type_name(nod.ast_node.type)); */
          /*   res.ast_node = n; */
          /* } */
          /* default: { */
          /*   n.type = JSON_ERROR; */
          /*   res.error = malloc(MAX_JSON_ERROR_SIZE); */
          /*   snprintf( */
          /*     res.error, */
          /*     MAX_JSON_ERROR_SIZE, */
          /*     "Error parsing object: %d, %d", */
          /*     lexer_node->token.line, */
          /*     lexer_node->token.column); */
          /*   res.ast_node = n; */
          /*   return res; */
          /* } */
        }
      }

      json_item *j_item = malloc(sizeof(json_item));
      char *test[2] = { "test", "test2" };
      j_item->j_object.properties = test;
      n->value = j_item;
      res.last_node = lexer_node;
      break;
    }
    case TK_LBRACKET: {
      n->type = JSON_ARRAY;
      n->value = malloc(sizeof(json_array));
      n->value->j_array.items = malloc(sizeof(node_list));

      int nest_level = 1;
      while (nest_level) {
        lexer_node = lexer_node->next;
        if (lexer_node->token.type == TK_RBRACKET) {
          nest_level--;
          continue;
        }

        if (lexer_node->token.type == TK_COMMA) {
          continue;
        }

        ast_result nod = parse_node(lexer_node);
        if (nod.error) {
          res.error = nod.error;
          return res;
        }

        if (nod.ast_node->type == JSON_UNKOWN) {
          n->type = JSON_ERROR;
          res.error = malloc(MAX_JSON_ERROR_SIZE);
          snprintf(
            res.error,
            MAX_JSON_ERROR_SIZE,
            "Error parsing array: %d, %d",
            lexer_node->token.line,
            lexer_node->token.column);
          return res;
        }

        new_node(n->value->j_array.items, nod.ast_node);
        printf("nod type: %s\n", get_ast_node_type_name(nod.ast_node->type));

        lexer_node = nod.last_node;
      }

      res.last_node = lexer_node;
      break;
    }
  }

  res.ast_node = n;
  return res;
};

void parse(token_list *tokens) {
  ast_result res = parse_node(tokens->tail);
  node *array = res.ast_node->value->j_array.items->tail;
  while (array) {
    printf("Array item: %s\n", get_ast_node_type_name(array->type));
    if (array->type == JSON_STRING) {
      printf("%s\n", array->value->j_string.value);
    }
    if (array->type == JSON_BOOL) {
      printf("%d\n", array->value->j_bool.value);
    }
    if (array->type == JSON_NULL) {
      printf("null\n");
    }
    if (array->type == JSON_NUMBER) {
      printf("%f\n", array->value->j_number.value);
    }
    array = array->next;
  }

  /* printf( */
  /*   "%d\n", */
  /*   res.ast_node->value->j_array.items->head->value->j_bool.value); */
  /* printf( */
  /*   "%s\n", */
  /*   res.ast_node->value->j_array.items->tail->value->j_string.value); */
  /* printf("AST: %s\n", get_ast_node_type_name(res.ast_node->type)); */
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
