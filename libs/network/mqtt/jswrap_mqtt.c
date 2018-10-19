/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2018 Lancer <502554248@qq.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Implementation of mqtt library
 * ----------------------------------------------------------------------------
 */


#include "jswrap_mqtt.h"
#include "jsvariterator.h"
#include "jsinteractive.h"
#include "jswrap_modules.h"
#include "jswrap_interactive.h"
#include "jswrap_object.h"
#include "network.h"
#include "socketerrors.h"


#define MQTT_CTRL_CONNECT           0x10
#define MQTT_CTRL_CONNACK           0x20
#define MQTT_CTRL_PUBLISH           0x30
#define MQTT_CTRL_PUBACK            0x40
#define MQTT_CTRL_PUBREC            0x50
#define MQTT_CTRL_PUBREL            0x60
#define MQTT_CTRL_PUBCOMP           0x70
#define MQTT_CTRL_SUBSCRIBE         0x80
#define MQTT_CTRL_SUBACK            0x90
#define MQTT_CTRL_UNSUBSCRIBE       0xA0
#define MQTT_CTRL_UNSUBACK          0xB0
#define MQTT_CTRL_PINGREQ           0xC0
#define MQTT_CTRL_PINGRESP          0xD0
#define MQTT_CTRL_DISCONNECT        0xE0


#define MQTT_OPT_FIELD              "opts"
#define MQTT_CKT_FIELD              JS_HIDDEN_CHAR_STR"m_sckt"

#define MQTT_EVT_CONNECTED          "connected"
#define MQTT_EVT_DISCONNECTED       "disconnected"
#define MQTT_EVT_PING               "ping"
#define MQTT_EVT_PUBLISH            "publish"
#define MQTT_EVT_ERROR              "error"


#define MQTT_CTRL_STA_CONNECTED     0x01
#define MQTT_CTRL_STA_HANDSHAKE     0x02

#define MQTT_CTRL_ENA_RECV          0x10
#define MQTT_CTRL_ENA_SEND          0x20
#define MQTT_CTRL_ENA_PING          0x40
#define MQTT_CTRL_ENA_IDLE          0x80

#define MQTT_MAX_PKG_SIZE           1533
#define MQTT_TMP_BUF_SIZE           64

#define MQTT_CNT_FLAGS_CLEAN_SESSION    0x02
#define MQTT_CNT_FLAGS_WILL_FLAG        0x04
#define MQTT_CNT_FLAGS_WILL_RETAIN      0x20
#define MQTT_CNT_FLAGS_PASSWORD         0x40
#define MQTT_CNT_FLAGS_USERNAME         0x80

#define MQTT_RETAIN_SIZE                MQTT_MAX_PKG_SIZE-mqtt_pkg_idx-2

static char         mqtt_ctrl;
static JsSysTime    mqtt_ping_interval;
static JsSysTime    mqtt_ping_next;
static uint16_t     mqtt_next_identifier;
static JsVar*       mqtt_send_data_buf;

bool _jswrap_mqtt_socket_recv(JsNetwork *net);
bool _jswrap_mqtt_socket_send(JsNetwork *net);
void _jswrap_mqtt_ping_request(JsNetwork *net);


/*JSON{
    "type":"library",
    "class":"mqtt"
}*/

/*JSON{
    "type":"init",
    "generate":"jswrap_mqtt_init"
}*/

void jswrap_mqtt_init(){
    mqtt_ctrl=0;
    mqtt_ping_interval=0;
    mqtt_ping_next=0;
    mqtt_next_identifier=0;
    mqtt_send_data_buf=jsvNewFromEmptyString();
}

/*JSON{
    "type":"idle",
    "generate":"jswrap_mqtt_idle"
}*/

bool jswrap_mqtt_idle(){
    JsNetwork net;
    bool is_busy=false;
    if(mqtt_ctrl&MQTT_CTRL_ENA_IDLE){
        if(networkGetFromVarIfOnline(&net)){
            if(mqtt_ctrl&MQTT_CTRL_ENA_RECV){
                if(_jswrap_mqtt_socket_recv(&net)){
                    is_busy=true;
                }
            }
            if(mqtt_ctrl&MQTT_CTRL_ENA_PING){
                _jswrap_mqtt_ping_request(&net);
            }
            if(mqtt_ctrl&MQTT_CTRL_ENA_SEND){
                if(_jswrap_mqtt_socket_send(&net)){
                    is_busy=true;
                }
            }
            networkFree(&net);
        }
    }
    return is_busy!=0;
}

