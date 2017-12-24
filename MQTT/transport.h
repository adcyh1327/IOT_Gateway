/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/
#include "Task_MQTT.h"

int transport_getdata(unsigned char* buf, int count);
int MQTT_Connect(u8 sn);
int mqtt_publish(u8 sn,struct MQTT_Topic_Info_t* pubmsg);
int mqtt_subscrib(u8 sn,struct MQTT_Topic_Info_t* submsg,u8 topic_num);
int8_t transport_close(u8 sn);
int mqtt_pingreq(uint8_t sn);
