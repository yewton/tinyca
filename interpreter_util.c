/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abssyn.h"
#include "parser.h"
#include "tinycaml.h"
#include "type_inference.h"

/**
 * 式リストの領域を確保して初期化する
 * @return ExpsPTr 確保した領域へのポインタ
 */  
ExpsPtr exps_alloc(void) {
    ExpsPtr exps = NULL;
    exps = (ExpsPtr)malloc(sizeof(Exps));
    memset(exps, 0xc, sizeof(Exps));
    return exps;
}

/**
 * 式の領域を確保して初期化する
 * @return ExpPtr 確保した領域へのポインタ
 */  
ExpPtr exp_alloc(void) {
    ExpPtr ep = NULL;
    ep = (ExpPtr)malloc(sizeof(Exp));
    memset(ep, 0xc, sizeof(Exp));
    ep->next = NULL;

    return ep;
}

/**
 * 式のメモリ開放
 * @param ExpPtr ep 式の先頭へのポインタ
 */
void clean_exp(ExpPtr ep) {
    ExpPtr tempi = NULL, tempj = NULL;

    for(tempi = ep; tempi != NULL; tempi = tempj) {
        tempj = tempi->next;
        free(tempi);
#ifdef _DEBUG
        printf(".");
#endif
    }
    return;
}

/**
 * ブール式の領域を確保して初期化する
 * @return BexpPtr 確保した領域へのポインタ
 */  
BexpPtr bexp_alloc(void) {
    BexpPtr bp = NULL;
    bp = (BexpPtr)malloc(sizeof(Bexp));
    memset(bp, 0xc, sizeof(Bexp));
    bp->ttp = tyvar_alloc();
    return bp;
}

/**
 * ブール式のメモリ開放
 * @param BexpPtr bp ブール式の先頭へのポインタ
 */
void clean_bexp(BexpPtr bp) {
    clean_exp(bp->exp1);
    clean_exp(bp->exp2);
    free(bp);
#ifdef _DEBUG
        printf(".");
#endif
    return;
}

/**
 * プログラムの領域を確保して初期化する
 * @return ProgPtr 確保した領域へのポインタ
 */  
ProgPtr prog_alloc(void) {
    ProgPtr pp = NULL;
    pp = (ProgPtr)malloc(sizeof(Prog));
    memset(pp, 0xc, sizeof(Prog));
    return pp;
}

/**
 * 変数リストの領域を確保して初期化する
 * @return ArgsPTr 確保した領域へのポインタ
 */  
ArgsPtr args_alloc(void) {
    ArgsPtr ap = NULL;
    ap = (ArgsPtr)malloc(sizeof(Args));
    memset(ap, 0xc, sizeof(Args));
    return ap;
}

/**
 * 型環境から変数を探す
 * @param EnvPtr env
 * @param char *var
 * @param ExTypePtr etp
 * @return int 成否
 */
int find_var(EnvPtr env, const char *var, ExTypePtr etp) {
    EnvPtr tmp = NULL;
    int result = 0;
    for(tmp = env; tmp != NULL; tmp = tmp->next) {
        if( strncmp(tmp->var, var, strlen(var)) == 0) {
            switch(tmp->et.t) {
            case FUNCTION_TYPE:
                etp->t = FUNCTION_TYPE;
                etp->of.FunTy.ttp = tmp->et.of.FunTy.ttp;
                etp->of.FunTy.tp = tmp->et.of.FunTy.tp;
                break;
            case SIMPLE_TYPE:
                etp->t = SIMPLE_TYPE;
                etp->of.SimpleTy.ttp = tmp->et.of.SimpleTy.ttp;
                break;
            }
            result = 1;
            break;
        }
    }
    return result;
}

/**
 * プログラムのメモリ開放
 * @param ProgPtr pp プログラムの先頭へのポインタ
 */
void clean_prog(ProgPtr pp) {
    switch(pp->t) {
    case EXP:
        clean_exp(pp->of.exp);
        break;
    case LETREC:
        clean_exp(pp->of.LetRec.exp);
        clean_prog(pp->of.LetRec.prog);
        break;
    }
    free(pp);
#ifdef _DEBUG
        printf(".");
#endif
    return;
}
