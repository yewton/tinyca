/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinycaml.h"
#include "abssyn.h"

/* 型変数の ID */
static int current_id = 0;

/**
 * 型変数を正規化する
 * @param TC_TypePtr ttp 型構造へのポインタ
 * @return TC_TypePtr 型構造へのポインタ
 */
TC_TypePtr norm(const TC_TypePtr ttp) {
    TC_TypePtr result;
    switch(ttp->t) {
    case TYVAR_TYPE:
        if(UNKNOWN_TYPE == ttp->varty.ty->t) {
            result = ttp;
        } else {
            result = norm(ttp->varty.ty);
        }
        break;
    default:
        result = ttp;
    }
    return result;
}

/**
 * 単純型の領域を確保
 * @return TC_TypePtr 確保した領域へのポインタ
 */
TC_TypePtr sty_alloc(void) {
    TC_TypePtr ttp = NULL;
    ttp = (TC_TypePtr)malloc(sizeof(TC_Type));
    memset(ttp, 0xc, sizeof(TC_Type));

    return ttp;
}

/**
 * フレッシュな型変数を生成し、その領域を確保
 * @return TC_TypePtr 確保した領域へのポインタ
 */
TC_TypePtr tyvar_alloc(void) {
    int i = 0;
    TC_TypePtr tvp;
    
    i = current_id;
    current_id = ++i;

    tvp = sty_alloc();
    tvp->t = TYVAR_TYPE;
    tvp->varty.id = i;
    tvp->varty.ty = sty_alloc();
    tvp->varty.ty->t = UNKNOWN_TYPE;

    return tvp;
}

/**
 * 単一化を実行する
 * @param TC_TypePtr ttp1
 * @param TC_TypePtr ttp2
 * @return int 成否
 */
int unify(const TC_TypePtr ttp1, const TC_TypePtr ttp2) {
    TC_TypePtr tt1, tt2;
    tt1 = norm(ttp1);
    tt2 = norm(ttp2);

    if(TYVAR_TYPE == tt1->t) {
        if(UNKNOWN_TYPE == tt1->varty.ty->t) {
            if(TYVAR_TYPE == tt2->t) {
                if(UNKNOWN_TYPE == tt2->varty.ty->t) {
                    if(tt1->varty.id != tt2->varty.id) {
                        tt1->varty.ty = ttp2;
                        return 1;
                    }
                }
            }
            tt1->varty.ty = ttp2;
            return 1;
        }
    }
    if(TYVAR_TYPE == tt2->t) {
        if(UNKNOWN_TYPE == tt2->varty.ty->t) {
            tt2->varty.ty = ttp1;
            return 1;
        }
    }
    if( (INT_TYPE == tt1->t) && (INT_TYPE == tt2->t) ) return 1;
    if( (FLOAT_TYPE == tt1->t) && (FLOAT_TYPE == tt2->t) ) return 1;
    return 0;
}

/**
 * 型リストの領域を確保して初期化する
 * @return TypesPTr 確保した領域へのポインタ
 */  
TypesPtr types_alloc(void) {
    TypesPtr tp = NULL;
    tp = (TypesPtr)malloc(sizeof(Types));
    memset(tp, 0xc, sizeof(Types));
    tp->next = NULL;
    return tp;
}

/**
 * 拡張型の領域を確保して初期化する
 * @return ExTypePTr 確保した領域へのポインタ
 */  
ExTypePtr ex_type_alloc(void) {
    ExTypePtr etp = NULL;
    etp = (ExTypePtr)malloc(sizeof(ExType));
    memset(etp, 0xc, sizeof(ExType));
    return etp;
}

/**
 * 型環境の領域を確保して初期化する
 * @return EnvPTr 確保した領域へのポインタ
 */  
EnvPtr env_alloc(void) {
    EnvPtr ep = NULL;
    ep = (EnvPtr)malloc(sizeof(Env));
    memset(ep, 0xc, sizeof(Env));
    ep->next = NULL;
    return ep;
}

