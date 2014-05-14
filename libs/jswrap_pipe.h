#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

bool jswrap_pipe_idle();
void jswrap_pipe_kill();

void jswrap_pipe(JsVar* source, JsVar* dest, JsVar* chunkSize, JsVar* callback);
