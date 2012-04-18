/*
 ============================================================================
 Name        : TinyJSC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/* REQUIRES -std=c99 FOR COMPILATION! */

#include <stdio.h>
#include <stdlib.h>
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"

int main(void) {
    jsvInit();
	printf("!!!Hello World!!!\n");

	JsVarRef s = jsvNewFromString("Hello. This is a test of very very very very long strings spanning over multiple JsVars");

    char buf[256];
	JsVar *v = jsvLock(s);
	jsvGetString(v, buf, 256);
	jsvUnLock(s);

	puts(buf);

	jsvKill();
	return EXIT_SUCCESS;
}
