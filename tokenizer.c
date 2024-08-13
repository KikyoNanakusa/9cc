#include "tokenizer.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


// Create a new token
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// Tokenize the input p and return it.
Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// Skip spaces
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

    error_at(p, "Invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

// If the next token is the expected symbol, read one token and return true.
// Otherwise, return false.
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		return false;
	}
	token = token->next;
	return true;
}

// If the next token is the expected symbol, read one token.
// Otherwise, it returns an error.
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		error_at(token->str, "expected '%c'", op);
	}
	token = token->next;
}

// If the next token is a number, read one token and return the value.
// Otherwise, it returns an error.
int expect_number() {
	if (token->kind != TK_NUM) {
		error_at(token->str, "expected_numner");
	}
	int val = token->val;
	token = token->next;
	return val;
}

// If the next token is a EOF, it returns true.
// Otherwise, it returns false.
bool at_eof() {
	return token->kind == TK_EOF;
}
