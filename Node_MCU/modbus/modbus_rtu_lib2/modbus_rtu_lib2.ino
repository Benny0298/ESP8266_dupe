/*

  Basic.pde - example using ModbusMaster library

  Library:: ModbusMaster
  Author:: Doc Walker <4-20ma@wvfans.net>

  Copyright:: 2009-2016 Doc Walker

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

*/

#include <ModbusMaster.h>
#include "UbidotsESPMQTT.h"
#include <SoftwareSerial.h>
#include <Ticker.h>

#define WIFI_SSID "HUAWEI_P30"
#define WIFI_PW   "zGmT!@.651926"

// MQTT
#define MQTT_DEVICE_LABEL "esp_wireless_v1"
#define MQTT_CLIENT_NAME  "wireless_v1"
#define MQTT_BROKER       "industrial.api.ubidots.com"
#define MQTT_TOKEN        "BBFF-pMEcxGgqtxQZdNtsC1Oih3NXq7lI8K"

#define MODBUS_SLAVE_ID   0x1
#define MODBUS_POS_REG    0x100
#define MODBUS_STATUS_REG 0x101

#define TIMER1_TICKS_HLF_S 156250
#define TIMER1_TICKS_ONE_S 312500
#define TIMER1_TICKS_TWO_S 625000

// instantiate ModbusMaster object
ModbusMaster node;
// ubidots
Ubidots ubidotsClient(MQTT_TOKEN);
// software serial for usb
SoftwareSerial usbSerial(3, 1);
// timer1
Ticker timer;

uint8_t result = 0;
uint16_t posReg = 0;
uint16_t statusReg = 0;
float pos = 0;

bool flagModbus = false;
bool flagDataChanged = false;


/*
 * ***************************************
 */
void ICACHE_RAM_ATTR onTime()
{
  flagModbus = true;
  timer1_write(TIMER1_TICKS_ONE_S); 
}
/*
 * ***************************************
 */
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  if(strncmp(topic, "led_switch", 10))
  {
    usbSerial.write("LED\n");
    if((char)payload[0] == '1')
    {
      digitalWrite(2, LOW);
    }
    else
    {
      digitalWrite(2, HIGH);
    }
  }
}

void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  delay(50);
  
  // use Serial (port 0); initialize Modbus communication baud rate
  Serial.begin(230400, SERIAL_8N1);
  Serial.swap();
  // communicate with Modbus slave ID 1 over Serial (port 0)
  node.begin(1, Serial);

  delay(50);

  usbSerial.begin(115200, SWSERIAL_8N1);

  delay(50);
  
  // mqtt
  ubidotsClient.setDebug(false);
  ubidotsClient.wifiConnection(WIFI_SSID, WIFI_PW);
  ubidotsClient.begin(mqttCallback);
  ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");

  delay(50);

  timer1_attachInterrupt(onTime);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(TIMER1_TICKS_ONE_S);
}


void loop()
{
  // reconnect in case of disconnect
  if(!ubidotsClient.connected())
  {
    ubidotsClient.reconnect();
    ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
  }

  if(flagModbus)
  {
    result = node.readHoldingRegisters(0x100, 1);
    if(result == node.ku8MBSuccess)
    {
      posReg = node.getResponseBuffer(0);
      char usbBuf[4] = {'\0', '\0', '\0', '\0'};
      sprintf(&usbBuf[0], "%04X", posReg);
      usbSerial.write("posHex: ");
      usbSerial.write(&usbBuf[0], sizeof(usbBuf));
      usbSerial.write("\n");
    }

    delay(5);
    
    result = node.readHoldingRegisters(0x101, 1);
    if(result == node.ku8MBSuccess)
    {
      statusReg = node.getResponseBuffer(0);
    }

    flagModbus = false;
    flagDataChanged = true;
  }
  
  // do something with data if read is successful
  if (flagDataChanged)
  {
    pos = (float)((posReg/1023.0)*100.0);
    char usbBuf[13] = "";
    sprintf(&usbBuf[0], "posFl:  %.1f", pos);
    usbSerial.write(&usbBuf[0], sizeof(usbBuf));
    usbSerial.write("\n");

    ubidotsClient.add("act_position", (float)pos);
    ubidotsClient.add("status_reg", statusReg);
    ubidotsClient.ubidotsPublish(MQTT_DEVICE_LABEL);

    flagDataChanged = false;
  }

  ubidotsClient.loop();
}
