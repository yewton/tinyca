/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abssyn.h"
#include "parser.h"
#include "type_inference.h"

#define MESSAGE_MAX_BUF_SIZE 1024

int eval_bexp(int *result, BexpPtr bp, EnvPtr env, FEnvPtr fenv);

int find_value(TC_Value *result, char *v, EnvPtr env) {
    EnvPtr tmp = NULL;
    for(tmp = env; NULL != tmp; tmp = tmp->next) {
        if( strcmp(tmp->var, v) == 0) {
            *result = tmp->tv;
            return 1;
        }
    }
    return 0;
}

int find_func(Func *result, char *v, FEnvPtr fenv) {
    FEnvPtr tmp = NULL;
    
    for(tmp = fenv; NULL != tmp; tmp = tmp->next) {
        if( strcmp(tmp->var, v) == 0) {
            *result = tmp->f;
            return 1;
        }
    }
    return 0;
}

/**
 * 式を評価する
 * @param  TC_Value *result 評価結果の格納先
 * @param  ExpPtr   ep      式
 * @param  EnvPtr   env     環境
 * @param  FEnvPtr  fenv    環境
 * @return int              成否
 */   
int eval_exp(TC_Value *result, ExpPtr ep, EnvPtr env, FEnvPtr fenv) {
    TC_Value tv = {0}, e1 = {0}, e2 = {0};
    EnvPtr env1 = NULL, tmp = NULL;
    Func func = {0};
    ArgsPtr ap = NULL;
    ExpsPtr esp = NULL;
    int bexp_result = 0;
    
    if(NULL == ep) {
        return 0;
    }
    switch(ep->t) {
    case VAR_EXP:
        if( 0 == (find_value(result, ep->of.var, env)) ) return 0;
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
        if(0 == eval_exp(&e1, ep->of.Prim.exp1, env, fenv)) return 0;
        if(0 == eval_exp(&e2, ep->of.Prim.exp2, env, fenv)) return 0;
        switch(ep->of.Prim.op) {
        case PLUS_OP:
            result->t = INT;
            result->of.i = e1.of.i + e2.of.i;
            break;
        case FPLUS_OP:
            result->t = FLOAT;
            result->of.f = e1.of.f + e2.of.f;
            break;
        case MINUS_OP:
            result->t = INT;
            result->of.i = e1.of.i - e2.of.i;
            break;
        case FMINUS_OP:
            result->t = FLOAT;
            result->of.f = e1.of.f - e2.of.f;
            break;
        case MUL_OP:
            result->t = INT;
            result->of.i = e1.of.i * e2.of.i;
            break;
        case FMUL_OP:
            result->t = FLOAT;
            result->of.f = e1.of.f * e2.of.f;
            break;
        }
        break;
    case LET_EXP:
        /* 元の環境で let 式評価 */
        if(0 == eval_exp(&tv, ep->of.Let.exp1, env, fenv)) return 0;
        env1 = env_alloc();
        strncpy_s(env1->var, MAX_VAR_NAME_LENGTH, ep->of.Let.tvar.var, strlen(ep->of.Let.tvar.var));
        env1->tv = tv;
        env1->next = env;
        /* 新たな環境で in 式評価 */
        if(0 == eval_exp(result, ep->of.Let.exp2, env1, fenv)) return 0;
        break;
    case IF_EXP:
        if(0 == eval_bexp(&bexp_result, ep->of.If.bp, env, fenv)) return 0;
        if(1 == bexp_result) {
            if(0 == eval_exp(result, ep->of.If.exp1, env, fenv)) return 0;
        } else if(0 == bexp_result) {
            if(0 == eval_exp(result, ep->of.If.exp2, env, fenv)) return 0;
        } else return 0;
        break;
    case APP_EXP:
        if( 0 == find_func(&func, ep->of.App.var, fenv) ) return 0;
        env1 = env_alloc();
        for(tmp = env1, ap = func.args, esp = ep->of.App.exps;
            ap != NULL; ap = ap->next, tmp = tmp->next, esp = esp->next) {
            strncpy_s(tmp->var, MAX_VAR_NAME_LENGTH, ap->tvar.var, strlen(ap->tvar.var));
            if(0 == eval_exp(&(tmp->tv), esp->elm, env, fenv)) return 0;
            if(NULL != ap->next) {
                tmp->next = env_alloc();
            } else {
                break;
            }
        }
        tmp->next = env;
        if(0 == eval_exp(result, func.exp, env1, fenv)) return 0;
        break;
    }
    return 1;
}

