#include "tokenizer.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


// Create a new token
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
  tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

bool is_alpha(char c) {
  return ('a' <= c && c <='z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alpha_num(char c) {
  return is_alpha(c) || ('0' <= c && c <= '9');
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

    if (startswith(p, "sizeof")) {
      cur = new_token(TK_SIZEOF, cur, p, strlen("sizeof"));
      p += strlen("sizeof");
      continue;
    }

    if(startswith(p, "return") && !is_alpha_num(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (startswith(p, "while")) {
      cur = new_token(TK_RESERVED, cur, p, 5);
      p += 5;
      continue;
    }

    if (startswith(p, "for")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("for"));
      p += strlen("for");
      continue;
    }

    if (startswith(p, "int")) {
      cur = new_token(TK_RESERVED, cur, p, 3);
      p += 3;
      continue;
    }

    if(startswith(p, "if")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if(startswith(p, "else")) {
      cur = new_token(TK_RESERVED, cur, p, 4);
      p += 4;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

		if (strchr("+-*/=;()<>{},&", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

    if (is_alpha(*p)) {
      char *q = p;
      while (is_alpha_num(*p)) {
        p++;
      }
      cur = new_token(TK_IDENT, cur, q, p - q);
      continue;
    }
    
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
			cur->val = strtol(p, &p, 10);
      cur->len = p - q;
			continue;
		}

    error_at(p, "Invalid token");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

// If the next token is the expected symbol, read one token and return true.
// Otherwise, return false.
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
		return false;
	}
	token = token->next;
	return true;
}

Token *consume_return() {
  if (token->kind != TK_RETURN) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_sizeof() {
  if (token->kind != TK_SIZEOF) {
    return NULL;
  }

  Token *tok = token;
  token = token->next;
  return tok;
}

// If the next token is the expected symbol, read one token.
// Otherwise, it returns an error.
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
  {
		error_at(token->str, "expected '%c'", op);
	}
	token = token->next;
}

char *expect_ident() {
  if (token->kind != TK_IDENT) {
    error_at(token->str, "expected an indentifier");
  }

  char *s = strndup(token->str, token->len);
  token = token->next;
  return s;
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
