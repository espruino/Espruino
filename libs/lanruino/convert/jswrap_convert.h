#include "jsvar.h"
#include "jsvariterator.h"
#include "jsinteractive.h"

bool jswrap_convert_isLittleEndian();
JsVar *jswrap_convert_floatToBytes(JsVar *floatVar);
JsVar *jswrap_convert_doubleToBytes(JsVar *doubleVar);
JsVarFloat jswrap_convert_BytesToFloat(JsVar *bytes);
JsVarFloat jswrap_convert_BytesToDouble(JsVar *bytes); 
JsVarInt jswrap_convert_BytesToInt32(JsVar *bytes);