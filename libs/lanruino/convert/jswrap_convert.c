#include "jswrap_convert.h"

/*JSON{
    "type":"library",
    "class":"convert"
}*/


/*JSON{
    "type":"staticmethod",
    "class":"convert",
    "name":"isLittleEndian",
    "generate":"jswrap_convert_isLittleEndian",
    "return":["bool",""]
}*/

bool jswrap_convert_isLittleEndian(){
    uint64_t x=0x12345678;
    uint8_t *p=&x;
    //jsiConsolePrintf("sizeof float:%d\r\n",sizeof(float));//4
    //jsiConsolePrintf("sizeof double:%d\r\n",sizeof(double));//8
    //jsiConsolePrintf("sizeof int:%d\r\n",sizeof(int));//4
    //jsiConsolePrintf("sizeof JsVarInt:%d\r\n",sizeof(JsVarInt));//4
    //jsiConsolePrintf("sizeof JsVarFloat:%d\r\n",sizeof(JsVarFloat));//8
    return (*p=0x78);
}

//JsVar *jswrap_convert_floatToBytes(JsVar *floatVar);
//JsVar *jswrap_convert_doubleToBytes(JsVar *doubleVar);
//JsVarFloat jswrap_convert_BytesToFloat(JsVar *bytes);
//JsVarFloat jswrap_convert_BytesToDouble(JsVar *bytes); 
//JsVarInt jswrap_convert_BytesToInt32(JsVar *bytes);