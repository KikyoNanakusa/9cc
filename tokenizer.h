#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include "utils.h"

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
    TK_RETURN,
    TK_SIZEOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val; // If kind is TK_NUM, its number
    char *str;
    int len; // Token length
};

extern Token *token;

Token *tokenize(char *p);
bool consume(char *op);
Token *consume_return();
Token *consume_ident();
Token *consume_sizeof();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();

#endif

