/* -*- coding: utf-8-dos; mode: C -*- */
#ifndef TINYCAML_H_INCLUDED
#define TINYCAML_H_INCLUDED

#ifdef _DEBUG
#define PAUSE system("PAUSE")
#else
#define PAUSE {}
#endif

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
    
int eval(TC_Value *);

#endif
