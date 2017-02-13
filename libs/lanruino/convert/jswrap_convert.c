/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 lancer
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

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
/*JSON{
    "type":"staticmethod",
    "class":"convert",
    "name":"floatToBytes",
    "generate":"jswrap_convert_floatToBytes",
    "params":[
        ["floatVar","JsVar",""]
    ],
    "return":["JsVar",""]
}*/
JsVar *jswrap_convert_floatToBytes(JsVar *floatVar){
    if(jsvIsFloat(floatVar)){
        float floatVal=(float)jsvGetFloat(floatVar);
        JsVar *bytesVar=jsvNewEmptyArray();
        uint8_t *ptr=&floatVal;
        for(int i=0;i<sizeof(float);i++){
            jsvArrayPushAndUnLock(bytesVar,jsvNewFromInteger(*(ptr+i)));
        }   
        return bytesVar;
    }else{
        jsExceptionHere(JSET_ERROR, "arguments must type of Float");
        return 0;
    }
}
/*JSON{
    "type":"staticmethod",
    "class":"convert",
    "name":"doubleToBytes",
    "generate":"jswrap_convert_doubleToBytes",
    "params":[
        ["doubleVar","JsVar",""]
    ],
    "return":["JsVar",""]
}
*/
JsVar *jswrap_convert_doubleToBytes(JsVar *doubleVar){
    if(jsvIsFloat(doubleVar)){
        double doubleVal=(double)jsvGetFloat(doubleVar);
        JsVar *bytesVar=jsvNewEmptyArray();
        uint8_t *ptr=&doubleVal;
        for(int i=0;i<sizeof(double);i++){
            jsvArrayPushAndUnLock(bytesVar,jsvNewFromInteger(*(ptr+i)));
        }   
        return bytesVar;
    }else{
        jsExceptionHere(JSET_ERROR, "arguments must type of Float");
        return 0;
    }
}
/*JSON{
    "type":"staticmethod",
    "class":"convert",
    "name":"bytesToFloat",
    "generate":"jswrap_convert_bytesToFloat",
    "params":[
        ["bytes","JsVar",""]
    ],
    "return":["JsVar",""]
}*/
JsVar *jswrap_convert_bytesToFloat(JsVar *bytes){
    if(jsvIsArray(bytes)&&jsvGetArrayLength(bytes)==sizeof(float)){
        uint8_t buf[sizeof(float)];
        float *ptr=buf;
        for(int i=0;i<sizeof(float);i++){
            buf[i]=(uint8_t)jsvGetInteger(jsvGetArrayItem(bytes,i));
        }
        return jsvNewFromFloat(*ptr);
    }else{
        jsExceptionHere(JSET_ERROR,"arguments must type of Array and Array length is %d",sizeof(float));
        return 0;
    }
}
/*JSON{
    "type":"staticmethod",
    "class":"convert",
    "name":"bytesToDouble",
    "generate":"jswrap_convert_bytesToDouble",
    "params":[
        ["bytes","JsVar",""]
    ],
    "return":["JsVar",""]
}*/
JsVar *jswrap_convert_bytesToDouble(JsVar *bytes){
    if(jsvIsArray(bytes)&&jsvGetArrayLength(bytes)==sizeof(double)){
        uint8_t buf[sizeof(double)];
        double *ptr=buf;
        for(int i=0;i<sizeof(double);i++){
            buf[i]=(uint8_t)jsvGetInteger(jsvGetArrayItem(bytes,i));
        }
        return jsvNewFromFloat(*ptr);
    }else{
        jsExceptionHere(JSET_ERROR,"arguments must type of Array and Array length is %d",sizeof(double));
        return 0;
    }
}