/*JSON{
    "type":"kill",
    "generate":"jswrap_mqtt_kill"
}*/

void jswrap_mqtt_kill(){
    jswrap_mqtt_disconnect();
}

uint16_t _jswrap_mqtt_get_identifier(){
    mqtt_next_identifier++;
    return mqtt_next_identifier;
}

JsVar *_jswrap_mqtt_module(){
    JsVar *mName=jsvNewFromString("mqtt");
    JsVar *m=jswrap_require(mName);
    jsvUnLock(mName);
    return m;
}

void _jswrap_mqtt_socket_close(JsNetwork *net,JsVar *module,int sckt){
    mqtt_ctrl=0;
    jsvUnLock(mqtt_send_data_buf);
    mqtt_send_data_buf=jsvNewFromEmptyString();

    if(sckt>=0){
        netCloseSocket(net,ST_NORMAL,sckt);
        jsvObjectSetChildAndUnLock(module,MQTT_CKT_FIELD,jsvNewFromInteger(0));
    }

    JsVar *mqtt_evt=jsvNewFromString(MQTT_EVT_DISCONNECTED);
    JsVar *mqtt_arg=jsvNewEmptyArray();
    jswrap_object_emit(module,mqtt_evt,mqtt_arg);
    jsvUnLock2(mqtt_evt,mqtt_arg);
}

bool _jswrap_mqtt_socket_send(JsNetwork *net){
    bool hasEvents=false;
    char *buf=alloca(net->chunkSize);
    JsVar *module=_jswrap_mqtt_module();
    int sckt=jsvGetIntegerAndUnLock(jsvObjectGetChild(module,MQTT_CKT_FIELD,0))-1;
    if(!jsvIsEmptyString(mqtt_send_data_buf)){
        hasEvents=true;
        int mqtt_tmp_len=jsvGetString(mqtt_send_data_buf,buf,net->chunkSize);
        int send_num=netSend(net,ST_NORMAL,sckt,buf,mqtt_tmp_len);

        if(send_num<0){
            _jswrap_mqtt_socket_close(net,module,sckt);
            hasEvents=false;
        }else if(send_num>0){
            JsVar *new_send_buf=0;
            if(send_num<(int)jsvGetStringLength(mqtt_send_data_buf)){
                new_send_buf=jsvNewFromStringVar(mqtt_send_data_buf,(size_t)send_num,JSVAPPENDSTRINGVAR_MAXLENGTH);
            }else{
                new_send_buf=jsvNewFromEmptyString();
            }
            jsvUnLock(mqtt_send_data_buf);
            mqtt_send_data_buf=new_send_buf;
        }
    }
    jsvUnLock(module);
    return hasEvents;
}

void _jswrap_mqtt_ping_request(JsNetwork *net){
    if(mqtt_ping_next==0){
        mqtt_ping_next=jshGetSystemTime();
    }else {
        JsSysTime cur_time=jshGetSystemTime();
        if(cur_time>mqtt_ping_next){
            mqtt_ping_next=cur_time+mqtt_ping_interval;

            JsVar *module=_jswrap_mqtt_module();
            //int sckt=jsvGetIntegerAndUnLock(jsvObjectGetChild(module,MQTT_CKT_FIELD,0))-1;

            JsVar *mqtt_evt=jsvNewFromString(MQTT_EVT_PING);
            JsVar *mqtt_arg=jsvNewEmptyArray();
            jsvArrayPushAndUnLock(mqtt_arg,jsvNewFromBool(false));

            jswrap_object_emit(module,mqtt_evt,mqtt_arg);
            jsvUnLock3(module,mqtt_evt,mqtt_arg);

            char mqtt_pkg_ptr[2];
            mqtt_pkg_ptr[0]=MQTT_CTRL_PINGREQ;
            mqtt_pkg_ptr[1]=0;
            jsvAppendStringBuf(mqtt_send_data_buf,mqtt_pkg_ptr,2);
        }
    }
}

void _jswrap_mqtt_ping_response(JsVar *module){
    JsVar *mqtt_evt=jsvNewFromString(MQTT_EVT_PING);
    JsVar *mqtt_arg=jsvNewEmptyArray();
    jsvArrayPushAndUnLock(mqtt_arg,jsvNewFromBool(true));

    jswrap_object_emit(module,mqtt_evt,mqtt_arg);
    jsvUnLock2(mqtt_evt,mqtt_arg);
}

