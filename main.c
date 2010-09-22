/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include "tinycaml.h"
#include "dump.h"

int main(void) {
    TC_Value v = {0};
    printf("\tTiny Small Minimum Caml version 0.0.0\n\n");
    for(;;) {
        printf("# ");
        dump();
        printf("\n");
        /* switch(v.t) { */
        /* case INT: */
        /*     printf("- : int = %d\n", v.of.i); */
        /*     break; */
        /* case FLOAT: */
        /*     printf("- : float = %f\n", v.of.f); */
        /*     break; */
        /* } */
    }
    return 0;
}
