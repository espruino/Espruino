/* REQUIRES -std=c99 FOR COMPILATION!
 *
 * TODO:
 *       IF..ELSE IS BROKEN WITH STATEMENTS. SEMI COLON ISSUE
 *       Memory leaks when errors - test cases? Maybe just do leak check after an error has occurred
 *       Memory leak cleanup code - to try and clean up if memory has been leaked
 *       break/continue
 *       Garbage collection for nested references
 *       'if ("key" in obj)' syntax
 *       function.call(scope)
 *       handle 'new Function() { X.call(this); Y.call(this); }' correctly
 *       'Array.prototype.clear = function () { this.X = 2e23; };'
 *       See if jsvNewVariableName/jsvAdd* can use pointers instead of refs?
 *       Handle errors gracefully (have an ERROR state in the JsExecFlags?)
 *       Add Array.splice
 *       Add 'delete' keyword for killing array items?
 *       Could get JsVar down to 20 bytes (4*N) so we can align it on a boundary. String equals can then compare whole 32 bit words
 *
 *  LOW PRIORITY
 *       Rename IO functions to Arduino style: http://arduino.cc/en/Reference/HomePage
 *       Automatically convert IDs in form A#,A##,B#,B## etc into numbers.
 *       Lex could use JsVars in order to store potentially very big strings that it parses
 *       Could store vars in arrays/objects/functions as a binary tree instead of a linked list
 *       Maybe keep track of whether JsVar was changed/written to? jsvLockWritable
 *       Memory manager to handle storing rarely used refs in flash
 *          - use binary tree to look up JsVar from its ref
 *          - maybe also linked list to keep track of what is used most often
 *       Add require(filename) function
 *       Currently, accessing an undefined array or object item creates it. Maybe that could be changed?
 *
 * In code:
 * TODO - should be fixed
 * FIXME - will probably break if used
 * OPT - potential for speed optimisation
 *
 *
 FIXME: save(); close, start, save(); breaks!
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsfunctions.h"

#include "jsinteractive.h"
#include "jshardware.h"


bool isRunning = true;

void nativeQuit(JsVarRef var) {
  isRunning = false;
}

void nativePrint(JsVarRef var) {
  JsVar *text = jsvSkipNameAndUnlock(jsvFindChildFromString(var, "text", false/*no create*/));
  char buf[256];
  jsvGetString(text, buf, 256);
  printf("PRINT: '%s'\n", buf);
  jsvUnLock(text);
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

  jsvInit();
  JsParse p;
  jspInit(&p);
  jspAddNativeFunction(&p, "function print(text)", nativePrint);

  jsvUnLock(jspEvaluate(&p, buffer ));

  JsVar *result = jsvSkipNameAndUnlock(jsvFindChildFromString(p.root, "result", false/*no create*/));
  bool pass = jsvGetBool(result);
  jsvUnLock(result);

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


bool run_all_tests() {
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
  return passed == count;
}


int main(int argc, char **argv) {

  if (argc==1) {
    printf("Interactive mode.\n");
  } else if (argc==2 && strcmp(argv[1],"test")==0) {
    bool ok = run_all_tests();
    exit(ok ? 0 : 1);
  } else if (argc==3 && strcmp(argv[1],"test")==0) {
    bool ok = run_test(argv[2]);
    exit(ok ? 0 : 1);
  } else {
    printf("USAGE:\n");
    printf("./TinyJSC           : JavaScript imemdiate mode\n");
    printf("./TinyJSC test      : Run Tests\n");
    printf("./TinyJSC test x.js : Run Single Test\n");
    exit(1);
  }

  printf("Size of JsVar is now %d bytes'\n", (int)sizeof(JsVar));

  jshInit();
  jsiInit(true);

  jspAddNativeFunction(jsiGetParser(), "function quit()", nativeQuit);

  while (isRunning) {
    jsiLoop();
  }
  jsPrint("\n");
  jsiKill();

  jsvShowAllocated();
  jshKill();
}
