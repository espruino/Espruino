/* REQUIRES -std=c99 FOR COMPILATION!
 *
 * TODO:
 *       print("press " + count++); fails, but print("press " + (count++)); works
 *       Garbage collection for nested references
 *       'for (i in array)' syntax
 *       'if ("key" in obj)' syntax
 *       function.call(scope)
 *       handle 'new Function() { X.call(this); Y.call(this); }' correctly
 *       'Array.prototype.clear = function () { this.X = 2e23; };'
 *       See if jsvNewVariableName/jsvAdd* can use pointers instead of refs?
 *       Lex could use JsVars in order to store potentially very big strings that it parses
 *       Handle errors gracefully (have an ERROR state in the JsExecFlags?)
 *       Could store vars in arrays/objects/functions as a binary tree instead of a linked list
 *       Possibly special array type that stores values directly, not with a Name
 *       Maybe keep track of whether JsVar was changed/written to?
 *       Memory manager to handle storing rarely used refs in flash
 *          - use binary tree to look up JsVar from its ref
 *          - maybe also linked list to keep track of what is used most often
 *       Add require(filename) function
 *       Add Array.push()/splice
 *       Add 'delete' keyword for killing array items?
 *       Currently, accessing an undefined array or object item creates it. Maybe that could be changed?
 *       Could get JsVar down to 20 bytes (4*N) so we can align it on a boundary. String equals can then compare whole 32 bit words
 *
 * In code:
 * TODO - should be fixed
 * FIXME - will probably break if used
 * OPT - potential for speed optimisation
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsfunctions.h"

JsParse p;
JsVarRef events = 0; 
bool isRunning = true;

void nativeQuit(JsVarRef var) {
  isRunning = false;
}

void nativeTrace(JsVarRef var) {
  jsvTrace(p.root, 0);
}

void nativePrint(JsVarRef var) {
  JsVar *text = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "text", false/*no create*/));
  char buf[256];
  jsvGetString(text, buf, 256);
  printf("PRINT: '%s'\n", buf);
  jsvUnLock(text);
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

void nativeAddEvent(JsVarRef var) {
  JsVar *func = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "func", false/*no create*/));
  // find the last event in our queue
  JsVar *lastEvent = 0;
  if (events) { 
    lastEvent = jsvLock(events);
    while (lastEvent->nextSibling) {
      JsVar *next = jsvLock(lastEvent->nextSibling);
      jsvUnLock(lastEvent);
      lastEvent = next;
    }
  }

  // if it is a single callback, just add it
  JsVar *event = jsvRef(jsvNewWithFlags(JSV_OBJECT|JSV_NATIVE));
  jsvUnLock(jsvAddNamedChild(jsvGetRef(event), jsvGetRef(func), "func"));
  if (lastEvent) { 
    lastEvent->nextSibling = jsvGetRef(event);
    jsvUnLock(lastEvent);
  } else
    events = jsvGetRef(event);
  jsvUnLock(lastEvent);
  jsvUnLock(event);
  jsvUnLock(func);
}

void jsmExecuteEvents() {
  while (events) {
    JsVar *event = jsvLock(events);
    // Get function to execute
    JsVar *func = jsvSkipNameAndUnlock(jsvFindChildFromString(jsvGetRef(event), "func", false));
    // free + go to next
    events = event->nextSibling;
    event->nextSibling = 0;
    jsvUnRef(event);
    jsvUnLock(event);

    // now run..
    if (func) jspExecuteFunction(&p, func);
    //jsPrint("Event Done\n");
    jsvUnLock(func);
  }
}

