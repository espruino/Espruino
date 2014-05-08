#include "jsutils.h"
#include "jsvar.h"
#include "jsparse.h"
#include "jsinteractive.h"

bool jswrap_Pipe_idle();
void jswrap_Pipe_kill();

//fs.pipe(source,destination,chunkSize,callback)
void wrap_Pipe(JsVar* source, JsVar* dest, JsVar* chunkSize, JsVar* callback);
bool _pipe(JsVar* source, JsVar* destination, JsVar* chunkSize, JsVar* position);

JsVar* PipeGetArray(const char *name, bool create);
bool HasFunction(JsVar * object, char* FunctionName);