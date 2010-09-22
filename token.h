/* -*- coding: utf-8-dos; mode: C -*- */
#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

/* トークンのタイプ */
typedef enum {
  ID_TOKEN,
  INT_TOKEN,
  FLOAT_TOKEN,
  LPAREN_TOKEN,
  RPAREN_TOKEN,
  PLUS_TOKEN,    /* + */
  MINUS_TOKEN,   /* - */
  TIMES_TOKEN,   /* * */
  FPLUS_TOKEN,   /* +. */
  FMINUS_TOKEN,  /* -. */
  FTIMES_TOKEN,  /* *. */
  EQ_TOKEN,
  LET_TOKEN,
  IN_TOKEN,
  REC_TOKEN,
  IF_TOKEN,
  THEN_TOKEN,
  ELSE_TOKEN,
  COMMA_TOKEN,
  SEMICOLON_TOKEN,
  BAD_TOKEN
} TokenType;

/* トークンの最大サイズ */
#define MAX_TOKEN_SIZE (100)

/* トークン型 */
typedef struct {
  TokenType t;
  char str[MAX_TOKEN_SIZE];
  union {
    int intval;
    float floatval;
    char var[MAX_TOKEN_SIZE];
  }  of;
} Token;

/* 現在のトークン取得 */
Token get_token(void);
/* 現在のトークンを破棄 */
void unget_token(void);

#endif
