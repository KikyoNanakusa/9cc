#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

extern Token *token;

Token *tokenize(char *p);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();

#endif