void _jswrap_mqtt_connect(JsVar *module);
void _jswrap_mqtt_connack(JsVar *module,char *buf);
void _jswrap_mqtt_publish(JsVar *module,char *mqtt_pkg_ptr,int mqtt_tmp_len);

bool _jswrap_mqtt_socket_recv(JsNetwork *net){
    JsVar *module=_jswrap_mqtt_module();
    int sckt=jsvGetIntegerAndUnLock(jsvObjectGetChild(module,MQTT_CKT_FIELD,0))-1;

    char *buf=alloca(net->chunkSize);
    int recv_num=netRecv(net,ST_NORMAL,sckt,buf,net->chunkSize);
    
    if(recv_num<0){
        if(recv_num!=SOCKET_ERR_NO_CONN){
            _jswrap_mqtt_socket_close(net,module,sckt);
        }
    }else{
        if((mqtt_ctrl&MQTT_CTRL_STA_HANDSHAKE)==0){
            _jswrap_mqtt_connect(module);
            mqtt_ctrl|=MQTT_CTRL_STA_HANDSHAKE;
        }

        if(recv_num>0){
            switch((buf[0]&0xF0)){
                case MQTT_CTRL_CONNACK:
                    _jswrap_mqtt_connack(module,buf);
                break;
                case MQTT_CTRL_PINGRESP:
                    _jswrap_mqtt_ping_response(module);
                break;
                case MQTT_CTRL_PUBLISH:
                    _jswrap_mqtt_publish(module,buf,recv_num);
                break;
                case MQTT_CTRL_PUBREL:
                {
                    buf[0]=MQTT_CTRL_PUBCOMP;
                    jsvAppendStringBuf(mqtt_send_data_buf,buf,4);
                }   
                break;
                case MQTT_CTRL_PUBREC:
                {
                    buf[0]=MQTT_CTRL_PUBREL+2;
                    jsvAppendStringBuf(mqtt_send_data_buf,buf,4);
                }
                break;
                default:
                break;
            }
            
            jsvUnLock(module);
            return true;
        }
    }

    jsvUnLock(module);
    return false;
}

