/*
 * Test code for embeddable Espruino build
 */

#include <sys/time.h> // gettimeofday
#include "../../espruino_embedded.h"

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

  struct ejs *ejs = ejs_create(1000);
  printf("Embedded Espruino test. Type JS and hit enter:\n>");

  char buf[1000];
  while (true) {
    fgets(buf, sizeof(buf), stdin);
    struct JsVar *v = ejs_exec(ejs, buf);
    jsiConsolePrintf("=%v\n>", v);
    jsvUnLock(v);
  }  
  ejs_destroy(ejs);
}


// V=1 BOARD=EMBED DEBUG=1 make sourcecode
// gcc espruino_embed.c test.c -lm
