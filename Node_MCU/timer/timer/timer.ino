/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
// WiFi
#include "UbidotsESPMQTT.h"
// Modbus RTU
#include <ModbusRTU.h>
// Timer
#include <Ticker.h>
// SoftwareSerial
#include <SoftwareSerial.h>

/**************************************
* * * * * * * D E F I N E S * * * * * * 
**************************************/
// WiFi
#define WIFI_SSID "HUAWEI_P30"
#define WIFI_PW   "zGmT!@.651926"
// MQTT
#define MQTT_DEVICE_LABEL "esp_wireless_v1"
#define MQTT_CLIENT_NAME  "wireless_v1"
#define MQTT_BROKER       "industrial.api.ubidots.com"
#define MQTT_TOKEN        "BBFF-pMEcxGgqtxQZdNtsC1Oih3NXq7lI8K"
// Modbus RTU
#define MODBUS_SLAVE_ID 0x01
#define MODBUS_POS_REG  0x100
#define MODBUS_STAT_REG 0x101

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
// MQTT
char mqttPayload[200];
char mqttTopic[70];
Ubidots ubidotsClient(MQTT_TOKEN);
// Modbus RTU
ModbusRTU mb;
// Timer
Ticker timer;
// Interrupts
volatile int interrupts = 0;
// other
int randNum = 0;
uint16_t posReg = 0;
uint16_t statReg = 0;
uint32_t eSource = 0;
float pos = 0;
bool flagRand = false;
bool flagModbus = false;
bool dataChanged = false;

/****************************************
* * * * * * C A L L B A C K S * * * * * * 
****************************************/
// MQTT
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
      // Serial.print("LED_ON\n");
    }
    else
    {
      digitalWrite(2, HIGH);
      // Serial.print("LED_OFF\n");
    }
  }
}


//Modbus RTU
bool modbusWrite(Modbus::ResultCode event, uint16_t transactionID, void* data)
{
  // check modbus write here
  if(event != Modbus::EX_SUCCESS)
  {
    eSource = mb.eventSource();  // return slave ID of transaction
    // Serial.printf_P("requestResult: 0x%02X | eventSource: 0x%02X\n", event, eSource);
    // usbSerial.write("requestRes: 0x");
    // char usbBuf[16];
    // sprintf(usbBuf, "requestRes:0x%02X\n", event);
    // usbSerial.write(usbBuf, sizeof(usbBuf));
    return false;
  }
  else if(event == Modbus::EX_SUCCESS)
  {
    pos = (float)((posReg/1023.0)*100.0);  // calc position [%] from returned reg value | range 0 ... 1023
    // Serial.printf_P("position:  %.1f%%\n", pos);
    // Serial.printf_P("statusReg: 0x%04X\n", statusReg);
    // char usbBuf[10];
    // sprintf(usbBuf, "pos:%.1f%%\n", pos); 
    // usbSerial.write(usbBuf, sizeof(usbBuf));
    // char usbBuf2[12];
    // sprintf(usbBuf2, "stat:0x%04X\n", statusReg);
    // usbSerial.write(usbBuf2, sizeof(usbBuf2));
  }
   
  return true;
}
/****************************************
* * * * * * * * * I S R s * * * * * * * * 
****************************************/
// ISR to Fire when Timer is triggered
void ICACHE_RAM_ATTR onTime()
{
  interrupts++;
  // Serial.print("timer1 isr");
  // Serial.println(interrupts);

  // set flag to publish randNum
  flagRand = true;

  // set flag to poll modbus slave
  flagModbus = true;
  
  // Re-Arm the timer as using TIM_SINGLE
  timer1_write(625000);//2s
}



/****************************************
* * * * * * * * S E T U P * * * * * * * * 
****************************************/
void setup()
{
  // LED
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  delay(100);

  // hardware serial
  // - used for modbus
  Serial.begin(230400, SERIAL_8N1);
  Serial.swap();
  mb.begin(&Serial);
  mb.master();

  delay(100);

  // config ubidots MQTT client
  ubidotsClient.setDebug(false);
  ubidotsClient.wifiConnection(WIFI_SSID, WIFI_PW);
  ubidotsClient.begin(mqttCallback);
  ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");

  delay(500);

  // config timer 
  // - TIM_DIV256 + 312500 ticks ... 1s
  // - config timer AFTER ubidots connection because it acts as a blocking mechanism preventing timer form firing
  timer1_attachInterrupt(onTime);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(625000);
}

/****************************************
* * * * * * M A I N   L O O P * * * * * * 
****************************************/
void loop()
{
  // reconnect upon connection loss
  if(!ubidotsClient.connected())
  {
    ubidotsClient.reconnect();
    ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
  }

  // check randNum flag
  if(flagRand)
  {
    randNum = random(1,10);
    ubidotsClient.add("randNum", randNum);
    // ubidotsClient.ubidotsPublish(MQTT_DEVICE_LABEL);

    // reset flag
    flagRand = false;
  }

  // check modbus flag
  if(flagModbus)
  {
    // read pos reg
    if(!mb.slave())
    {
      mb.readHreg(0x01, 0x100, &posReg, 0x01, modbusWrite);

      while(mb.slave())
      {
        mb.task();
        delay(10);
      }
    }

    // read stat reg
    if(!mb.slave())
    {
      mb.readHreg(0x01, 0x101, &statReg, 0x01, modbusWrite);

      while(mb.slave())
      {
        mb.task();
        delay(10);
      }
    }

    // set dataChanged flag
    dataChanged = true;
    // reset flag
    flagModbus = false;
  }


  // publish changed modbus slave data
  if(dataChanged == true)
  {
    // publihs pos
    ubidotsClient.add("act_position", (float)pos);
    // ubidotsClient.ubidotsPublish(MQTT_DEVICE_LABEL);
    // publish stat
    ubidotsClient.add("status_reg", statReg);
    ubidotsClient.ubidotsPublish(MQTT_DEVICE_LABEL);
    
    dataChanged = false;
  }

  ubidotsClient.loop();
}
