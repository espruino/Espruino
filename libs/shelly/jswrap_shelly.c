#include "jsvar.h"

/*JSON{
  "type" : "library",
  "class" : "shelly"
}

Simple library for testing
*/


/*JSON{
  "type" : "staticmethod",
  "class" : "shelly",
  "name" : "print",
  "generate" : "jswrap_shelly_print",
  "params" : [
    ["data","JsVar","The data to print"]
  ]
}
*/
void jswrap_shelly_print(JsVar *data) {
  char buf[100];
  jsvGetString(data, buf, 100);
  printf("shelly.print: %s\n", buf);
}

/*JSON{
  "type" : "class",
  "class" : "Shelly"
}

Simple object for testing
*/


/*JSON{
  "type" : "staticmethod",
  "class" : "Shelly",
  "name" : "print",
  "generate" : "jswrap_Shelly_print",
  "params" : [
    ["data","JsVar","The data to print"]
  ]
}
*/
void jswrap_Shelly_print(JsVar *data) {
  char buf[100];
  jsvGetString(data, buf, 100);
  printf("Shelly.print: %s\n", buf);
}
