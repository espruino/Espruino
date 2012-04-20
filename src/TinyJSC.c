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
 * TODO:
 *       See if we can remove jsvUnlock and just use UnlockPtr
 *       See if jsvNewVariableName/jsvAdd* can use pointers instead of refs?
 *       Handle errors gracefully (have an ERROR state in the JsExecFlags?)
 *
 * In code:
 * TODO - should be fixed
 * FIXME - will probably break if used
 * OPT - potential for speed optimisation
 * */

#include <stdio.h>
#include <stdlib.h>
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"

void nativePrint(JsVarRef var) {
  JsVar *text = jsvSkipNameAndUnlock(jsvFindChild(var, "text", false/*no create*/));
  char buf[64];
  jsvGetString(text, buf, 64);
  printf("PRINT: '%s'\n", buf);
  jsvUnLockPtr(text);
}

void nativeSetPin(JsVarRef var) {
  JsVar *pin = jsvSkipNameAndUnlock(jsvFindChild(var, "pin", false/*no create*/));
  JsVar *value = jsvSkipNameAndUnlock(jsvFindChild(var, "value", false/*no create*/));
  printf("Setting pin %d to value %d\n", (int)jsvGetInteger(pin),  (int)jsvGetInteger(value));
  jsvUnLockPtr(pin);
  jsvUnLockPtr(value);
}

void nativeGetPin(JsVarRef var) {
  JsVar *pin = jsvSkipNameAndUnlock(jsvFindChild(var, "pin", false/*no create*/));
  JsVar *returnValue = jsvFindChild(var, JSPARSE_RETURN_VAR, false/*no create*/); // no skip - because we want the name to write to
  int actualValue = 1;
  JsVar *newValue;
  printf("Getting value of pin %d (it was %d)\n", (int)jsvGetInteger(pin), actualValue);
  newValue = jsvNewFromInteger(actualValue);
  jsvUnLockPtr(jsvSetValueOfName(returnValue, newValue));
  jsvUnLockPtr(newValue);
  jsvUnLockPtr(pin);
}


int main(void) {
    JsParse p;
    JsVar *v;

    jsvInit();

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


	jspInit(&p);
	jspAddNativeFunction(&p, "function print(text)", nativePrint);
	jspAddNativeFunction(&p, "function setPin(pin, value)", nativeSetPin);
	jspAddNativeFunction(&p, "function getPin(pin)", nativeGetPin);
	//v = jspEvaluate(&p, "print('Hello World from JavaScript!');for (i=0;i<10;i++) { setPin(1, (i&1) ^ getPin(1)); }" );
	//v = jspEvaluate(&p, "var Z = 1+2+__ONE; if (Z==4) X=1; else Y=1; var A = [1,2,3]; var B={ a:1, b:2, c:3 };B.c" );
	//v = jspEvaluate(&p, "var Z = []; Z[0] = 'hello'; Z[1] = 'world'; Z[0]+' '+Z[1]" );
	//v = jspEvaluate(&p, "var a = 1;for (i=0;i<5;i++) a=a*2; a" );
	//v = jspEvaluate(&p, "var a = 1;while (a<5) a=a*1.1; a" );
    //v = jspEvaluate(&p, "function foo(a,b) { return a+b; } var bar=function (a,b) { return a*b; };foo(1,2)" );
	// hacky fibonnacci
	//v = jspEvaluate(&p, "function fib(a,b,cnt) { if (cnt<=0) return a; return fib(b,a+b,cnt-1); } var fibs=[]; for (i=0;i<7;i++) fibs[i] = fib(1,1,i);" );
	//v = jspEvaluate(&p, "var Z = 1+2;function a() {};a();" ); // cope with no return
	//v = jspEvaluate(&p, "for (i=0;i<7;i++) ;" ); // had a memory leak -> no more!
	v = jspEvaluate(&p, "1+2" );

	if (v) {
      char buf[256];
      jsvGetString(v, buf, 256);
      jsvUnLockPtr(v);
      printf("RESULT : '%s'\n", buf);
	} else
	  printf("NO RESULT\n");

	jsvTrace(p.root, 0);

	printf("BEFORE: %d Memory Records Used\n", jsvGetMemoryUsage());
	jspKill(&p);
	printf("AFTER: %d Memory Records Used (should be 0!)\n", jsvGetMemoryUsage());
	jsvShowAllocated();


	jsvKill();
	printf("Done!\n");
	return EXIT_SUCCESS;
}
