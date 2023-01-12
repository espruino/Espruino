/*
 * Test code for embeddable Espruino build
 */

#include <sys/time.h> // gettimeofday
#include "../../bin/espruino_embedded.h"
#include "../../bin/espruino_embedded_utils.h"

/** We have to define these */
uint64_t ejs_get_microseconds() {
  struct timeval tm;
  gettimeofday(&tm, 0);
  return (uint64_t)(tm.tv_sec)*1000000L + tm.tv_usec;
}
void ejs_print(const char *str) {
  printf("%s",str);
}
// ----------------------------------

// 
int main() {
  ejs_create(1000);
  struct ejs* ejs[2];
  ejs[0] = ejs_create_instance();
  ejs[1] = ejs_create_instance();
  printf("Embedded Espruino test.\n===========================\nTwo instances.\nType JS and hit enter, or type 'quit' to exit:\n0>");
  int instanceNumber = 0;

  char buf[1000];
  while (true) {
    fgets(buf, sizeof(buf), stdin);
    if (strcmp(buf,"quit\n")==0) break;
    JsVar *v = ejs_exec(ejs[instanceNumber], buf, false);
    // FIXME - we need a way to set the active interpreter to 'ejs' here for the js* functions
    instanceNumber = !instanceNumber; // toggle instance
    jsiConsolePrintf("=%v\n%d>", v, instanceNumber);
    jsvUnLock(v);
  }  

  ejs_destroy_instance(ejs[0]);
  ejs_destroy_instance(ejs[1]);
  ejs_destroy();
}


// BOARD=EMBED DEBUG=1 make
// gcc targets/embed/test.c bin/espruino_embedded.c -Isrc -lm -m32 -g
