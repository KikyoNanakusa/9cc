#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>

typedef enum {
    TK_RESERVED,
    TK_IDENT,
    TK_NUM,
    TK_EOF,
    TK_RETURN,
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
void expect(char *op);
int expect_number();
bool at_eof();

#endif