void _jswrap_mqtt_connect(JsVar *module){
    
    int mqtt_pkg_total_len=10;
    char connect_flags=0;
    JsVar *options_var=jsvObjectGetChild(module,MQTT_OPT_FIELD,0);

    // client id
    JsVar *client_id_var=jsvObjectGetChild(options_var,"clientid",0);
    if(jsvIsString(client_id_var)){
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(client_id_var);
    }

    // will topic and will message
    JsVar *will_obj_var=jsvObjectGetChild(options_var,"will",0);
    JsVar *will_topic_var=0,*will_msg_var=0;
    if(jsvIsObject(will_obj_var)){
        
        connect_flags|=MQTT_CNT_FLAGS_WILL_FLAG;
        will_topic_var=jsvObjectGetChild(will_obj_var,"topic",0);
        will_msg_var=jsvObjectGetChild(will_obj_var,"message",0);
        
        JsVar *will_qos_var=jsvObjectGetChild(will_obj_var,"qos",0);
        connect_flags|=((jsvGetInteger(will_qos_var)&0x03)<<3);
        jsvUnLock(will_qos_var);

        JsVar *will_retain_var=jsvObjectGetChild(will_obj_var,"retain",0);
        if(jsvGetBool(will_retain_var)){
            connect_flags|=MQTT_CNT_FLAGS_WILL_RETAIN;
        }
        jsvUnLock(will_retain_var);

    }
    jsvUnLock(will_obj_var);

    if(jsvIsString(will_topic_var)){
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(will_topic_var);
    }

    if(jsvIsString(will_msg_var)){
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(will_msg_var);
    }

    //username
    JsVar *username_var=jsvObjectGetChild(options_var,"username",0);
    if(jsvIsString(username_var)){
        connect_flags|=MQTT_CNT_FLAGS_USERNAME;
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(username_var);
    }

    //password
    JsVar *password_var=jsvObjectGetChild(options_var,"password",0);
    if(jsvIsString(password_var)){
        connect_flags|=MQTT_CNT_FLAGS_PASSWORD;
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(password_var);
    }

    //clean session
    JsVar *clean_session_var=jsvObjectGetChild(options_var,"cleansession",0);
    if(jsvGetBool(clean_session_var)){
        connect_flags|=MQTT_CNT_FLAGS_CLEAN_SESSION;
    }
    jsvUnLock(clean_session_var);

    if(mqtt_pkg_total_len<MQTT_MAX_PKG_SIZE){

        char mqtt_pkg_ptr[MQTT_MAX_PKG_SIZE];
        int mqtt_pkg_idx=0,mqtt_tmp_len=0;

        mqtt_pkg_ptr[mqtt_pkg_idx++]=MQTT_CTRL_CONNECT;

        if(mqtt_pkg_total_len<128){
            mqtt_pkg_ptr[mqtt_pkg_idx++]=mqtt_pkg_total_len;
        }else{
            mqtt_pkg_ptr[mqtt_pkg_idx++]=(mqtt_pkg_total_len&0x7F)|0x80;
            mqtt_pkg_ptr[mqtt_pkg_idx++]=(mqtt_pkg_total_len>>7)&0x7F;
        }

        mqtt_pkg_ptr[mqtt_pkg_idx++]=0;
        mqtt_pkg_ptr[mqtt_pkg_idx++]=4;
        mqtt_pkg_ptr[mqtt_pkg_idx++]='M';
        mqtt_pkg_ptr[mqtt_pkg_idx++]='Q';
        mqtt_pkg_ptr[mqtt_pkg_idx++]='T';
        mqtt_pkg_ptr[mqtt_pkg_idx++]='T';

        mqtt_pkg_ptr[mqtt_pkg_idx++]=4;

        mqtt_pkg_ptr[mqtt_pkg_idx++]=connect_flags;

        JsVar *keep_alive_var=jsvObjectGetChild(options_var,"keepalive",0);
        uint16_t keep_alive=(uint16_t)jsvGetIntegerAndUnLock(keep_alive_var);
        if(keep_alive<20){
            keep_alive=20;
        }
        mqtt_ping_interval=jshGetTimeFromMilliseconds((keep_alive*2000)/3);
        mqtt_pkg_ptr[mqtt_pkg_idx++]=((keep_alive>>8)&0xFF);
        mqtt_pkg_ptr[mqtt_pkg_idx++]=((keep_alive>>0)&0xFF);

        mqtt_tmp_len=jsvGetString(client_id_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
        mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xFF);
        mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xFF);
        mqtt_pkg_idx+=mqtt_tmp_len;

        if(jsvIsString(will_topic_var)){
            mqtt_tmp_len=jsvGetString(will_topic_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xFF);
            mqtt_pkg_idx+=mqtt_tmp_len;
        }
        
        if(jsvIsString(will_msg_var)){
            mqtt_tmp_len=jsvGetString(will_msg_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xFF);
            mqtt_pkg_idx+=mqtt_tmp_len;
        }

        if(jsvIsString(username_var)){
            mqtt_tmp_len=jsvGetString(username_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xFF);
            mqtt_pkg_idx+=mqtt_tmp_len;
        }

        if(jsvIsString(password_var)){
            mqtt_tmp_len=jsvGetString(password_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xFF);
            mqtt_pkg_idx+=mqtt_tmp_len;
        }

        jsvAppendStringBuf(mqtt_send_data_buf,mqtt_pkg_ptr,mqtt_pkg_idx);
    }

    jsvUnLock3(password_var,username_var,will_msg_var);
    jsvUnLock3(will_topic_var,client_id_var,options_var);

}

void _jswrap_mqtt_connack(JsVar *module,char *buf){
    JsVar *mqtt_evt=0,*mqtt_arg=jsvNewEmptyArray();
    if(buf[3]==0){
        mqtt_ping_next=0;
        mqtt_ctrl|=MQTT_CTRL_ENA_PING;
        mqtt_ctrl|=MQTT_CTRL_STA_CONNECTED;
        mqtt_evt=jsvNewFromString(MQTT_EVT_CONNECTED);
    }else{
        mqtt_evt=jsvNewFromString(MQTT_EVT_ERROR);
        jsvArrayPushAndUnLock(mqtt_arg,jsvNewFromString("connack"));
        jsvArrayPushAndUnLock(mqtt_arg,jsvNewFromInteger(buf[3]));
    }

    jswrap_object_emit(module,mqtt_evt,mqtt_arg);
    jsvUnLock2(mqtt_evt,mqtt_arg);
}

