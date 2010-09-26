/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "abssyn.h"
#include "parser.h"
#include "type_inference.h"

#define MESSAGE_MAX_BUF_SIZE (1024)
#define FILE_NAME_MAX_SIZE (1024)
#define LABEL_NAME_MAX_SIZE (256)

static int current_labelnum = 0;
static int current_varnum = 0;

void newlabel(char *dest, const char *label) {
    int i = current_labelnum;
    char labelnum[LABEL_NAME_MAX_SIZE] = "";
    current_labelnum = i + 1;
    sprintf_s(labelnum, LABEL_NAME_MAX_SIZE, "%d", i);

    strncpy_s(dest, LABEL_NAME_MAX_SIZE, label, strlen(label));
    strncat_s(dest, LABEL_NAME_MAX_SIZE, labelnum, strlen(labelnum));
    return;
}

int newvar(void) {
    int i = current_varnum;
    current_varnum = i + 1;
    return i;
}

int varnum(const char *var, EnvCPtr env) {
    EnvCPtr tmp = env;
    int i = 0, j = -1;
    int result = 0;
    for(; tmp != NULL; tmp = tmp->next, ++i) {
        if( (j < 0) && (0 == strcmp(tmp->var, var)) ) {
            /* 初発見時の順番を保持 */
            j = i;
        }
    }
    result = (i - 2) - j;
    return result;
}

TyType find_var_c(const char *var, EnvCPtr env) {
    EnvCPtr tmp = env;
    for(; tmp != NULL; tmp = tmp->next) {
        if( 0 == strcmp(tmp->var, var) ) {
            return tmp->t;
        }
    }
    return UNKNOWN_TYPE;
}

FuncT find_func_c(const char *var, FEnvCPtr fenv) {
    FEnvCPtr tmp = fenv;
    FuncT t = {0};
    for(; tmp != NULL; tmp = tmp->next) {
        if( 0 == strcmp(tmp->var, var) ) {
            t = tmp->t;
        }
    }
    return t;
}

int desc_of_ty(char *dest, TyType t) {
    int result = 0;
    switch(t) {
    case INT_TYPE:
        strncpy_s(dest, 2, "I", 2);
        result = 1;
        break;
    case FLOAT_TYPE:
        strncpy_s(dest, 2, "F", 2);
        result = 1;
        break;
    default:
        break;
    }
    return result;
}

void pr_instr(FILE *fp, const char *format, ...) {
    va_list list;
    const char *p;

    fprintf_s(fp, "\t");
    va_start(list, format);
    for(p = format; *p; ++p) {
        if( *p != '%' ) {
			fprintf(fp, "%c", *p);
			continue;
		}
		++p;
		switch( *p )
		{
		case 'd':
			fprintf(fp, "%d", va_arg(list, int));
			break;
		case 'c':
			fprintf(fp, "%c", va_arg(list, char));
			break;
		case 's':
			fprintf(fp, "%s", va_arg(list, char*));
			break;
		default:
			fprintf(fp, "%c", *p);
			break;
		}
    }
    va_end(list);
    fprintf_s(fp, "\n");
    return;
}

/**
 * コードを生成する
 * @param FILE     *ofp  出力先
 * @param char     *file ファイル名
 * @param FEnvCPtr fenvc 関数の型環境
 * @param EnvCPtr  envc  変数の型環境
 * @param ExpPtr   ep    式
 * @return int           成否
 */   
