#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

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
  RBRACET,
  IDENTIFIER,
  STR,
  COLON,
  COMMA
} token_type;

typedef struct {
  token_type type;
  char *value;
} token;

typedef struct token_list {
  struct token_list *next;
  token token;
} token_list;

typedef struct {
  token_list *tokens;
  enum {
    SUCCESS, ERROR
  } status;
} parse_result;

parse_result parse(char *buf, long buf_len, int idx) {
  char ch = buf[idx];
  token_list *list = NULL;
  token_list *head = list;

  parse_result err = { .status = ERROR };

  do {
    switch (ch) {
      case '{': {
        token_list *tk_l = malloc(sizeof(token_list));
        token tk = { .type = LBRACE, .value = "{" };
        tk_l->token = tk;
        if (!list) {
          list = tk_l;
        } else {
          head->next = tk_l;
        }
        head = tk_l;
        break;
      }
      case '}': {
        token_list *tk_l = malloc(sizeof(token_list));
        token tk = { .type = LBRACE, .value = "}" };
        tk_l->token = tk;
        if (!list) {
          fprintf(stderr, "error, root elemnt must be obj or array");
          return err;
        } else {
          head->next = tk_l;
        }
        head = tk_l;
        break;
      }
      case '[': {
        token_list *tk_l = malloc(sizeof(token_list));
        token tk = { .type = LBRACKET, .value = "[" };
        tk_l->token = tk;
        if (!list) {
          list = tk_l;
        } else {
          head->next = tk_l;
        }
        head = tk_l;
        break;
      }
      case ']': {
        token_list *tk_l = malloc(sizeof(token_list));
        token tk = { .type = LBRACE, .value = "]" };
        tk_l->token = tk;
        if (!list) {
          fprintf(stderr, "error, root elemnt must be obj or array");
          return err;
        } else {
          head->next = tk_l;
        }
        head = tk_l;
        break;
      }
    }
  } while ((ch = buf[++idx]) != EOF);

  /* parse_result res = { .status = SUCCESS, .tokens = list }; */
  parse_result res;
  res.status = SUCCESS;
  res.tokens = list;
  return res;
}

int main(int argv, char **argc) {
  FILE *input = fopen("./test.json", "r");
  if (!input) {
    fprintf(stderr, "Error opening file");
  }
  //TODO: erorr handle here
  fseek(input, 0, SEEK_END);
  int input_size = ftell(input);
  fseek(input, 0, 0);

  char *buffer = malloc(input_size);
  if (buffer == NULL) {
    fprintf(stderr, "Error allocating buffer");
  }

  int count = 0;
  char ch;

  do {
    ch = fgetc(input);
    buffer[count++] = ch;
  } while (ch != EOF);
  buffer[count] = '\0';

  parse_result res = parse(buffer, input_size, 0);
}
