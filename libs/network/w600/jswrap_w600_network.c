#include "jswrap_w600_network.h"
#include "network.h"

void jswrap_wifi_connect(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback){

}

void jswrap_wifi_disconnect(JsVar *jsCallback){

}

void jswrap_wifi_startAP(JsVar *jsSsid, JsVar *jsOptions, JsVar *jsCallback){

}

void jswrap_wifi_stopAP(JsVar *jsCallback){

}

void jswrap_wifi_scan(JsVar *jsCallback){

}

void jswrap_wifi_save(JsVar *what){

}

void jswrap_wifi_restore(void){

}

JsVar *jswrap_wifi_getStatus(JsVar *jsCallback){
 return 0;
}

void jswrap_wifi_setConfig(JsVar *jsOptions){

}

JsVar *jswrap_wifi_getDetails(JsVar *jsCallback){
 return 0;
}

JsVar *jswrap_wifi_getAPDetails(JsVar *jsCallback){
 return 0;
}

JsVar *jswrap_wifi_getIP(JsVar *jsCallback){
 return 0;
}

JsVar *jswrap_wifi_getAPIP(JsVar *jsCallback){
 return 0;
}

JsVar *jswrap_wifi_getHostname(JsVar *jsCallback){
 return 0;
}

void   jswrap_wifi_setHostname(JsVar *jsHostname, JsVar *jsCallback){

}

void   jswrap_wifi_getHostByName(JsVar *jsHostname, JsVar *jsCallback){

}

void   jswrap_wifi_ping(JsVar *jsHostname, JsVar *jsCallback){

}

void   jswrap_wifi_setSNTP(JsVar *zone, JsVar *server){

}

void   jswrap_wifi_setIP(JsVar *jsSettings, JsVar *jsCallback){

}

void   jswrap_wifi_setAPIP(JsVar *jsSettings, JsVar *jsCallback){

}



void jswrap_wifi_init(){

}

/*JSON{
  "type":"init",
  "generate":"jswrap_wifi_soft_init"
}*/
void jswrap_wifi_soft_init(){
  JsNetwork net;
  networkCreate(&net, JSNETWORKTYPE_W600);
  networkState = NETWORKSTATE_ONLINE;
}


