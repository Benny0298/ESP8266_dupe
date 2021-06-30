/******************************************
 *
 * This example works for both Industrial and STEM users.
 * If you are using the old educational platform,
 * please consider to migrate your account to a STEM plan
 *
 * ****************************************/

/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsESPMQTT.h"

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "BBFF-jDX74fwNBKtjeL0NY1yXsLmavxBboY" // Your Ubidots TOKEN
#define WIFINAME "HUAWEI_P30" //Your SSID
#define WIFIPASS "zGmT!@.651926" // Your Wifi Pass
#define DEVICE_LABEL "my-new-device"

Ubidots ubidotsClient(TOKEN);

// random number
int randNum = 0;
// count of messages published
int count = 0;
// text buffer
char textBuffer[20] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // check for message from topic esp8266_led_switch
  if(strncmp(topic, "esp8266_led_switch", 18))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    if ((char)payload[0] == '1') {
      digitalWrite(2, LOW);
    } else {
      digitalWrite(2, HIGH);
    }
    Serial.println();
  }

  // check for message from topic web2board
  else if(strncmp(topic, "web2board", 9))
  {
    Serial.print("Message web2board ... ");
    for(int i = 0; i < length; i++)
      Serial.print((char)payload[i]);

    Serial.println();
  }  
}

/****************************************
 * Main Functions
 ****************************************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ubidotsClient.setDebug(false);  // Pass a true or false bool value to activate debug messages
  ubidotsClient.wifiConnection(WIFINAME, WIFIPASS);
  ubidotsClient.begin(callback);
  pinMode(2, OUTPUT);
  ubidotsClient.ubidotsSubscribe("my-new-device", "esp8266_led_switch");  // Insert the dataSource and Variable's Labels
  ubidotsClient.ubidotsSubscribe("my-new-device", "web2board");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!ubidotsClient.connected()) {
    ubidotsClient.reconnect();
    ubidotsClient.ubidotsSubscribe("my-new-device", "esp8266_led_switch");  // Insert the dataSource and Variable's Labels
    ubidotsClient.ubidotsSubscribe("my-new-device", "web2board");
  }
  // ubidotsClient.add("stuff", 10);
  // ubidotsClient.ubidotsPublish("source1");
  // ubidotsClient.ubidotsSubscribe("my-new-device", "esp8266_led_switch");  // Insert the dataSource and Variable's Labels

  // sprintf(textBuffer, "test%d\0", count);
  // sprintf(textBuffer, "test");
  // char * myContext = &textBuffer[0];
  // ubidotsClient.add("text2web", 20);
  // ubidotsClient.ubidotsPublish(DEVICE_LABEL);

  ubidotsClient.add("board2web", count);
  ubidotsClient.ubidotsPublish(DEVICE_LABEL);
  Serial.print("Message published [/v1.6/devices/my-new-device/board2web/lv] ");
  Serial.println(count);
  count = count + 1;
  
  ubidotsClient.loop();
  delay(5000);
}
