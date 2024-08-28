#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stddef.h>

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

char *strndup(const char *s, size_t n);
size_t strnlen(const char *s, size_t maxlen);
#endif
