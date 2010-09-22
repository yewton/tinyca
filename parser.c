/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "abssyn.h"
#include "tinycaml.h"
#include "type_inference.h"

int parse_bexp(BexpPtr);
int parse_exp(ExpPtr);

/**
 * 文法エラーを表示する
 * @return int main 関数の返り値
 */
void print_error(Token token) {
    fprintf(stderr, "Parser error at near '%s'.\n", token.str);
    return;
}

/**
 * F' 式のパースを行う
 * @param ExpsPtr esp パース結果を保存する式リスト型へのポインタ
 * @return int 成功:1 失敗:0 最後の引数だった:-1
 */
int parse_F_1(ExpsPtr esp) {
    Token token = {0};
    int result = 1;

    token = get_token();
    switch(token.t) {
    case COMMA_TOKEN:
        esp->elm = exp_alloc();
        if(0 == parse_exp(esp->elm)) return 0;
        esp->next = exps_alloc();
        switch(parse_F_1(esp->next)) {
        case 0:
            /* 異常 */
            result = 0;
            break;
        case -1:
            /* もう引数がない */
            free(esp->next);
            esp->next = NULL;
            break;
        default:
            /* 何もしない */
            break;
            return 0;
        }
        break;
    case RPAREN_TOKEN:
        result = -1;
        break;
    default:
        print_error(token);
        result = 0;
        break;
    }
    return result;
}

/**
 * F 式のパースを行う
 * @param ExpPtr ep パース結果を保存する式型へのポインタ
 * @return int      成否
 */
