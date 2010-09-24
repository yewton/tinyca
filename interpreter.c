/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abssyn.h"
#include "parser.h"
#include "tinycaml.h"

int eval_bexp(int *result, BexpPtr bp, EnvPtr env);

/**
 * 式を評価する
 * @param  TC_Value *result 評価結果の格納先
 * @param  ExpPtr ep        式
 * @param  EnvPtr env       環境
 * @return int              成否
 */   
int eval_exp(TC_Value *result, ExpPtr ep, EnvPtr env) {
    TC_Value e1 = {0}, e2 = {0};
    int bexp_result = 0;
    
    if(NULL == ep) {
        return 0;
    }
    switch(ep->t) {
    case VAR_EXP:
        printf("VAR_EXP is not yet implemented.\n");
        return 0;
        break;
    case INT_EXP:
        result->t = INT;
        result->of.i = ep->of.intval;
        break;
    case FLOAT_EXP:
        result->t = FLOAT;
        result->of.f = ep->of.floatval;
        break;
    case PRIM_EXP:
        switch(ep->of.Prim.op) {
        case PLUS_OP:
        case FPLUS_OP:
            if(0 == eval_exp(&e1, ep->of.Prim.exp1, env)) return 0;
            if(0 == eval_exp(&e2, ep->of.Prim.exp2, env)) return 0;
            /* TODO ちゃんと考える */
            result->t = INT;
            result->of.i = e1.of.i + e2.of.i;
            break;
        case MINUS_OP:
        case FMINUS_OP:
            if(0 == eval_exp(&e1, ep->of.Prim.exp1, env)) return 0;
            if(0 == eval_exp(&e2, ep->of.Prim.exp2, env)) return 0;
            /* TODO ちゃんと考える */
            result->t = INT;
            result->of.i = e1.of.i - e2.of.i;
            break;
        case MUL_OP:
        case FMUL_OP:
            if(0 == eval_exp(&e1, ep->of.Prim.exp1, env)) return 0;
            if(0 == eval_exp(&e2, ep->of.Prim.exp2, env)) return 0;
            /* TODO ちゃんと考える */
            result->t = INT;
            result->of.i = e1.of.i * e2.of.i;
            break;
        }
        break;
    case LET_EXP:
        printf("LET_EXP is not yet implemented.\n");
        return 0;
        break;
    case IF_EXP:
        if(0 == eval_bexp(&bexp_result, ep->of.If.bp, env)) return 0;
        if(1 == bexp_result) {
            if(0 == eval_exp(result, ep->of.If.exp1, env)) return 0;
        } else if(0 == bexp_result) {
            if(0 == eval_exp(result, ep->of.If.exp2, env)) return 0;
        } else return 0;
        break;
    case APP_EXP:
        printf("APP_EXP is not yet implemented.\n");
        return 0;
        break;
    }
    return 1;
}

/**
 * ブール式を評価する
 * @param int *result 評価結果の格納先
 * @param ProgPtr pp  プログラム
 * @param EnvPtr env  環境
 * @return int        成否
 */   
int eval_bexp(int *result, BexpPtr bp, EnvPtr env) {
    TC_Value e1 = {0}, e2 = {0};
    if(NULL == bp) {
        return 0;
    }
    if(0 == eval_exp(&e1, bp->exp1, env)) return 0;        
    if(0 == eval_exp(&e2, bp->exp2, env)) return 0;
    /* TODO ちゃんと考える */
    if(e1.of.i == e2.of.i) {
        *result = 1;
    } else {
        *result = 0;
    }
    return 1;
}

/**
 * プログラムを評価する
 * @param TC_Value *result 評価結果の格納先
 * @param ProgPtr  pp      プログラム
 * @param EnvPtr   env     環境
 * @return int             成否
 */   
int eval_prog(TC_Value *result, ProgPtr pp, EnvPtr env) {
    if(NULL == pp) {
        return 0;
    }
    switch(pp->t) {
    case EXP:
        if(0 == eval_exp(result, pp->of.exp, env)) {
            return 0;
        }
        break;
    case LETREC:
        return 0;
        break;
    }
    return 1;
}

/**
 * プログラムを評価する
 * プログラムを初期化してパースし、
 * 結果を評価関数に渡す
 * @param  TC_Value *v 結果の格納先
 * @return int         成否
 */
int eval(TC_Value *v) {
    ProgPtr pp = NULL;
    EnvPtr env = NULL;
    TC_TypePtr ttp = NULL;
    EnvPtr ep = NULL;
    FEnvPtr fep = NULL;

    /* 構文解析を実行 */
    pp = prog_alloc();
    if(0 == parse_prog(pp)) return;
    env = env_alloc();
    if( NULL != (ttp = typing_prog(env, pp)) ) {
        printf("- : ");
        /* float 以外は（Unknown 型であろうと）int と見なす */
        (FLOAT_TYPE == norm(ttp)->t) ? printf("float") : printf("int");
        printf(" = ");
    } else {
        printf("typing error.");
    }
    ep = env_alloc();
    fep = fenv_alloc();
    eval_prog(v);
    printf("\n");
    

    return is_success;
}