void _jswrap_mqtt_publish(JsVar *module,char *mqtt_pkg_ptr,int mqtt_tmp_len){
    JsVar *mqtt_evt=jsvNewFromString(MQTT_EVT_PUBLISH);
    JsVar *mqtt_arg=jsvNewEmptyArray();

    int mqtt_pkg_idx=1,mqtt_pkg_total_len=0;

    if((mqtt_pkg_ptr[mqtt_pkg_idx]&0x80)){
        mqtt_pkg_total_len|=((mqtt_pkg_ptr[mqtt_pkg_idx++]&0x7F)<<0);
        mqtt_pkg_total_len|=((mqtt_pkg_ptr[mqtt_pkg_idx++]&0x7F)<<7);
    }else{
        mqtt_pkg_total_len=mqtt_pkg_ptr[mqtt_pkg_idx++];
    }

    if(mqtt_pkg_total_len<=mqtt_tmp_len){
        int dup=(mqtt_pkg_ptr[0]&0x08)>>3;
        int qos=(mqtt_pkg_ptr[0]&0x06)>>1;
        int retain=(mqtt_pkg_ptr[0]&0x01);
        int identifier=0;

        mqtt_tmp_len=(mqtt_pkg_ptr[mqtt_pkg_idx++]<<8);
        mqtt_tmp_len|=(mqtt_pkg_ptr[mqtt_pkg_idx++]<<0);
        JsVar *topic_var=jsvNewFromEmptyString();
        jsvAppendStringBuf(topic_var,&mqtt_pkg_ptr[mqtt_pkg_idx],mqtt_tmp_len);
        jsvArrayPushAndUnLock(mqtt_arg,topic_var);
        mqtt_pkg_idx+=mqtt_tmp_len;

        //remain
        mqtt_pkg_total_len-=2;
        mqtt_pkg_total_len-=mqtt_tmp_len;

        if(qos!=0){
            identifier|=(mqtt_pkg_ptr[mqtt_pkg_idx++]<<8);
            identifier|=(mqtt_pkg_ptr[mqtt_pkg_idx++]<<0);
            mqtt_pkg_total_len-=2;
        }

        JsVar *message_var=jsvNewFromEmptyString();
        jsvAppendStringBuf(message_var,&mqtt_pkg_ptr[mqtt_pkg_idx],mqtt_pkg_total_len);
        jsvArrayPushAndUnLock(mqtt_arg,message_var);

        JsVar *options_var=jsvNewObject();
        jsvObjectSetChildAndUnLock(options_var,"qos",jsvNewFromInteger(qos));
        jsvObjectSetChildAndUnLock(options_var,"dup",jsvNewFromBool(dup!=0));
        jsvObjectSetChildAndUnLock(options_var,"retain",jsvNewFromBool(retain!=0));
        if(qos!=0){
            jsvObjectSetChildAndUnLock(options_var,"identifier",jsvNewFromInteger(identifier));
        }
        jsvArrayPushAndUnLock(mqtt_arg,options_var);


        jswrap_object_emit(module,mqtt_evt,mqtt_arg);

        if(qos!=0){
            char pkg_ptr[4];
            if(qos==1){
                pkg_ptr[0]=MQTT_CTRL_PUBACK;
            }else{
                pkg_ptr[0]=MQTT_CTRL_PUBREC;
            }
            pkg_ptr[1]=2;
            pkg_ptr[2]=((identifier>>8)&0xFF);
            pkg_ptr[3]=((identifier>>0)&0xFF);
            jsvAppendStringBuf(mqtt_send_data_buf,pkg_ptr,4);
        }

    }

    jsvUnLock2(mqtt_evt,mqtt_arg);
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "setup",
  "generate" : "jswrap_mqtt_setup",
  "params" : [
    ["options","JsVar","mqtt options"]
  ]
}
*/
void jswrap_mqtt_setup(JsVar *options){
    if(jsvIsObject(options)){
        JsVar *module=_jswrap_mqtt_module();
        jsvObjectSetChild(module,MQTT_OPT_FIELD,options);
        jsvUnLock(module);
    }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "connect",
  "generate" : "jswrap_mqtt_connect"
}
*/
void jswrap_mqtt_connect(){
    if(!(mqtt_ctrl&MQTT_CTRL_STA_CONNECTED)){
        JsNetwork net;
        if(networkGetFromVarIfOnline(&net)){
            JsVar *module=_jswrap_mqtt_module();
            
            JsVar *options_var=jsvObjectGetChild(module,MQTT_OPT_FIELD,0);
            JsVar *host_var=jsvObjectGetChild(options_var,"host",0);
            JsVar *port_var=jsvObjectGetChild(options_var,"port",0);

            char host_str[128];
            jsvGetString(host_var,host_str,sizeof(host_str));
            int port=jsvGetInteger(port_var);
            jsvUnLock2(host_var,port_var);

            uint32_t host_addr=0;
            networkGetHostByName(&net,host_str,&host_addr);

            if(!host_addr){
                jsiConsolePrintf("Unable to locate mqtt serve host\n");
                jsvUnLock2(module,options_var);
                networkFree(&net);
                return;
            }

            int sckt=netCreateSocket(&net,ST_NORMAL,host_addr,port,0)+1;
            if(sckt<=0){
                jsiConsolePrintf("Unable to create socket\n");
                jsvUnLock2(module,options_var);
                networkFree(&net);
                return;
            }

            jsvObjectSetChildAndUnLock(module,MQTT_CKT_FIELD,jsvNewFromInteger(sckt));

            mqtt_ctrl|=MQTT_CTRL_ENA_RECV;
            mqtt_ctrl|=MQTT_CTRL_ENA_SEND;
            mqtt_ctrl|=MQTT_CTRL_ENA_IDLE;

            jsvUnLock2(module,options_var);
            networkFree(&net);

        }
    }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "disconnect",
  "generate" : "jswrap_mqtt_disconnect"
}
*/
void jswrap_mqtt_disconnect(){
    if(mqtt_ctrl&MQTT_CTRL_STA_CONNECTED){
        JsNetwork net;
        if(networkGetFromVarIfOnline(&net)){
            JsVar *module=_jswrap_mqtt_module();
            int sckt=jsvGetIntegerAndUnLock(jsvObjectGetChild(module,MQTT_CKT_FIELD,0))-1;

            char mqtt_pkg_ptr[2];
            mqtt_pkg_ptr[0]=MQTT_CTRL_DISCONNECT;
            mqtt_pkg_ptr[1]=0;

            netSend(&net,ST_NORMAL,sckt,mqtt_pkg_ptr,2);

            _jswrap_mqtt_socket_close(&net,module,sckt);

            networkFree(&net);
        }
    }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "subscribe",
  "generate" : "jswrap_mqtt_subscribe",
  "params" : [
    ["topic_var","JsVar","mqtt topic"],
    ["qos_var","JsVar","mqtt qos"]
  ]
}
*/
void jswrap_mqtt_subscribe(JsVar *topic_var,JsVar *qos_var){
    if((mqtt_ctrl&MQTT_CTRL_STA_CONNECTED)&&jsvIsString(topic_var)){
        
        char mqtt_pkg_ptr[MQTT_MAX_PKG_SIZE];
        int mqtt_pkg_idx=0,mqtt_tmp_len=0,mqtt_pkg_total_len=0;

        int qos=jsvGetInteger(qos_var)&0x03;
        mqtt_pkg_total_len=2;
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(topic_var);
        mqtt_pkg_total_len+=1;

        if(mqtt_pkg_total_len<MQTT_MAX_PKG_SIZE){
            mqtt_pkg_ptr[mqtt_pkg_idx++]=MQTT_CTRL_SUBSCRIBE+2;
            
            if(mqtt_pkg_total_len<128){
                mqtt_pkg_ptr[mqtt_pkg_idx++]=mqtt_pkg_total_len;
            }else{
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>0)&0x7F)|0x80;
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>7)&0x7F);
            }

            uint16_t identifier=_jswrap_mqtt_get_identifier();
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>0)&0xFF);

            mqtt_tmp_len=jsvGetString(topic_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);

            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xff);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xff);
            mqtt_pkg_idx+=mqtt_tmp_len;

            mqtt_pkg_ptr[mqtt_pkg_idx++]=qos;

            jsvAppendStringBuf(mqtt_send_data_buf,mqtt_pkg_ptr,mqtt_pkg_idx);
        }

    }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "unsubscribe",
  "generate" : "jswrap_mqtt_unsubscribe",
  "params" : [
    ["topic_var","JsVar","mqtt topic"]
  ]
}
*/
void jswrap_mqtt_unsubscribe(JsVar *topic_var){
    if((mqtt_ctrl&MQTT_CTRL_STA_CONNECTED)&&jsvIsString(topic_var)){
        
        char mqtt_pkg_ptr[MQTT_MAX_PKG_SIZE];
        int mqtt_pkg_idx=0,mqtt_tmp_len=0,mqtt_pkg_total_len=0;

        mqtt_pkg_total_len=2;
        mqtt_pkg_total_len+=2;
        mqtt_pkg_total_len+=jsvGetStringLength(topic_var);

        if(mqtt_pkg_total_len<MQTT_MAX_PKG_SIZE){
            mqtt_pkg_ptr[mqtt_pkg_idx++]=MQTT_CTRL_UNSUBSCRIBE+2;
            
            if(mqtt_pkg_total_len<128){
                mqtt_pkg_ptr[mqtt_pkg_idx++]=mqtt_pkg_total_len;
            }else{
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>0)&0x7F)|0x80;
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>7)&0x7F);
            }

            uint16_t identifier=_jswrap_mqtt_get_identifier();
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>8)&0xFF);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>0)&0xFF);

            mqtt_tmp_len=jsvGetString(topic_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);

            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xff);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xff);
            mqtt_pkg_idx+=mqtt_tmp_len;

            jsvAppendStringBuf(mqtt_send_data_buf,mqtt_pkg_ptr,mqtt_pkg_idx);
        }

    }
}