TC_TypePtr typing(EnvPtr env, ExpPtr ep) {
    ExTypePtr etp = NULL;
    EnvPtr env1 = NULL;
    TC_TypePtr ttp = NULL;
    TC_TypePtr ttp1 = NULL, ttp2 = NULL;
    TC_TypePtr result = NULL;

    etp = ex_type_alloc();
    switch(ep->t) {
    case VAR_EXP:
        if(1 == find_var(env, ep->of.var, etp)) {
            if(SIMPLE_TYPE == etp->t) {
                return etp->of.SimpleTy.ttp;
            } else {
                printf("typing error\n");
                return NULL;
            }
        } else {
            printf("Undefined variable '%s'.\n", ep->of.var);
            return NULL;
        }
        break;
    case INT_EXP:
        result = sty_alloc();
        result->t = INT_TYPE;
        return result;
        break;
    case FLOAT_EXP:
        result = sty_alloc();
        result->t = FLOAT_TYPE;
        return result;
        break;
    case PRIM_EXP:
        switch(ep->of.Prim.op) {
        case PLUS_OP:
        case MINUS_OP:
        case MUL_OP:
            /* 演算子の型 */
            ttp = sty_alloc();
            ttp->t = INT_TYPE;
            break;
        case FPLUS_OP:
        case FMINUS_OP:
        case FMUL_OP:
            /* 演算子の型 */
            ttp = sty_alloc();
            ttp->t = FLOAT_TYPE;
            break;
        }
        ttp1 = typing(env, ep->of.Prim.exp1);
        ttp2 = typing(env, ep->of.Prim.exp2);
        if(0 == unify(ttp, ttp1)) return NULL;
        if(0 == unify(ttp1, ttp2)) return NULL;
        result = ttp;
        break;
    case LET_EXP:
        /* 現在の環境のもとで let 式を評価 */
        if(NULL == (ttp = typing(env, ep->of.Let.exp1))) return NULL;
        if(0 == unify(ep->of.Let.tvar.ttp, ttp)) return NULL;
        /* 結果を元に、新たな環境要素を追加 */
        env1 = env_alloc();
        strncpy_s(env1->var, MAX_VAR_NAME_LENGTH, ep->of.Let.tvar.var, strlen(ep->of.Let.tvar.var));
        env1->et.t = SIMPLE_TYPE;
        env1->et.of.SimpleTy.ttp = ttp;
        env1->next = env;
        /* 新たな環境のもとで in 式を評価 */
        if(NULL == (result = typing(env1, ep->of.Let.exp2))) return NULL;
        break;
    default:
        return NULL;
        break;
    }
    return NULL;
}
        
TC_TypePtr typing_prog(EnvPtr env, ProgPtr pp) {
    EnvPtr env1 = NULL, tmp = NULL;
    TypesPtr tp = NULL;
    ArgsPtr ap = NULL;
    TC_TypePtr ttp = NULL, result = NULL;

    switch(pp->t) {
    case EXP:
        result = typing(env, pp->of.exp);
        break;
    case LETREC:
        /* 元の環境の先頭要素を収める領域を確保 */
        tmp = env_alloc();
        *tmp = *env;
        /* 環境の先頭に関数型環境要素を格納 */
        strncpy_s(env->var, MAX_VAR_NAME_LENGTH, pp->of.LetRec.tvar.var, strlen(pp->of.LetRec.tvar.var));
        env->et.t = FUNCTION_TYPE;
        env->et.of.FunTy.ttp = pp->of.LetRec.tvar.ttp;
        env->et.of.FunTy.tp = types_alloc();
        for(tp = env->et.of.FunTy.tp, ap = pp->of.LetRec.args; ap != NULL; ap = ap->next, tp = tp->next) {
            tp->ttp = ap->tvar.ttp;
            tp->next = types_alloc();
        }
        free(tp->next);
        tp->next = NULL;
        /* 元の先頭要素を追加する要素の次につなぎ直す */
        env->next = tmp;
        /* let rec 式内のローカル環境を定義 */
        env1 = env_alloc();
        for(tmp = env1, ap = pp->of.LetRec.args; ap != NULL; ap = ap->next, tmp = tmp->next) {
            strncpy_s(tmp->var, MAX_VAR_NAME_LENGTH, ap->tvar.var, strlen(ap->tvar.var));
            tmp->et.t = SIMPLE_TYPE;
            tmp->et.of.SimpleTy.ttp = ap->tvar.ttp;
            tmp->next = env_alloc();
        }
        /* 再帰関数定義のために、先程確保した型環境(+元の環境)をローカル環境の末尾につなげる */
        tmp = env;
        if(NULL == (ttp = typing(env1, pp->of.LetRec.exp))) {
            result = NULL;
            break;
        }
        if(0 == unify(pp->of.LetRec.tvar.ttp, ttp)) {
            result = NULL;
        }
        result = typing_prog(env, pp->of.LetRec.prog);
        break;
    }
    return result;
}

