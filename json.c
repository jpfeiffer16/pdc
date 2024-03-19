#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*
  RFC: https://datatracker.ietf.org/doc/html/rfc8259#section-2
*/

typedef enum {
  OBJECT,
  ARRAY,
  NUMBER,
  STRING,
  BOOL
} node_type;

/* typedef struct { */
/*   node_type type; */
/*  */
/*   // Value accessors */
/*   char **str; */
/*   int *num; */
/*   short *bool; */
/*  */
/*   node *children; */
/* } node; */

typedef enum {
  NONE,
  LBRACE,
  RBRACE,
  LBRACKET,
  RBRACKET,
  IDENTIFIER,
  STR,
  COLON,
  COMMA
} token_type;

char* get_token_type_name(token_type token) {
  switch (token) {
    case NONE:       return "NONE";
    case LBRACE:     return "LBRACE";
    case RBRACE:     return "RBRACE";
    case LBRACKET:   return "LBRACKET";
    case RBRACKET:   return "RBRACKET";
    case IDENTIFIER: return "IDENTIFIER";
    case STR:        return "STR";
    case COLON:      return "COLON";
    case COMMA:      return "COMMA";
    default:         return "UNKNOWN!";
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
        token tk = { .type = LBRACE, .value = "{", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '}': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = RBRACE, .value = "}", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '[': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = LBRACKET, .value = "[", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ']': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = RBRACKET, .value = "]", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ',': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = COMMA, .value = ",", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case ':': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = COLON, .value = ":", .line = line, .column = idx - last_newline };
        tk_l->token = tk;
        break;
      }
      case '"': {
        token_node *tk_l = new_token(tlist);
        int begin = idx;
        token tk = { .type = STR, .line = line, .column = idx - last_newline };
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
          token tk = { .type = IDENTIFIER, .line = line, .column = idx - last_newline };
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

void parse(token_list tokens) {
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
  if (res.status == SUCCESS) {
    print_list(res.tokens);
  } else {
    fprintf(stderr, "Error parsing json:\n");
    fprintf(stderr, "%s", res.error);
  }
}