bool run_test(const char *filename) {
  printf("----------------------------------\n");
  printf("----------------------------- TEST %s \n", filename);
  struct stat results;
  if (!stat(filename, &results) == 0) {
    printf("Cannot stat file! '%s'\n", filename);
    return false;
  }
  int size = results.st_size;
  FILE *file = fopen( filename, "rb" );
  /* if we open as text, the number of bytes read may be > the size we read */
  if( !file ) {
     printf("Unable to open file! '%s'\n", filename);
     return false;
  }
  char *buffer = malloc(size+1);
  long actualRead = fread(buffer,1,size,file);
  buffer[actualRead]=0;
  buffer[size]=0;
  fclose(file);

  JsVar *v;
  events = 0;
  jsvInit();
  jspInit(&p);
  jspAddNativeFunction(&p, "function print(text)", nativePrint);
  jspAddNativeFunction(&p, "function addEvent(func)", nativeAddEvent);

  jsvUnLock(jspEvaluate(&p, buffer ));

  JsVar *result = jsvSkipNameAndUnlock(jsvFindChildFromString(p.root, "result", false/*no create*/));
  bool pass = jsvGetBool(result);
  jsvUnLock(result);

  jsmExecuteEvents();

  if (pass)
    printf("----------------------------- PASS %s\n", filename);
  else {
    printf("----------------------------------\n");
    printf("----------------------------- FAIL %s <-------\n", filename);
    jsvTrace(p.root, 0);
    printf("----------------------------- FAIL %s <-------\n", filename);
    printf("----------------------------------\n");
  }
  printf("BEFORE: %d Memory Records Used\n", jsvGetMemoryUsage());
 // jsvTrace(p.root, 0);
  jspKill(&p);
  printf("AFTER: %d Memory Records Used (should be 0!)\n", jsvGetMemoryUsage());
  jsvShowAllocated();
  jsvKill();
  printf("\n");


  free(buffer);
  return pass;
}


void run_all_tests() {
  int test_num = 1;
  int count = 0;
  int passed = 0;

  while (test_num<1000) {
    char fn[32];
    sprintf(fn, "../tests/test%03d.js", test_num);
    // check if the file exists - if not, assume we're at the end of our tests
    FILE *f = fopen(fn,"r");
    if (!f) break;
    fclose(f);

    if (run_test(fn))
      passed++;
    count++;
    test_num++;
  }

  if (count==0) printf("No tests found in ../tests/test*.js!\n");
  printf("--------------------------------------------------\n");
  printf(" %d of %d tests passed\n", passed, count);
  printf("--------------------------------------------------\n");
}


int main(int argc, char **argv) {

  if (argc==1) {
    printf("Interactive mode.\n");
  } else if (argc==2 && strcmp(argv[1],"test")==0) {
    run_all_tests();
    exit(0);
  } else if (argc==3 && strcmp(argv[1],"test")==0) {
      run_test(argv[2]);
      exit(0);
  } else {
    printf("USAGE:\n");
    printf("./TinyJSC           : JavaScript imemdiate mode\n");
    printf("./TinyJSC test      : Run Tests\n");
    printf("./TinyJSC test x.js : Run Single Test\n");
    exit(1);
  }

    char javaScript[256];
    JsVar *v;

    printf("Size of JsVar is now %d bytes'\n", (int)sizeof(JsVar));

    jsvInit();

	jspInit(&p);
	jspAddNativeFunction(&p, "function quit()", nativeQuit);
	jspAddNativeFunction(&p, "function trace()", nativeTrace);
	jspAddNativeFunction(&p, "function print(text)", nativePrint);
	jspAddNativeFunction(&p, "function setPin(pin, value)", nativeSetPin);
	jspAddNativeFunction(&p, "function getPin(pin)", nativeGetPin);
        jspAddNativeFunction(&p, "function inputa(value)", nativeInputA);
        jspAddNativeFunction(&p, "function addEvent(func)", nativeAddEvent);
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

      jsmExecuteEvents();
	}

	printf("BEFORE: %d Memory Records Used\n", jsvGetMemoryUsage());
	jspKill(&p);
	printf("AFTER: %d Memory Records Used (should be 0!)\n", jsvGetMemoryUsage());
	jsvShowAllocated();
	jsvKill();
	printf("Done!\n");
	return 0;
}
