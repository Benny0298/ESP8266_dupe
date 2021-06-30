#pragma once
#ifndef _myMQTT_h
#define _myMQTT_h

#include <Arduino.h>
#include "UbidotsESPMQTT.h"

// WiFi
#define WIFI_SSID "HUAWEI_P30"
#define WIFI_PW   "zGmT!@.651926"
// MQTT
#define MQTT_DEVICE_LABEL "esp_wireless_v1"
#define MQTT_CLIENT_NAME  "wireless_v1"
#define MQTT_BROKER       "industrial.api.ubidots.com"
#define MQTT_TOKEN        "BBFF-pMEcxGgqtxQZdNtsC1Oih3NXq7lI8K"



  
namespace myMQTT
{
  void mqttCallback(char* topic, byte* payload, unsigned int length);

  void mqttInit();
}
#endif
// EOF
