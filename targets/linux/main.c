#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "jslex.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsfunctions.h"

#include "jsinteractive.h"
#include "jshardware.h"

#define TEST_DIR "tests/"

bool isRunning = true;

void nativeQuit(JsVarRef var) {
  NOT_USED(var);
  isRunning = false;
}

void nativeInterrupt(JsVarRef var) {
  NOT_USED(var);
  jspSetInterrupted(true);
}

bool run_test(const char *filename) {
  printf("----------------------------------\r\n");
  printf("----------------------------- TEST %s \r\n", filename);
  struct stat results;
  if (!stat(filename, &results) == 0) {
    printf("Cannot stat file! '%s'\r\n", filename);
    return false;
  }
  int size = (int)results.st_size;
  FILE *file = fopen( filename, "rb" );
  /* if we open as text, the number of bytes read may be > the size we read */
  if( !file ) {
     printf("Unable to open file! '%s'\r\n", filename);
     return false;
  }
  char *buffer = malloc(size+1);
  size_t actualRead = fread(buffer,1,size,file);
  buffer[actualRead]=0;
  buffer[size]=0;
  fclose(file);


  jshInit();
  jsiInit(false /* do not autoload!!! */);

  jspAddNativeFunction(jsiGetParser(), "function quit()", nativeQuit);
  jspAddNativeFunction(jsiGetParser(), "function interrupt()", nativeInterrupt);

  jsvUnLock(jspEvaluate(jsiGetParser(), buffer ));

  isRunning = true;
  while (isRunning && jsiHasTimers()) {
    jsiLoop();
  }

  JsVar *result = jsvSkipNameAndUnLock(jsvFindChildFromString(jsiGetParser()->root, "result", false/*no create*/));
  bool pass = jsvGetBool(result);
  jsvUnLock(result);

  if (pass)
    printf("----------------------------- PASS %s\r\n", filename);
  else {
    printf("----------------------------------\r\n");
    printf("----------------------------- FAIL %s <-------\r\n", filename);
    jsvTrace(jsiGetParser()->root, 0);
    printf("----------------------------- FAIL %s <-------\r\n", filename);
    printf("----------------------------------\r\n");
  }
  printf("BEFORE: %d Memory Records Used\r\n", jsvGetMemoryUsage());
 // jsvTrace(jsiGetParser()->root, 0);
  jsiKill();
  printf("AFTER: %d Memory Records Used\r\n", jsvGetMemoryUsage());
  jsvGarbageCollect();
  printf("AFTER GC: %d Memory Records Used (should be 0!)\r\n", jsvGetMemoryUsage());
  jsvShowAllocated();
  jshKill();

  //jsvDottyOutput();
  printf("\r\n");

  free(buffer);
  return pass;
}


bool run_all_tests() {
  int test_num = 1;
  int count = 0;
  int passed = 0;

  char *fails = malloc(1);
  fails[0] = 0;

  while (test_num<1000) {
    char fn[32];
    sprintf(fn, TEST_DIR"test%03d.js", test_num);
    // check if the file exists - if not, assume we're at the end of our tests
    FILE *f = fopen(fn,"r");
    if (!f) break;
    fclose(f);

    if (run_test(fn)) {
      passed++;
    } else {
      char *t = malloc(strlen(fails)+3+strlen(fn));
      strcpy(t, fails);
      strcat(t,fn);
      strcat(t,"\r\n");  
      free(fails);
      fails  =t;
    }
    count++;
    test_num++;
  }

  if (count==0) printf("No tests found in "TEST_DIR"test*.js!\r\n");
  printf("--------------------------------------------------\r\n");
  printf(" %d of %d tests passed\r\n", passed, count);
  if (passed!=count) {
   printf("FAILS:\r\n%s", fails);
  }
  printf("--------------------------------------------------\r\n");
  free(fails);
  return passed == count;
}

bool run_memory_test(const char *fn, int vars) {
  int i;
  int min = 20;
  int max = 100;
  if (vars>0) {
    min = vars;
    max = vars+1;
  }
  for (i=min;i<max;i++) {
    jsvSetMaxVarsUsed(i);
    printf("----------------------------------------------------- MEMORY TEST WITH %d VARS\n", i);
    run_test(fn);
  }
  return true;
}

bool run_memory_tests(int vars) {
  int test_num = 1;
  int count = 0;
  int passed = 0;

  while (test_num<1000) {
    char fn[32];
    sprintf(fn, TEST_DIR"test%03d.js", test_num);
    // check if the file exists - if not, assume we're at the end of our tests
    FILE *f = fopen(fn,"r");
    if (!f) break;
    fclose(f);

    run_memory_test(fn, vars);
    test_num++;
  }

  if (count==0) printf("No tests found in "TEST_DIR"test*.js!\n");
  return true;
}


void sig_handler(int sig)
{
  printf("Got Signal %d\n",sig);fflush(stdout);
    if (sig==SIGINT) 
      jspSetInterrupted(true);
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
  } else if (argc==2 && strcmp(argv[1],"mem")==0) {
    bool ok = run_memory_tests(0);
    exit(ok ? 0 : 1);
  } else if (argc==3 && strcmp(argv[1],"mem")==0) {
      bool ok = run_memory_test(argv[2], 0);
      exit(ok ? 0 : 1);
  } else if (argc==4 && strcmp(argv[1],"mem")==0) {
      bool ok = run_memory_test(argv[2], atoi(argv[3]));
      exit(ok ? 0 : 1);
  } else {
    printf("USAGE:\n");
    printf("./TinyJSC            : JavaScript imemdiate mode\n");
    printf("./TinyJSC test       : Run Tests\n");
    printf("./TinyJSC test x.js  : Run Single Test\n");
    printf("./TinyJSC mem        : Run Exhaustive Memory crash test\n");
    printf("./TinyJSC mem        : Run Exhaustive Memory crash test for Single test\n");
    printf("./TinyJSC mem x.js # : Run Memory crash test for one amount of vars\n");
    exit(1);
  }

  printf("Size of JsVar is now %d bytes\n", (int)sizeof(JsVar));
  printf("Size of JsVarRef is now %d bytes\n", (int)sizeof(JsVarRef));

  struct sigaction sa;
  sa.sa_handler = sig_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIGINT, &sa, NULL) == -1)
    printf("Adding SIGINT hook failed\n");
  else
    printf("Added SIGINT hook\n");
  if (sigaction(SIGHUP, &sa, NULL) == -1)
    printf("Adding SIGHUP hook failed\n");
  else
    printf("Added SIGHUP hook\n");
  if (sigaction(SIGTERM, &sa, NULL) == -1)
    printf("Adding SIGTERM hook failed\n");
  else
    printf("Added SIGTERM hook\n");

  jshInit();
  jsiInit(true);

  jspAddNativeFunction(jsiGetParser(), "function quit()", nativeQuit);
  jspAddNativeFunction(jsiGetParser(), "function interrupt()", nativeInterrupt);

  while (isRunning) {
    jsiLoop();
  }
  jsiConsolePrint("\n");
  jsiKill();

  jsvShowAllocated();
  jshKill();

  return 0;
}
