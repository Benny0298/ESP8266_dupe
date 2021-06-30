#include "modules_v1.h"
#include "myGlobals.h"
#include "myMQTT.h"




void setup() {
  // put your setup code here, to run once:
  
  
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  Serial.begin(115200);

//  // config ubidots MQTT client
//  ubidotsClient.setDebug(false);
//  ubidotsClient.wifiConnection(WIFI_SSID, WIFI_PW);
//  ubidotsClient.begin(myMQTT::mqttCallback);
//  ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
  myMQTT::mqttInit();
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  if(ledOut == true)
  {
    digitalWrite(2, LOW);
    ledOut = false;
  }
  else
  {
    digitalWrite(2, HIGH);
    ledOut = true;
  }
  */
  // reconnect in case of disconnect
  if(!ubidotsClient.connected())
  {
    ubidotsClient.reconnect();
    ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
  }

  delay(1000);
  count++;
  ubidotsClient.loop();
}
