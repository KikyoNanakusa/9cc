#include "utils.h"

extern char *file_path;
extern char *source_code;

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *msg) {
  char *line = loc;
  while(source_code < line && line[-1] != '\n') {
    line--;
  }

  char *end = loc;
  while (*end != '\n') {
    end++;
  }

  int line_num = 1;
  for (char *p = source_code; p < line; p++) {
    if (*p == '\n') {
      line_num++;
    }
  }

  int ident = fprintf(stderr, "%s:%d ", file_path, line_num);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  int pos = loc - line + ident;
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);

  exit(1);
}

size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}

char *strndup(const char *s, size_t n) {
    char *result;
    size_t len = strnlen(s, n);
    result = (char *)malloc(len + 1);
    if (!result) return NULL;
    result[len] = '\0';
    return (char *)memcpy(result, s, len);
}

