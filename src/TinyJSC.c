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
 *       Add Deep copy! Now function params > 8 chars cause a failure!
 *       See if jsvNewVariableName/jsvAdd* can use pointers instead of refs?
 *       Handle errors gracefully (have an ERROR state in the JsExecFlags?)
 *       Could store vars in arrays/objects/functions as a binary tree instead of a linked list
 *       Possibly special array type that stores values directly, not with a Name
 *       Maybe keep track of whether JsVar was changed/written to?
 *       Memory manager to handle storing rarely used refs in flash
 *          - use binary tree to look up JsVar from its ref
 *          - maybe also linked list to keep track of what is used most often
 *       Add a function to handle built-in functions without using up RAM
 *       Add require(filename) function
 *       Add Array.push()/indexOf/splice
 *       Add String builtins
 *       Add 'delete' keyword for killing array items?
 *       Currently, accessing an undefined array or object item creates it. Maybe that could be changed?
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
#include "jsfunctions.h"

JsParse p;
bool isRunning = true;

void nativeQuit(JsVarRef var) {
  isRunning = false;
}

void nativeTrace(JsVarRef var) {
  jsvTrace(p.root, 0);
}

void nativePrint(JsVarRef var) {
  JsVar *text = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "text", false/*no create*/));
  char buf[64];
  jsvGetString(text, buf, 64);
  printf("PRINT: '%s'\n", buf);
  jsvUnLock(text);
}

void nativeJSONStringify(JsVarRef var) {
  JsVar *data = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "data", false/*no create*/));
  JsVar *returnValue = jsvFindChildFromString(var, JSPARSE_RETURN_VAR, false/*no create*/); // no skip - because we want the name to write to
  JsVar *result = jsvNewFromString("");
  jsfGetJSON(data, result);
  jsvUnLock(jsvSetValueOfName(returnValue, result));
  jsvUnLock(result);
  jsvUnLock(data);
}

void nativeEval(JsVarRef var) {
  JsVar *js = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "js", false/*no create*/));
  JsVar *returnValue = jsvFindChildFromString(var, JSPARSE_RETURN_VAR, false/*no create*/); // no skip - because we want the name to write to
  JsVar *result = jspEvaluateVar(&p, js);
  jsvUnLock(jsvSetValueOfName(returnValue, result));
  jsvUnLock(result);
  jsvUnLock(js);
}

void nativeSetPin(JsVarRef var) {
  JsVar *pin = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "pin", false/*no create*/));
  JsVar *value = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "value", false/*no create*/));
  printf("Setting pin %d to value %d\n", (int)jsvGetInteger(pin),  (int)jsvGetInteger(value));
  jsvUnLock(pin);
  jsvUnLock(value);
}

void nativeGetPin(JsVarRef var) {
  JsVar *pin = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "pin", false/*no create*/));
  JsVar *returnValue = jsvFindChildFromString(var, JSPARSE_RETURN_VAR, false/*no create*/); // no skip - because we want the name to write to
  int actualValue = 1;
  JsVar *newValue;
  printf("Getting value of pin %d (it was %d)\n", (int)jsvGetInteger(pin), actualValue);
  newValue = jsvNewFromInteger(actualValue);
  jsvUnLock(jsvSetValueOfName(returnValue, newValue));
  jsvUnLock(newValue);
  jsvUnLock(pin);
}

void nativeInputA(JsVarRef var) {
  JsVar *value = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "value", false/*no create*/));
  JsVar *returnValue = jsvFindChildFromString(var, JSPARSE_RETURN_VAR, false/*no create*/); // no skip - because we want the name to write to
  static int this_is_my_pin = 1;
  if (!jsvIsUndefined(value)) {
     this_is_my_pin = (int)jsvGetInteger(value);
     printf("Setting INPUTA to %d\n", this_is_my_pin);
  }

  JsVar *newValue;
  printf("Getting INPUTA (it was %d)\n", this_is_my_pin);
  newValue = jsvNewFromInteger(this_is_my_pin);
  jsvUnLock(jsvSetValueOfName(returnValue, newValue));
  jsvUnLock(newValue);
  jsvUnLock(value);
}


int main(void) {


    char javaScript[256];

    JsVar *v;

    printf("Size of JsVar is now %d bytes'\n", (int)sizeof(JsVar));

    jsvInit();

	jspInit(&p);
	jspAddNativeFunction(&p, "function quit()", nativeQuit);
	jspAddNativeFunction(&p, "function trace()", nativeTrace);
	jspAddNativeFunction(&p, "function print(text)", nativePrint);
    jspAddNativeFunction(&p, "function JSON.stringify(data,xxx)", nativeJSONStringify);
    jspAddNativeFunction(&p, "function eval(js)", nativeEval);
	jspAddNativeFunction(&p, "function setPin(pin, value)", nativeSetPin);
	jspAddNativeFunction(&p, "function getPin(pin)", nativeGetPin);
    jspAddNativeFunction(&p, "function inputa(value)", nativeInputA);
	//v = jspEvaluate(&p, "print('Hello World from JavaScript!');for (i=0;i<10;i++) { setPin(1, (i&1) ^ getPin(1)); }" );
	//v = jspEvaluate(&p, "var Z = 1+2; if (Z==4) X=1; else Y=1; var A = [1,2,3]; var B={ a:1, b:2, c:3 };B.c" );
	//v = jspEvaluate(&p, "var Z = []; Z[0] = 'hello'; Z[1] = 'world'; Z[0]+' '+Z[1]" );
	//v = jspEvaluate(&p, "var a = 1;for (i=0;i<5;i++) a=a*2; a" );
	//v = jspEvaluate(&p, "var a = 1;while (a<5) a=a*1.1; a" );
    //v = jspEvaluate(&p, "function foo(a,b) { return a+b; } var bar=function (a,b) { return a*b; };foo(1,2)" );
	// hacky fibonnacci
	//v = jspEvaluate(&p, "function fib(a,b,cnt) { if (cnt<=0) return a; return fib(b,a+b,cnt-1); } var fibs=[]; for (i=0;i<7;i++) fibs[i] = fib(1,1,i);" );
	//v = jspEvaluate(&p, "var Z = 1+2;function a() {};a();" ); // cope with no return
	//v = jspEvaluate(&p, "for (i=0;i<7;i++) ;" ); // had a memory leak -> no more!
	//v = jspEvaluate(&p, "1+2" );
	//v = jspEvaluate(&p, "var A=[];A[1]=2;" );
	//v = jspEvaluate(&p, "function aVeryVeryVeryLongFunctionName() {}" );
	//v = jspEvaluate(&p, "function aVeryVeryVeryLongFunctionNameThatIsFarTooLong() { print('Hello!'); };aVeryVeryVeryLongFunctionNameThatIsFarTooLong();" );
	//v = jspEvaluate(&p, "var A = {a:1};A.a;" );

	printf("Please enter some JavaScript, or:\n");
	printf("   quit(); to exit\n");
	printf("   trace(); to dump contents of memory\n");
	isRunning = true;
	while (isRunning) {
      printf(">");

      gets(javaScript);
      v = jspEvaluate(&p, javaScript );
      printf("RESULT : ");
      jsvTrace(jsvGetRef(v), 0);
      jsvUnLock(v);
	}

	printf("BEFORE: %d Memory Records Used\n", jsvGetMemoryUsage());
	jspKill(&p);
	printf("AFTER: %d Memory Records Used (should be 0!)\n", jsvGetMemoryUsage());
	jsvShowAllocated();


	jsvKill();
	printf("Done!\n");
	return 0;
}
