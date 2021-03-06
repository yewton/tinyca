/* -*- coding: utf-8-dos; mode: C -*- */
#ifndef ABSSYN_H_INCLUDED
#define ABSSYN_H_INCLUDED

#define MAX_VAR_NAME_LENGTH (100)

/* 「型」型 */
typedef enum {
    INT_TYPE, /* 整数型 */
    FLOAT_TYPE, /* 浮動小数点数型 */
    TYVAR_TYPE, /* 型変数型 */
    UNKNOWN_TYPE /* まだ未定義の型変数を表すためのダミーの値 */
} TyType;

/* 型変数の構造定義 */
typedef struct _TC_Type {
    TyType t;
    struct {
        int id;
        struct _TC_Type *ty;
    } varty;
} TC_Type, *TC_TypePtr;

/* 型付きの変数 */
typedef struct _TypedVar {
    char var[MAX_VAR_NAME_LENGTH];
    TC_TypePtr ttp;
} TypedVar;

/* 型ポインタリスト */
typedef struct _Types {
    TC_TypePtr ttp;
    struct _Types *next;
} Types, *TypesPtr;

/* 拡張型 */
typedef enum {
    FUNCTION_TYPE, /* 関数型 */
    SIMPLE_TYPE /* 単純型 */
} ExTyType;

/* 拡張型の構造定義 */
typedef struct _ExType {
    ExTyType t;
    union {
        struct {
            TC_TypePtr ttp;
            TypesPtr tp;
        } FunTy;
        struct {
            TC_TypePtr ttp;
        } SimpleTy;
    } of;
} ExType, *ExTypePtr;

/* 型環境 */
typedef struct _TEnv {
    char var[MAX_VAR_NAME_LENGTH];
    ExType et;
    struct _TEnv *next;
} TEnv, *TEnvPtr;

/* 演算子型 */
typedef enum {
    PLUS_OP,
    FPLUS_OP,
    MINUS_OP,
    FMINUS_OP,
    MUL_OP,
    FMUL_OP
} OpType;

/* プログラム型 */
typedef enum {
    EXP,
    LETREC
} ProgType;

/* 式型 */
typedef enum {
    VAR_EXP,
    INT_EXP,
    FLOAT_EXP, 
    PRIM_EXP, /* 足し算、引き算、掛け算 の式 */
    LET_EXP,
    IF_EXP,
    APP_EXP /* 関数適用 */
} ExpType;

/* 式リスト */
typedef struct _Exps {
    struct _Exp *elm;
    struct _Exps *next;
} Exps, *ExpsPtr;

/* 式の構造定義 */
typedef struct _Exp {
    ExpType t;
    union {
        char var[MAX_VAR_NAME_LENGTH];
        int intval;
        float floatval;
        struct {
            OpType op;
            struct _Exp *exp1, *exp2;
        } Prim;
        struct {
            TypedVar tvar;
            struct _Exp *exp1, *exp2;
        } Let;
        struct {
            struct _Bexp *bp;
            struct _Exp *exp1, *exp2;
        } If;
        struct {
            char var[MAX_VAR_NAME_LENGTH];
            struct _Exps *exps;
        } App;
    } of;
    struct _Exp *next;
} Exp, *ExpPtr;

/* ブール式型 */
typedef enum {
    EQ_BEXP
} BexpType;

/* ブール式の構造定義 */
typedef struct _Bexp {
    BexpType t;
    TC_TypePtr ttp;
    ExpPtr exp1, exp2;
} Bexp, *BexpPtr;

/* 変数リスト */
typedef struct _Args {
    TypedVar tvar;
    struct _Args *next;
} Args, *ArgsPtr;

/* プログラムの構造定義 */
typedef struct _Prog {
    ProgType t;
    union {
        ExpPtr exp;
        struct {
            TypedVar tvar;
            ArgsPtr args;
            ExpPtr exp;
            struct _Prog *prog;
        } LetRec;
    } of;
} Prog, *ProgPtr;

/* tinycaml における型 */
typedef enum {
    INT,
    FLOAT
} PrimType;

/* tinycaml における値 */
typedef struct _TC_Value {
    PrimType t;
    union {
        int i;
        float f;
    } of;
} TC_Value;

/* 変数の環境 */
typedef struct _Env {
    char var[MAX_VAR_NAME_LENGTH];
    TC_Value tv;
    struct _Env *next;
} Env, *EnvPtr;

/* 関数の構造 */
typedef struct _Func {
    ArgsPtr args;
    ExpPtr exp;
} Func;    

/* 関数の環境 */
typedef struct _FEnv {
    char var[MAX_VAR_NAME_LENGTH];
    Func f;
    struct _FEnv *next;
} FEnv, *FEnvPtr;

/* コンパイラ用変数の型環境 */
typedef struct _EnvC {
    char var[MAX_VAR_NAME_LENGTH];
    TyType t;
    struct _EnvC *next;
} EnvC, *EnvCPtr;

/* 型リスト */
typedef struct _TyTypes {
    TyType t;
    struct _TyTypes *next;
} TyTypes, *TyTypesPtr;

/* 関数の型構造 */
typedef struct _FuncT {
    TyType fty;
    TyTypesPtr tys;
} FuncT;

/* コンパイラ用関数の型環境 */
typedef struct _FEnvC {
    char var[MAX_VAR_NAME_LENGTH];
    FuncT t;
    struct _FEnvC *next;
} FEnvC, *FEnvCPtr;

/* 式 */
ExpPtr exp_alloc(void);
void clean_exp(ExpPtr);
/* 式リスト */
ExpsPtr exps_alloc(void);
/* 変数リスト */
ArgsPtr args_alloc(void);
/* 型リスト */
TypesPtr types_alloc(void);
/* 変数の環境 */
EnvPtr env_alloc(void);
int find_var(EnvPtr, const char*);
/* 関数の環境 */
FEnvPtr fenv_alloc(void);
int find_var_f(FEnvPtr, const char*);
/* 型環境 */
TEnvPtr tenv_alloc(void);
int find_var_t(TEnvPtr, const char *, ExTypePtr);

/* 変数の型環境 */
EnvCPtr envc_alloc(void);
/* 変数の型環境 */
TyTypesPtr tytypes_alloc(void);
/* 関数の環境 */
FEnvCPtr fenvc_alloc(void);

/* ブール式 */
BexpPtr bexp_alloc(void);
/* プログラム */
ProgPtr prog_alloc(void);

int parse_prog(ProgPtr);
int eval(TC_Value *);
int compile(char *);

#endif
