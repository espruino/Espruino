/** Wrapper file */
#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

#include "ff.h" // filesystem stuff

#define JS_DIR_BUF_SIZE 64

#if _USE_LFN
  #define GET_FILENAME(Finfo) *Finfo.lfname ? Finfo.lfname : Finfo.fname
#else
  #define GET_FILENAME(Finfo) Finfo.fname
#endif

FATFS jsfsFAT;

void jsfsWarn(const char *str, FRESULT res) {
  char buf[JS_ERROR_BUF_SIZE];
  strncpy(buf, str, JS_ERROR_BUF_SIZE);
  strncat(buf, "(code ", JS_ERROR_BUF_SIZE);
  itoa(res, &buf[strlen(buf)], 10);
  strncat(buf, ")", JS_ERROR_BUF_SIZE);
  jsWarn(buf);
}

bool jsfsInit() {
  static bool inited = false;

  if (!inited) {
    FRESULT res;
    if ((res = f_mount(0, &jsfsFAT)) != FR_OK) {
      jsfsWarn("Unable to mount SD card", res);
      return false;
    }
    inited = true;
  }
  return true;
}

/*JSON{ "type":"method",
         "class" : "fs", "name" : "list",
         "wrap" : "wrap_fs_list",
         "desc" : [ "List all files in the supplied directory" ],
         "params" : { "path" : "The path of the directory to list. If it is not supplied, '' is assumed, which will list the root directory" },
         "return" : "An array of filename strings"
}*/
JsVar *wrap_fs_list(JsVar *parent, JsVar *parentName) {
  JsVar *arr = jsvNewWithFlags(JSV_ARRAY);
  if (!arr) return 0; // out of memory

  JsVar *dirVar = jspParseSingleFunction();
  char dirStr[JS_DIR_BUF_SIZE] = "";
  if (dirVar) {
    jsvGetString(dirVar, dirStr, JS_DIR_BUF_SIZE);
    jsvUnLock(dirVar);
  }

  DIR dirs;
  FILINFO Finfo;
  FRESULT res = 0;

  if (jsfsInit()) {
    jsiConsolePrint("Scanning...\n");
    if ((res=f_opendir(&dirs, dirStr)) == FR_OK) {
      jsiConsolePrint("Opened...\n");
      while (((res=f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
        char *fn = GET_FILENAME(Finfo);
        JsVar *fnVar = jsvNewFromString(fn);
        if (fnVar) // out of memory
          jsvArrayPush(arr, fnVar);

        /*if (Finfo.fattrib & AM_DIR) {
            jsiConsolePrint(" DIR ");
            jsiConsolePrint(fn);
            jsiConsolePrint("\n");
        } else {
          jsiConsolePrint(" FILE ");
          jsiConsolePrint(fn);
          jsiConsolePrint("\n");
        }*/

      }
    }
    if (res==FR_OK) {
      jsiConsolePrint("Unmounting...\n");
      res = f_mount(0, 0);
    }
  }
  jsiConsolePrint("Code ");
  jsiConsolePrintInt(res);
  jsiConsolePrint("\nDone.\n");

  return arr;
}

/*JSON{ "type":"method",
         "class" : "Math", "name" : "sin",
         "wrap" : "wrap_math_sin",
         "params" : { "theta" : "The angle to get the sine of" },
         "return" : "The sine of theta"
}*/
/*JSON{ "type":"method",
         "class" : "Math", "name" : "atan",
         "wrap" : "wrap_math_atan",
         "params" : { "theta" : "The angle to get the sine of" },
         "return" : "The sine of theta"
}*/
/*JSON{ "type":"method",
         "class" : "Math", "name" : "atan2",
         "wrap" : "wrap_math_atan2",
         "params" : { "theta" : "The angle to get the sine of" },
         "return" : "The sine of theta"
}*/

/*JSON{ "type":"function",
         "name" : "eval",
         "wrap" : "wrap_eval",
         "params" : { "code" : "The code to evaluate" },
         "return" : "What code evaluated to"
}*/

