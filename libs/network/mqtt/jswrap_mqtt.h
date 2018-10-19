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

#ifndef _JSWRAP_MQTT_H_
#define _JSWRAP_MQTT_H_

#include "jsvar.h"

void jswrap_mqtt_init();
bool jswrap_mqtt_idle();
void jswrap_mqtt_kill();


void jswrap_mqtt_setup(JsVar *options);
void jswrap_mqtt_connect();
void jswrap_mqtt_disconnect();

void jswrap_mqtt_subscribe(JsVar *topic_var,JsVar *qos_var);
void jswrap_mqtt_unsubscribe(JsVar *topic_var);
void jswrap_mqtt_publish(JsVar *topic_var,JsVar *message_var,JsVar *options);


#endif