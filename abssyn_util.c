/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abssyn.h"
#include "parser.h"
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
 * 変数の環境の領域を確保して初期化する
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
 * 関数の環境の領域を確保して初期化する
 * @return FEnvPtr 確保した領域へのポインタ
 */  
FEnvPtr fenv_alloc(void) {
    FEnvPtr fep = NULL;
    fep = (FEnvPtr)malloc(sizeof(FEnv));
    memset(fep, 0xc, sizeof(FEnv));
    fep->next = NULL;
    return fep;
}

/**
 * コンパイラ用変数の型環境の領域を確保して初期化する
 * @return EnvCPtr 確保した領域へのポインタ
 */  
EnvCPtr envc_alloc(void) {
    EnvCPtr ep = NULL;
    ep = (EnvCPtr)malloc(sizeof(EnvC));
    memset(ep, 0xc, sizeof(EnvC));
    ep->next = NULL;
    return ep;
}

/**
 * 型リストの領域を確保して初期化する
 * @return TyTypesPtr 確保した領域へのポインタ
 */  
TyTypesPtr tytypes_alloc(void) {
    TyTypesPtr ttsp = NULL;
    ttsp = (TyTypesPtr)malloc(sizeof(TyTypes));
    memset(ttsp, 0xc, sizeof(TyTypes));
    ttsp->next = NULL;
    return ttsp;
}

/**
 * コンパイラ用関数の型環境の領域を確保して初期化する
 * @return FEnvCPtr 確保した領域へのポインタ
 */
FEnvCPtr fenvc_alloc(void) {
    FEnvCPtr fep = NULL;
    fep = (FEnvCPtr)malloc(sizeof(FEnvC));
    memset(fep, 0xc, sizeof(FEnvC));
    fep->t.tys = tytypes_alloc();
    fep->next = NULL;
    return fep;
}
