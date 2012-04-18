/*
 ============================================================================
 Name        : TinyJSC.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/* REQUIRES -std=c99 FOR COMPILATION!
 *
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"

int main(void) {
    jsvInit();
	printf("!!!Hello World!!!\n");

	/*JsVarRef s = jsvNewFromString("Hello. This is a test of very very very very long strings spanning over multiple JsVars");
    char buf[256];
	JsVar *v = jsvLock(s);
	jsvGetString(v, buf, 256);
	jsvUnLock(s);

	puts(buf);*/

	/*const char *codeString = "print('Hello World!');";
	JsVarRef code = jsvNewFromString(codeString);
	JsVar *codep = jsvLock(code);
	printf("%d vs %d\n", (int)strlen(codeString), jsvGetStringLength(codep));
	jsvUnLockPtr(codep);*/

/*	JsLex lex;
	jslInit(&lex, code, 0, strlen(codeString));
	while (lex.tk != LEX_EOF) {
	  jslGetTokenString(&lex, buf, 256);
	  jslGetNextToken(&lex);
	  printf("TOKEN: %s\n",buf);
	}
    jslKill(&lex);*/

	JsParse p;
	jspInit(&p);

	JsVar *v = jspEvaluate(&p, "Z = 1+2");
	char buf[256];
    jsvGetString(v, buf, 256);
    jsvUnLockPtr(v);
    printf("RESULT : '%s'\n", buf);
	jspKill(&p);
	jsvTrace(p.root, 0);

	jsvKill();
	printf("Done!\n");
	return EXIT_SUCCESS;
}
