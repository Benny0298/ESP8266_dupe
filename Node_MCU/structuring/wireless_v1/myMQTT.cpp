/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include "myMQTT.h"
#include "myWiFi.h"

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
Ubidots ubidotsClient(MQTT_TOKEN);

/****************************************
* * * * * * F U N C T I O N S * * * * * * 
****************************************/
namespace myMQTT
{
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  // Serial.println("test");
  // handle mqtt message here
  // Serial.printf_P("mqtt message arrived in %s ... %s\n", topic, (char*)payload);

  // check for mqtt topic
  // for example a LED switch
  if(strncmp(topic, "led_switch", 10))
  {
    if((char)payload[0] == '1')
    {
      digitalWrite(2, LOW);
      // usbSerial.write("LED_ON\n");
    }
    else
    {
      digitalWrite(2, HIGH);
      // usbSerial.write("LED_OFF\n");
    }
  }
}

void mqttInit()
{
  ubidotsClient.setDebug(false);
  ubidotsClient.wifiConnection(WIFI_SSID, WIFI_PW);
  ubidotsClient.begin(myMQTT::mqttCallback);
  ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
}
} // enf of namespace

/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
// EOF
