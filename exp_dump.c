/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abssyn.h"
#include "tinycaml.h"
#include "type_inference.h"

void dump_e(ExpPtr ep);

void dump_b(BexpPtr bp) {
    printf("(Bexp of (=) * ");
    dump_e(bp->exp1);
    printf(" * ");
    dump_e(bp->exp2);
    printf(")");
}

void dump_e(ExpPtr ep) {
    ExpsPtr esp;
    switch(ep->t) {
    case VAR_EXP:
        printf("(Exp of %s)", ep->of.var);
        break;
    case INT_EXP:
        printf("(Exp of %d)", ep->of.intval);
        break;
    case FLOAT_EXP:
        printf("(Exp of %f)", ep->of.floatval);
        break;
    case PRIM_EXP:
        printf("(Exp of ");
        switch(ep->of.Prim.op) {
        case PLUS_OP:
            printf("(+) ");
            break;
        case FPLUS_OP:
            printf("(+.) ");
            break;
        case MINUS_OP:
            printf("(-) ");
            break;
        case FMINUS_OP:
            printf("(-.) ");
            break;
        case MUL_OP:
            printf("( * ) ");
            break;
        case FMUL_OP:
            printf("( *. ) ");
            break;
        }
        dump_e(ep->of.Prim.exp1);
        printf(" * ");
        dump_e(ep->of.Prim.exp2);
        printf(")");
        break;
    case LET_EXP:
        printf("(Exp of Let %s", ep->of.Let.tvar.var);
        printf(" * ");
        dump_e(ep->of.Let.exp1);
        printf(" * ");
        dump_e(ep->of.Let.exp2);
        printf(")");
        break;
    case IF_EXP:
        printf("(Exp of If ");
        dump_b(ep->of.If.bp);
        printf(" * ");
        dump_e(ep->of.If.exp1);
        printf(" * ");
        dump_e(ep->of.If.exp2);
        printf(")");
        break;
    case APP_EXP:
        printf("(Exp of App %s", ep->of.App.var);
        for(esp = ep->of.App.exps; NULL != esp; esp = esp->next) {
            dump_e(esp->elm);
            if(NULL != esp->next) printf(", ");
        }
        break;
    }
}

void dump_p(ProgPtr pp) {
    ArgsPtr ap;
    switch(pp->t) {
    case EXP:
        printf("(Prog of Exp ");
        dump_e(pp->of.exp);
        printf(")");
        break;
    case LETREC:
        printf("(Prog of LetRec ");
        printf("%s(", pp->of.LetRec.tvar.var);
        for(ap = pp->of.LetRec.args; NULL != ap; ap = ap->next) {
            printf("%s", ap->tvar.var);
            if(NULL != ap->next) printf(", ");
        }
        printf(") * ");
        dump_e(pp->of.LetRec.exp);
        printf(" * ");
        dump_p(pp->of.LetRec.prog);
        break;
    }
}

/**
 * 抽象構文構造を表示する
 * @return int 成否
 */
void dump(void) {
    ProgPtr pp = NULL;
    EnvPtr env = NULL;
    pp = prog_alloc();
    if(0 == parse_prog(pp)) return;
    env = env_alloc();
    if(NULL == typing_prog(env, pp)) {
    }
    dump_p(pp);
    printf("\n");
    return;
}