int parse_F(ExpPtr ep) {
    Token token = {0};
    char tmp[MAX_VAR_NAME_LENGTH];
    
#ifdef _DEBUG
    printf("parsing F...\n");
#endif
    token = get_token();
    switch(token.t) {
    case INT_TOKEN:
        /* 整数トークンだったら */
        ep->t = INT_EXP;
        ep->of.intval = token.of.intval;
        break;
    case FLOAT_TOKEN:
        /* 小数点数トークンだったら */
        ep->t = FLOAT_EXP;
        ep->of.floatval = token.of.floatval;
        break;
    case ID_TOKEN:
        /* 識別子トークンだったら */
        strncpy_s(tmp, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
        token = get_token();
        if(LPAREN_TOKEN == token.t) {
            /* 関数適用 */
            ep->t = APP_EXP;
            strncpy_s(ep->of.App.var, MAX_TOKEN_SIZE, tmp, strlen(tmp));
            ep->of.App.exps = exps_alloc();
            ep->of.App.exps->elm = exp_alloc();
            if(0 == parse_exp(ep->of.App.exps->elm)) return 0;
            ep->of.App.exps->next = exps_alloc();
            switch(parse_F_1(ep->of.App.exps->next)) {
            case 0:
                /* 異常 */
                return 0;
                break;
            case -1:
                /* もう引数がない */
                free(ep->of.App.exps->next);
                ep->of.App.exps->next = NULL;
                break;
            default:
                /* 何もしない */
                break;
            }
        } else {
            /* 変数 */
            unget_token();
            ep->t = VAR_EXP;
            strncpy_s(ep->of.var, MAX_TOKEN_SIZE, tmp, strlen(tmp));
        }
        break;
    case LPAREN_TOKEN:
        /* 左括弧トークンだったら */
        if(0 == parse_exp(ep)) return 0;
        token = get_token();
        if(RPAREN_TOKEN != token.t) return 0;
        break;
    default:
        print_error(token);
        return 0;
    }
#ifdef _DEBUG
    printf("finish parsing F.\n");
#endif
    return 1;
}

/**
 * T' 式のパースを行う
 * @param ExpPtr ep1   パース結果を保存するプログラム型へのポインタ
 * @return int       成否
 */
int parse_T_1(ExpPtr ep) {
    Token token = {0};
    ExpPtr ep1 = NULL;
    ExpPtr ep2 = NULL;

#ifdef _DEBUG
    printf("parsing T'...\n");
#endif
    token = get_token();
    switch(token.t) {
    case TIMES_TOKEN:
    case FTIMES_TOKEN:
        /* 式の領域を確保 */
        ep1 = exp_alloc();
        ep2 = exp_alloc();
        /* 左辺コピー */
        *ep1 = *ep;
        /* 右辺パース */
        if(parse_F(ep2) == 0) {
            return 0;
        }
        ep->t = PRIM_EXP;
        ep->of.Prim.exp1 = ep1;
        ep->of.Prim.exp2 = ep2;
        break;
    default:
        unget_token();
        return 1;
    }
    
    switch(token.t) {
    case TIMES_TOKEN:
        ep->of.Prim.op = MUL_OP;
    case FTIMES_TOKEN:
        ep->of.Prim.op = FMUL_OP;
        break;
    default:
        break;
    }
    if(parse_T_1(ep) == 0) {
        return 0;
    }

#ifdef _DEBUG
    printf("finish parsing T'.\n");
#endif
    return 1;
}

/**
 * T 式のパースを行う
 * @param ExpPtr ep パース結果を保存するプログラム型へのポインタ
 * @return int      成否
 */
int parse_T(ExpPtr ep) {
#ifdef _DEBUG
    printf("parsing T...\n");
#endif
    if(parse_F(ep) == 0) {
        return 0;
    }
    if(parse_T_1(ep) == 0) {
        return 0;
    }

#ifdef _DEBUG
    printf("finish parsing T.\n");
#endif
    return 1;
}

/**
 * E' 式のパースを行う
 * @param ExpPtr ep   パース結果を保存するプログラム型へのポインタ
 * @return int       成否
 */
int parse_E_1(ExpPtr ep) {
    Token token = {0};
    ExpPtr ep1 = NULL;
    ExpPtr ep2 = NULL;

#ifdef _DEBUG
    printf("parsing E'...\n");
#endif
    token = get_token();
    switch(token.t) {
    case PLUS_TOKEN:
    case FPLUS_TOKEN:
    case MINUS_TOKEN:
    case FMINUS_TOKEN:
        /* 式の領域を確保 */
        ep1 = exp_alloc();
        ep2 = exp_alloc();
        /* 左辺コピー */
        *ep1 = *ep;
        /* 右辺パース */
        if(parse_T(ep2) == 0) {
            return 0;
        }
        ep->t = PRIM_EXP;
        ep->of.Prim.exp1 = ep1;
        ep->of.Prim.exp2 = ep2;
        break;
    case SEMICOLON_TOKEN:
        /* 文の終わり */
        return 1;
        break;
    default:
        unget_token();
        return 1;
        break;
    }
        
    switch(token.t) {
    case PLUS_TOKEN:
        ep->of.Prim.op = PLUS_OP;
        break;
    case FPLUS_TOKEN:
        ep->of.Prim.op = FPLUS_OP;
        break;
    case MINUS_TOKEN:
        ep->of.Prim.op = MINUS_OP;
        break;
    case FMINUS_TOKEN:
        ep->of.Prim.op = FMINUS_OP;
        break;
    default:
        break;
    }
    if(parse_E_1(ep) == 0) {
        return 0;
    }
#ifdef _DEBUG
    printf("finish parsing E'.\n");
#endif
    return 1;
}

/**
 * E 式のパースを行う
 * @param ExpPtr ep パース結果を保存するプログラム型へのポインタ
 * @return int      成否
 */
int parse_E(ExpPtr ep) {
#ifdef _DEBUG
    printf("parsing E...\n");
#endif
    if(parse_T(ep) == 0) {
        return 0;
    }
    if(parse_E_1(ep) == 0) {
        return 0;
    }
#ifdef _DEBUG
    printf("finish parsing E.\n");
#endif
    return 1;
}

/**
 * 式のパースを行う
 * @param ExpPtr ep パース結果を保存する式型へのポインタ
 * @return int      成否
 */
int parse_exp(ExpPtr ep) {
    Token token = {0};
    int result = 1;
    
#ifdef _DEBUG
    printf("parsing Exp...\n");
#endif
    token = get_token();
    switch(token.t) {
    case IF_TOKEN:
        ep->t = IF_EXP;
        ep->of.If.bp = bexp_alloc();
        ep->of.If.exp1 = exp_alloc();
        ep->of.If.exp2 = exp_alloc();
        if(0 == parse_bexp(ep->of.If.bp)) {
            return 0;
        }
        token = get_token();
        switch(token.t) {
        case THEN_TOKEN:
            if(0 == parse_exp(ep->of.If.exp1)) {
                return 0;
            }
            token = get_token();
            if(ELSE_TOKEN == token.t) {
                if(0 == parse_exp(ep->of.If.exp2)) {
                    return 0;
                }
            } else {
                print_error(token);
                return 0;
            }
            break;
        default:
            print_error(token);
            return 0;
        }
        break;
    case LET_TOKEN:
        ep->t = LET_EXP;
        ep->of.Let.exp1 = exp_alloc();
        ep->of.Let.exp2 = exp_alloc();
        token = get_token();
        if(ID_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        strncpy_s(ep->of.Let.tvar.var, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
        ep->of.Let.tvar.ttp = tyvar_alloc();
        token = get_token();
        if(EQ_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        if(0 == parse_exp(ep->of.Let.exp1)) {
            return 0;
        }
        token = get_token();
        if(IN_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        if(0 == parse_exp(ep->of.Let.exp2)) {
            return 0;
        }
        break;
    default:
        /* E 式として構文解析 */
        unget_token();
        result = parse_E(ep);
        break;
    }
#ifdef _DEBUG
    printf("finish parsing Exp.\n");
#endif
    return result;
}

/**
 * ブール式のパースを行う
 * @param BexpPtr bp パース結果を保存するブール式型へのポインタ
 * @return int 成否
 */
int parse_bexp(BexpPtr bp) {
    Token token = {0};
    ExpPtr exp1 = NULL, exp2 = NULL;
    int result = 1;
#ifdef _DEBUG
    printf("parsing Bexp...\n");
#endif

    exp1 = exp_alloc();
    exp2 = exp_alloc();

    if(0 == parse_exp(exp1)) {
        return 0;
    }
    
    token = get_token();
    switch(token.t) {
    case EQ_TOKEN:
        if(0 == parse_exp(exp2)) {
            return 0;
        }
        bp->t = EQ_BEXP;
        bp->ttp = tyvar_alloc();
        bp->exp1 = exp1;
        bp->exp2 = exp2;
        break;
    default:
        return 0;
        break;
    }
#ifdef _DEBUG
    printf("finish parsing Bexp.\n");
#endif
    return 1;
}

/**
 * 変数リストのパースを行う
 * @param ArgsPtr ap パース結果を保存する変数リスト型へのポインタ
 * @return int 成功:1 失敗:0 最後の引数だった:-1
 */
int parse_args(ArgsPtr ap) {
    Token token = {0};
    int result = 1;

#ifdef _DEBUG
    printf("parsing Args...\n");
#endif

    token = get_token();
    switch(token.t) {
    case COMMA_TOKEN:
        token = get_token();
        if(ID_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        strncpy_s(ap->tvar.var, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
        ap->tvar.ttp = tyvar_alloc();
        ap->next = args_alloc();
        switch(parse_args(ap->next)) {
        case 0:
            /* 異常 */
            result = 0;
            break;
        case -1:
            /* もう引数がない */
            free(ap->next);
            ap->next = NULL;
            break;
        default:
            /* 何もしない */
            break;
        }
        break;
    case RPAREN_TOKEN:
        result = -1;
        break;
    default:
        print_error(token);
        result = 0;
        break;
    }
    return result;
}

/**
 * プログラムのパースを行う
 * @param ProgPtr pp パース結果を保存するプログラム型へのポインタ
 * @return int       成否
 */
int parse_prog(ProgPtr pp) {
    Token token = {0};
    ExpPtr ep = NULL;
    ArgsPtr ap = NULL;
    int result = 1;

#ifdef _DEBUG
    printf("parsing Prog...\n");
#endif
    token = get_token();
    switch(token.t) {
    case LET_TOKEN:
        token = get_token();
        if(REC_TOKEN != token.t) {
            /*
             * let rec 文ではなかったので let 式として解析
             * 式の解析部分と同じなのでなんとかしたかった
             */
            unget_token();
            pp->t = EXP;
            pp->of.exp = exp_alloc();
            pp->of.exp->t = LET_EXP;
            pp->of.exp->of.Let.exp1 = exp_alloc();
            pp->of.exp->of.Let.exp2 = exp_alloc();
            token = get_token();
            if(ID_TOKEN != token.t) {
                print_error(token);
                return 0;
            }
            strncpy_s(pp->of.exp->of.Let.tvar.var, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
            pp->of.exp->of.Let.tvar.ttp = tyvar_alloc();
            token = get_token();
            if(EQ_TOKEN != token.t) {
                print_error(token);
                return 0;
            }
            if(0 == parse_exp(pp->of.exp->of.Let.exp1)) {
                return 0;
            }
            token = get_token();
            if(IN_TOKEN != token.t) {
                print_error(token);
                return 0;
            }
            if(0 == parse_exp(pp->of.exp->of.Let.exp2)) {
                return 0;
            }
            break;
        }
        pp->t = LETREC;
        token = get_token();
        if(ID_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        /* 関数名保持 */
        strncpy_s(pp->of.LetRec.tvar.var, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
        pp->of.LetRec.tvar.ttp = tyvar_alloc();
        token = get_token();
        if(LPAREN_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        token = get_token();
        if(ID_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        /* 引数保持 */
        pp->of.LetRec.args = args_alloc();
        strncpy_s(pp->of.LetRec.args->tvar.var, MAX_TOKEN_SIZE, token.of.var, strlen(token.of.var));
        pp->of.LetRec.args->tvar.ttp = tyvar_alloc();
        pp->of.LetRec.args->next = args_alloc();
        switch(parse_args(pp->of.LetRec.args->next)) {
        case 0:
            /* 異常 */
            result = 0;
            break;
        case -1:
            /* 引数がひとつだけだった */
            free(pp->of.LetRec.args->next);
            pp->of.LetRec.args->next = NULL;
            break;
        default:
            /* 何もしない */
            break;
        }
        token = get_token();
        if(EQ_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        /* 式パース */
        pp->of.LetRec.exp = exp_alloc();
        if(0 == parse_exp(pp->of.LetRec.exp)) {
            result = 0;
            break;
        }
        token = get_token();
        if(IN_TOKEN != token.t) {
            print_error(token);
            result = 0;
            break;
        }
        /* プログラムパース */
        pp->of.LetRec.prog = prog_alloc();
        if(0 == parse_prog(pp->of.LetRec.prog)) {
            result = 0;
            break;
        }
        break;
    default:
        /* 式として構文解析 */
        unget_token();
        ep = exp_alloc();
        pp->t = EXP;
        pp->of.exp = ep;
        result = parse_exp(ep);
        break;
    }
#ifdef _DEBUG
    printf("finish parsing Prog.\n");
#endif
    return result;
}
