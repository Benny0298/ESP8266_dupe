#pragma once
#ifndef _myMQTT_h
#define _myMQTT_h

/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include <Arduino.h>
#include "UbidotsESPMQTT.h"
#include "myWiFi.h"

/****************************************
* * * * * * * D E F I N E S * * * * * * * 
****************************************/
// MQTT
#define MQTT_DEVICE_LABEL "esp_wireless_v1"
#define MQTT_CLIENT_NAME  "wireless_v1"
#define MQTT_BROKER       "industrial.api.ubidots.com"
#define MQTT_TOKEN        "BBFF-pMEcxGgqtxQZdNtsC1Oih3NXq7lI8K"

/****************************************
* * * * * P R O T O T Y P E S * * * * * * 
****************************************/
namespace myMQTT
{
  void mqttCallback(char* topic, byte* payload, unsigned int length);

  void mqttInit();
} // end of namespace

/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
#endif
