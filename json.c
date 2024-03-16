#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
  char ch = buf[idx];

  do {
    switch (ch) {
      case '{': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = LBRACE, .value = "{" };
        tk_l->token = tk;
        break;
      }
      case '}': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = RBRACE, .value = "}" };
        tk_l->token = tk;
        break;
      }
      case '[': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = LBRACKET, .value = "[" };
        tk_l->token = tk;
        break;
      }
      case ']': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = RBRACKET, .value = "]" };
        tk_l->token = tk;
        break;
      }
      case ',': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = COMMA, .value = "," };
        tk_l->token = tk;
        break;
      }
      case ':': {
        token_node *tk_l = new_token(tlist);
        token tk = { .type = COLON, .value = ":" };
        tk_l->token = tk;
        break;
      }
      case '"': {
        token_node *tk_l = new_token(tlist);
        int begin = ++idx;
        while (buf[++idx] != '"') {
          if (buf[idx] == '\\') idx++;
        }
        int val_size = ++idx - begin;
        --idx;
        char *val = malloc(val_size + 1);
        strncpy(val, (char*)buf + begin, val_size - 1);
        val[val_size] = '\0';
        token tk = { .type = STR, .value = val };
        tk_l->token = tk;
        break;
      }
      default: {
        if (ch > 32 && ch < 127) {
          token_node *tk_l = new_token(tlist);
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
          token tk = { .type = IDENTIFIER, .value = val };
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
      "%s:\t\t%s\n",
      get_token_type_name(tokens->token.type),
      tokens->token.value);
    tokens = tokens->next;
  }
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