int codegen(FILE *ofp, const char *file, FEnvCPtr fenv, EnvCPtr env, ExpPtr ep) {
    TyType t;
    TyTypesPtr ts;
    EnvCPtr env1 = NULL;
    ExpsPtr es = NULL;
    FuncT ft = {0};
    char l[LABEL_NAME_MAX_SIZE] = "";
    char m[LABEL_NAME_MAX_SIZE] = "";
    char dty[2] = "";
        
    if(NULL == ep) {
        return 0;
    }
    switch(ep->t) {
    case VAR_EXP:
        t = find_var_c(ep->of.var, env);
        switch(t) {
        case FLOAT_TYPE:
            pr_instr(ofp, "fload %d", varnum(ep->of.var, env));
            break;
        default:
            pr_instr(ofp, "iload %d", varnum(ep->of.var, env));
            break;
        }
        break;
    case INT_EXP:
        pr_instr(ofp, "ldc %d", ep->of.intval);
        break;
    case FLOAT_EXP:
        pr_instr(ofp, "ldc %f", ep->of.floatval);
        break;
    case PRIM_EXP:
        codegen(ofp, file, fenv, env, ep->of.Prim.exp1);
        codegen(ofp, file, fenv, env, ep->of.Prim.exp2);
        switch(ep->of.Prim.op) {
        case PLUS_OP:
            pr_instr(ofp, "iadd");
            break;
        case FPLUS_OP:
            pr_instr(ofp, "fadd");
            break;
        case MINUS_OP:
            pr_instr(ofp, "isub");
            break;
        case FMINUS_OP:
            pr_instr(ofp, "fsub");
            break;
        case MUL_OP:
            pr_instr(ofp, "imul");
            break;
        case FMUL_OP:
            pr_instr(ofp, "fmul");
            break;
        }
        break;
    case LET_EXP:
        codegen(ofp, file, fenv, env, ep->of.Let.exp1);
        switch((norm(ep->of.Let.tvar.ttp))->t) {
        case FLOAT_TYPE:
            pr_instr(ofp, "fstore %d", newvar());
            break;
        default:
            pr_instr(ofp, "istore %d", newvar());
            break;
        }
        env1 = envc_alloc();
        strncpy_s(env1->var, MAX_VAR_NAME_LENGTH, ep->of.Let.tvar.var, strlen(ep->of.Let.tvar.var));
        env1->t = (norm(ep->of.Let.tvar.ttp))->t;
        env1->next = env;
        codegen(ofp, file, fenv, env1, ep->of.Let.exp2);
        break;
    case IF_EXP:
        newlabel(l, "else");
        newlabel(m, "endif");
        codegen(ofp, file, fenv, env, ep->of.If.bp->exp1);
        codegen(ofp, file, fenv, env, ep->of.If.bp->exp2);
        switch((norm(ep->of.If.bp->ttp))->t) {
        case FLOAT_TYPE:
            pr_instr(ofp, "fcmpl");
            pr_instr(ofp, "ifne %s", l);
            break;
        default:
            pr_instr(ofp, "if_icmpne %s", l);
            break;
        }
        codegen(ofp, file, fenv, env, ep->of.If.exp1);
        pr_instr(ofp, "goto %s", m);
        fprintf_s(ofp, "%s:\n", l);
        codegen(ofp, file, fenv, env, ep->of.If.exp2);
        fprintf_s(ofp, "%s:\n", m);
        break;
    case APP_EXP:
        for(es = ep->of.App.exps; NULL != es; es = es->next) {
            codegen(ofp, file, fenv, env, es->elm);
        }
        ft = find_func_c(ep->of.App.var, fenv);
        fprintf_s(ofp, "\tinvokestatic %s/%s(", file, ep->of.App.var);
        for(ts = ft.tys; NULL != ts; ts = ts->next) {
            desc_of_ty(dty, ts->t);
            fprintf_s(ofp, "%s", dty);
            printf("dty -> %s\n", dty);
        }
        desc_of_ty(dty, ft.fty);
        fprintf_s(ofp, ")%s\n", dty);
        break;
    }
    return 1;
}

/**
 * プログラムのコードを生成する
 * @param FILE     *ofp  出力先
 * @param char     *file ファイル名
 * @param TyType   t     プログラムの型
 * @param ProgPtr  pp    プログラム
 * @param FEnvCPtr fenvc 関数の型環境
 * @return int           成否
 */   