/**
 * ブール式を評価する
 * @param int     *result 評価結果の格納先
 * @param ProgPtr pp      プログラム
 * @param EnvPtr  env     環境
 * @param FEnvPtr fenv    関数の型環境
 * @return int            成否
 */   
int eval_bexp(int *result, BexpPtr bp, EnvPtr env, FEnvPtr fenv) {
    TC_Value e1 = {0}, e2 = {0};
    if(NULL == bp) {
        return 0;
    }
    if(0 == eval_exp(&e1, bp->exp1, env, fenv)) return 0;        
    if(0 == eval_exp(&e2, bp->exp2, env, fenv)) return 0;

    switch(e1.t) {
    case INT:
        *result = (e1.of.i == e2.of.i) ? 1 : 0;
        break;
    case FLOAT:
        *result = (e1.of.f == e2.of.f) ? 1 : 0;
        break;
    }
    return 1;
}

/**
 * プログラムを評価する
 * @param TC_Value *result 評価結果の格納先
 * @param ProgPtr  pp      プログラム
 * @param FEnvPtr  fenv    関数の型環境
 * @return int             成否
 */   
int eval_prog(TC_Value *result, ProgPtr pp, FEnvPtr fenv) {
    FEnvPtr fenv1 = NULL;
    ArgsPtr ap = NULL;
    TC_Value tv = {0};
    Func func = {0};
    if(NULL == pp) {
        return 0;
    }
    switch(pp->t) {
    case EXP:
        if(0 == eval_exp(result, pp->of.exp, env_alloc(), fenv)) return 0;
        break;
    case LETREC:
        fenv1 = fenv_alloc();
        strncpy_s(fenv1->var, MAX_VAR_NAME_LENGTH, pp->of.LetRec.tvar.var, strlen(pp->of.LetRec.tvar.var));
        fenv1->f.args = pp->of.LetRec.args;
        fenv1->f.exp = pp->of.LetRec.exp;
        fenv1->next = fenv;
        
        if(0 == eval_prog(result, pp->of.LetRec.prog, fenv1)) return 0;
        break;
    }
    return 1;
}

/**
 * プログラムを評価する
 * 
 * プログラムを初期化してパースし、
 * 結果を評価関数に渡す
 * @param  TC_Value *v 結果の格納先
 * @return int         成否
 */
int eval(TC_Value *v) {
    ProgPtr pp = NULL;
    TEnvPtr tenv = NULL;
    TC_TypePtr ttp = NULL;
    char mes[MESSAGE_MAX_BUF_SIZE] = "- : ";
    char floatmes[] = "float = ";
    char intmes[] = "int = ";

    /* 構文解析を実行 */
    pp = prog_alloc();
    if(0 == parse_prog(pp)) return 0;
    tenv = tenv_alloc();
    if( NULL != (ttp = typing_prog(tenv, pp)) ) {
        if( FLOAT_TYPE == norm(ttp)->t ) {
            strncat_s(mes, MESSAGE_MAX_BUF_SIZE, floatmes, strlen(floatmes));
        } else {
            strncat_s(mes, MESSAGE_MAX_BUF_SIZE, intmes, strlen(intmes));
        }
    } else {
        fprintf(stderr, "typing error.\n");
        return 0;
    }
    if( 0 == eval_prog(v, pp, fenv_alloc()) ) {
        fprintf(stderr, "eval error\n");
        return 0;
    }

    switch(v->t) {
    case INT:
        printf("%s%d\n", mes, v->of.i);
        break;
    case FLOAT:
        printf("%s%f\n", mes, v->of.f);
        break;
    }

    return 1;
}
