/* -*- coding: utf-8-dos; mode: C -*- */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "abssyn.h"

#define FILE_NAME_MAX_SIZE (1024)

/* 入力をどこから読むか */
FILE *fp = NULL;

int main(int argc, char *argv[]) {
    TC_Value v = {0};
    char ifile[FILE_NAME_MAX_SIZE] = "";
    char ext[] = ".ml";
    if(2 == argc) {
        strncpy_s(ifile, FILE_NAME_MAX_SIZE, argv[1], strlen(argv[1]));
        strncat_s(ifile, FILE_NAME_MAX_SIZE, ext, strlen(ext));
        if( 0 != fopen_s(&fp, ifile, "r") ) {
            perror("fopen_s");
            return 1;
        }
        if( 0 == compile(argv[1]) ) printf("error\n");
        fclose(fp);
    } else {
        fp = stdin;
        printf("\tTiny Caml version 0.0.1\n\n");
        for(;;) {
            printf("# ");
            eval(&v);
        }
    }
    return 0;
}
