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
    ap->next = NULL;
    return ap;
}

/**
 * 変数の型環境の領域を確保して初期化する
 * @return EnvPtr 確保した領域へのポインタ
 */  
EnvPtr env_alloc(void) {
    EnvPtr ep = NULL;
    ep = (EnvPtr)malloc(sizeof(Env));
    memset(ep, 0xc, sizeof(Env));
    ep->next = NULL;
    return ep;
}

/**
 * 型リストの領域を確保して初期化する
 * @return TypeListPtr 確保した領域へのポインタ
 */  
TypeListPtr tl_alloc(void) {
    TypeListPtr tlp = NULL;
    tlp = (TypeListPtr)malloc(sizeof(TypeList));
    memset(tlp, 0xc, sizeof(TypeList));
    tlp->next = NULL;
    return tlp;
}

/**
 * 関数の型環境の領域を確保して初期化する
 * @return FEnvPtr 確保した領域へのポインタ
 */  
FEnvPtr fenv_alloc(void) {
    FEnvPtr fep = NULL;
    fep = (FEnvPtr)malloc(sizeof(FEnv));
    memset(fep, 0xc, sizeof(TypeList));
    fep->t.aty = tl_alloc();
    fep->next = NULL;
    return fep;
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