/*JSON{
  "type" : "staticmethod",
  "class" : "mqtt",
  "name" : "publish",
  "generate" : "jswrap_mqtt_publish",
  "params" : [
    ["topic_var","JsVar","mqtt topic"],
    ["message_var","JsVar","mqtt message"],
    ["options","JsVar","mqtt options"]
  ]
}
*/
void jswrap_mqtt_publish(JsVar *topic_var,JsVar *message_var,JsVar *options){
    if((mqtt_ctrl&MQTT_CTRL_STA_CONNECTED)&&jsvIsString(topic_var)&&jsvIsString(message_var)){
        
        char mqtt_pkg_ptr[MQTT_MAX_PKG_SIZE];
        int mqtt_pkg_idx=1,mqtt_tmp_len=0,mqtt_pkg_total_len=0;
        int qos=0;
        bool retain=false;

        if(jsvIsObject(options)){
            qos=jsvGetIntegerAndUnLock(jsvObjectGetChild(options,"qos",0))&0x03;
            //dup=jsvGetBoolAndUnLock(jsvObjectGetChild(options,"dup",0));
            retain=jsvGetBoolAndUnLock(jsvObjectGetChild(options,"retain",0));
        }

        mqtt_pkg_total_len=2;
        mqtt_pkg_total_len+=jsvGetStringLength(topic_var);

        if(qos!=0){
            mqtt_pkg_total_len+=2;
        }

        mqtt_pkg_total_len+=jsvGetStringLength(message_var);

        if(mqtt_pkg_total_len<MQTT_MAX_PKG_SIZE){

            mqtt_pkg_ptr[0]=MQTT_CTRL_PUBLISH;

            if(retain){
                mqtt_pkg_ptr[0]|=0x01;
            }
            if(qos!=0){
                mqtt_pkg_ptr[0]|=(qos<<1);
            }

            
            if(mqtt_pkg_total_len<128){
                mqtt_pkg_ptr[mqtt_pkg_idx++]=mqtt_pkg_total_len;
            }else{
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>0)&0x7F)|0x80;
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_pkg_total_len>>7)&0x7F);
            }

            mqtt_tmp_len=jsvGetString(topic_var,&mqtt_pkg_ptr[mqtt_pkg_idx+2],MQTT_RETAIN_SIZE);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>8)&0xff);
            mqtt_pkg_ptr[mqtt_pkg_idx++]=((mqtt_tmp_len>>0)&0xff);
            mqtt_pkg_idx+=mqtt_tmp_len;

            if(qos!=0){
                uint16_t identifier=_jswrap_mqtt_get_identifier();
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>8)&0xFF);
                mqtt_pkg_ptr[mqtt_pkg_idx++]=((identifier>>0)&0xFF);
            }

            mqtt_tmp_len=jsvGetString(message_var,&mqtt_pkg_ptr[mqtt_pkg_idx],MQTT_MAX_PKG_SIZE-mqtt_pkg_idx);
            mqtt_pkg_idx+=mqtt_tmp_len;

            jsvAppendStringBuf(mqtt_send_data_buf,mqtt_pkg_ptr,mqtt_pkg_idx);
        }

    }
}