int codegen_prog(FILE *ofp, const char *file, TyType t, ProgPtr pp, FEnvCPtr fenvc) {
    FEnvCPtr fenvc1 = NULL;
    EnvCPtr env = NULL;
    ArgsPtr ap = NULL;
    TyTypesPtr ttsp = NULL;
    char dty[2] = "";
    
    if(NULL == pp) {
        return 0;
    }
    switch(pp->t) {
    case EXP:
        fprintf_s(ofp, ".method public static main([Ljava/lang/String;)V\n");
        pr_instr(ofp, ".limit stack 100");
        pr_instr(ofp, ".limit locals 100");
        pr_instr(ofp, "getstatic java/lang/System/out Ljava/io/PrintStream;");
        codegen(ofp, file, fenvc, envc_alloc(), pp->of.exp);
        desc_of_ty(dty, t);
        pr_instr(ofp, "invokevirtual java/io/PrintStream/println(%s)V", dty);
        pr_instr(ofp, "return");
        fprintf_s(ofp, ".end method\n\n");
        break;
    case LETREC:
        fenvc1 = fenvc_alloc();
        env = envc_alloc();
        strncpy_s(fenvc1->var, MAX_VAR_NAME_LENGTH, pp->of.LetRec.tvar.var, strlen(pp->of.LetRec.tvar.var));
        fenvc1->t.fty = (norm(pp->of.LetRec.tvar.ttp))->t;
        for(ap = pp->of.LetRec.args, ttsp = fenvc1->t.tys;
            ap != NULL; ap = ap->next, ttsp = ttsp->next) {
            ttsp->t = (norm(ap->tvar.ttp))->t;
            ttsp->next = (NULL != ap->next) ? tytypes_alloc() : NULL;
        }
        fenvc1->next = fenvc;
        fprintf_s(ofp, ".method public static %s(", pp->of.LetRec.tvar.var);
        for(ap = pp->of.LetRec.args; ap != NULL; ap = ap->next) {
            if( 0 == desc_of_ty(dty, (norm(ap->tvar.ttp))->t) ) {
                return 0;
            }
            fprintf_s(ofp, "%s", dty);
            strncpy_s(env->var, MAX_VAR_NAME_LENGTH, ap->tvar.var, strlen(ap->tvar.var));
            env->t = (norm(ap->tvar.ttp))->t;
            env->next = envc_alloc();
        }
        if( 0 == desc_of_ty(dty, (norm(pp->of.LetRec.tvar.ttp))->t) ) return 0;
        fprintf_s(ofp, ")%s\n", dty);
        pr_instr(ofp, ".limit stack 100");
        pr_instr(ofp, ".limit locals 100");
        codegen(ofp, file, fenvc1, env, pp->of.LetRec.exp);
        switch((norm(pp->of.LetRec.tvar.ttp))->t) {
        case FLOAT_TYPE:
            pr_instr(ofp, "freturn");
            break;
        default:
            pr_instr(ofp, "ireturn");
            break;
        }
        fprintf_s(ofp, ".end method\n", dty);
        codegen_prog(ofp, file, (norm(pp->of.LetRec.tvar.ttp))->t, pp->of.LetRec.prog, fenvc1);
    }
    return 1;
}

/**
 * コンパイルを行う
 * @param  char *file 書き出すファイル名
 * @return int        成否
 */
int compile(char *file) {
    ProgPtr pp = NULL;
    TEnvPtr tenv = NULL;
    TC_TypePtr ttp = NULL;
    TyType t = UNKNOWN_TYPE;
    FILE *ofp = NULL;
    char mes[MESSAGE_MAX_BUF_SIZE] = "";
    char ofile[FILE_NAME_MAX_SIZE] = "";
    char ext[] = ".j";

    /* 構文解析を実行 */
    pp = prog_alloc();
    if(0 == parse_prog(pp)) return 0;
    tenv = tenv_alloc();
    if( NULL != (ttp = typing_prog(tenv, pp)) ) {
        t = norm(ttp)->t;
    } else {
        printf("typing error.\n");
        return 0;
    }
    strncpy_s(ofile, FILE_NAME_MAX_SIZE, file, strlen(file));
    strncat_s(ofile, FILE_NAME_MAX_SIZE, ext, strlen(ext));
    if( 0 != fopen_s(&ofp, ofile, "w") ) {
        perror("fopen_s");
        return 0;
    }
    fprintf_s(ofp, ".class public %s\n", file);
    fprintf_s(ofp, ".super java/lang/Object\n", file);
    
    codegen_prog(ofp, file, t, pp, fenvc_alloc());
    fclose(ofp);
    return 1;
}
