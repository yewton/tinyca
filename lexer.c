/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "token.h"
#include "input.h"

static Token st_look_ahead_token; /* 先読みトークン */
static int st_look_ahead_token_exists = 0; /* 先読みトークンの存在フラグ */

/* 予約語 */
char * KEYWORDS[] = {"let", "in", "rec", "if", "then", "else", NULL};

typedef enum {
    LET_KEYWORD = 0,
    IN_KEYWORD,
    REC_KEYWORD,
    IF_KEYWORD,
    THEN_KEYWORD,
    ELSE_KEYWORD
} KeywordType;

/*
 * レキサーの状態を表す列挙型
 */
typedef enum {
    INITIAL_STATUS,
    IN_INT_PART_STATUS,
    DOT_STATUS,
    IN_FRAC_PART_STATUS,
    IN_ID_PART_STATUS,
    SHARP_STATUS,
    IN_COMMAND_STATUS
} LexerStatus;

/**
 * 次のトークンを取得
 * @return Token トークン
 */
Token next_token(void) {
    LexerStatus status = INITIAL_STATUS;
    Token token = {BAD_TOKEN, "", 0};
    int c = 0;
    int out_pos = 0;
    int i = 0;
    int is_keyword = 0;
    char str[MAX_TOKEN_SIZE] = "";

    while( EOF != (c = getc(fp)) ) {
        if ((status == IN_INT_PART_STATUS)
            && !isdigit(c) && c != '.') {
            /* 整数部分 */
            ungetc(c, fp);
            sscanf_s(str, "%d", &token.of.intval, sizeof(str));
            token.t = INT_TOKEN;
            break;
        }
        if((status == IN_FRAC_PART_STATUS)
           && !isdigit(c)) {
            /* 小数点数部分 */
            ungetc(c, fp);
            sscanf_s(str, "%f", &token.of.floatval, sizeof(str));
            token.t = FLOAT_TOKEN;
            break;
        }
        if((status == DOT_STATUS) && !isdigit(c)) {
            /* 小数点部分 */
            ungetc(c, fp);
            /* 0 を補って少数として解釈する */
            str[out_pos] = '0';
            ++out_pos;
            str[out_pos] = '\0';
            sscanf_s(str, "%f", &token.of.floatval, sizeof(str));
            token.t = FLOAT_TOKEN;
            break;
        }
        if ((status == IN_ID_PART_STATUS)
            && !isalpha(c) && !isdigit(c)) {
            /* 識別子部分 */
            ungetc(c, fp);
            for(i = 0; KEYWORDS[i] != NULL && 0 == is_keyword; ++i) {
                /* キーワードか判定 */
                if(strcmp(KEYWORDS[i], str) == 0) {
                    switch(i) {
                    case LET_KEYWORD:
                        token.t = LET_TOKEN;
                        break;
                    case IN_KEYWORD:
                        token.t = IN_TOKEN;
                        break;
                    case REC_KEYWORD:
                        token.t = REC_TOKEN;
                        break;
                    case IF_KEYWORD:
                        token.t = IF_TOKEN;
                        break;
                    case THEN_KEYWORD:
                        token.t = THEN_TOKEN;
                        break;
                    case ELSE_KEYWORD:
                        token.t = ELSE_TOKEN;
                        break;
                    }
                    is_keyword = 1;
                }
            }
            if (0 == is_keyword) {
                token.t = ID_TOKEN;
                strncpy_s(token.of.var, MAX_TOKEN_SIZE, str, strlen(str));
            }
            break;
        }
        if ( (status == IN_COMMAND_STATUS)
             && !(isalpha(c)) && !isdigit(c) ) {
            /* 特殊命令部分 */
            if(';' == c) {
                if(';' == (c = getc(fp))) {
                    if( strcmp("#quit", str) == 0) {
                        exit(0);
                    } else {
                        /* それ以外の時に来たらエラー */
                        fprintf(stderr, "bad character(%c)\n", c);
                        break;
                    }                       
                } else {
                    /* それ以外の時に来たらエラー */
                    fprintf(stderr, "bad character(%c)\n", c);
                    break;
                }
            } else {
                /* それ以外の時に来たらエラー */
                fprintf(stderr, "bad character(%c)\n", c);
                break;
            }
        }
        if(isspace(c)) {
            /* 空白は読み飛ばす */
            continue;
        }
        if (out_pos >= MAX_TOKEN_SIZE-1) {
            /* 長すぎるトークンはエラー */
            fprintf(stderr, "token too long.\n"); 
            break;
        }
        str[out_pos] = (char)c;
        /* 終端文字代入。次のループで上書きされて、また終端文字が挿入される */
        ++out_pos;
        str[out_pos] = '\0';
        if ('+' == c ) {
            if('.' == (c = getc(fp))) {
                token.t = FPLUS_TOKEN;
            } else {
                ungetc(c, fp);
                token.t = PLUS_TOKEN;
            }
            break;
        } else if ('-' == c ) {
            if('.' == (c = getc(fp))) {
                token.t = FMINUS_TOKEN;
            } else {
                ungetc(c, fp);
                token.t = MINUS_TOKEN;
            }
            break;
        } else if ('*' == c) {
            if('.' == (c = getc(fp))) {
                token.t = FTIMES_TOKEN;
            } else {
                ungetc(c, fp);
                token.t = TIMES_TOKEN;
            }
            break;
        } else if (';' == c) {
            if(';' == (c = getc(fp))) {
                token.t = SEMICOLON_TOKEN;
            } else {
                /* エラー */
                fprintf(stderr, "bad character(;)\n");
            }
            break;
        }else if ('(' == c) {
            token.t = LPAREN_TOKEN;
            break;
        } else if (')' == c) {
            token.t = RPAREN_TOKEN;
            break;
        } else if ('=' == c) {
            token.t = EQ_TOKEN;
            break;
        } else if (',' == c) {
            token.t = COMMA_TOKEN;
            break;
        } else if (isdigit(c)) {
            if (status == INITIAL_STATUS) {
                status = IN_INT_PART_STATUS;
            } else if (status == DOT_STATUS) {
                status = IN_FRAC_PART_STATUS;
            }
        } else if ('.' == c) {
            if (status == IN_INT_PART_STATUS) {
                /* 整数解析中に . が来たら小数点 */
                status = DOT_STATUS;
            } else {
                /* それ以外の時に来たらエラー */
                fprintf(stderr, "bad character(%c)\n", c);
            }
        } else if (isalpha(c)) {
            if (status == SHARP_STATUS) {
                status = IN_COMMAND_STATUS;
            } else if (status == INITIAL_STATUS) {
                status = IN_ID_PART_STATUS;
            }
        } else if ('#' == c) {
            if (status == INITIAL_STATUS) {
                status = SHARP_STATUS;
            } else {
                /* それ以外の時に来たらエラー */
                fprintf(stderr, "bad character(%c)\n", c);
            }          
        }else {
            /* トークンになりえない文字が来たらエラー */
            fprintf(stderr, "bad character(%c)\n", c);
        }
    }
    if(c == EOF) {
        printf("bye.\n");
        exit(0);
    }
    strncpy_s(token.str, MAX_TOKEN_SIZE, str, strlen(str));
    return token;
}

/**
 * トークンを取得
 * @return Token トークン
 */
Token get_token(void) {
    Token token;
    if(st_look_ahead_token_exists) {
        /* 
         * 先読みトークンがあったらそれを返し、
         * 先読みトークン存在フラグを下げる
         */
        st_look_ahead_token_exists = 0;
        return st_look_ahead_token;
    } else {
        /*
         * 先読みトークンがなかったらトークンを取得して
         * 先読みトークンにセットし、返す 
         */
        token = next_token();
        st_look_ahead_token = token;
        return token;
    }
}

/**
 * トークンを押し戻す
 * 
 * 実際には先読みトークン存在フラグを立てるだけ
 */
void unget_token(void) {
    st_look_ahead_token_exists = 1;
}
