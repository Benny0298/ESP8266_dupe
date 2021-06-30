/****************************************
* * * * * * * U S A G E * * * * * * * * * 
****************************************/
/* 
* WiFi connection via Smartphone Hotspot:
* - SSID ... HUAWEI_P30
* - PW   ... zGmT!@.651926
* 
* config SoftwareSerial for NodeMCU Modbus RTU usage:
* - Baud      ... 230400
* - Data      ... 8 Bit
* - Stop      ... 1 Bit
* - Parity    ... No
* - Slave Adr ... 0x01
* - 
* - NodeMCU Rx ... GPIO 13 / D7 || actuator Tx ... blue   wire of actuator || logic analyzer red wire 
* - NodeMCU Tx ... GPIO 15 / D8 || actuator Rx ... yellow wire of actuator || logic analyzer white wire
* - Connect Grounds of NodeMCU / Actuator / Logic Analyzer
* - Cannot have both Actuator and Logic Analyzer connected at the same time -> debug separately
*/

/****************************************
* * * * * * C H A N G E S * * * * * * * * 
****************************************/
/*
 * changes over wireless_v0.ino
 * 
 * - added basic timer1 functionality
 * --- made slavePolling/mqttPublish interrupt controlled
 * 
 * - bulk publishing slave data via mqtt
 * 
 * - re-structured project into seperate files
 */
/****************************************
* * * * * * * * T O D O s * * * * * * * * 
****************************************/
/*
 * - MQTT callback prints to usb !twice!
 */
/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include "myGlobals.h"
#include "myMQTT.h"
#include "myModbusRTU.h"
#include "myTimer.h"
#include "myWiFi.h"
#include "wireless_v1.h"

/****************************************
* * * * * * * D E F I N E S * * * * * * * 
****************************************/

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/

/****************************************
* * * * * * C A L L B A C K S * * * * * * 
****************************************/
//// MQTT
//void mqttCallback(char* topic, byte* payload, unsigned int length)
//{
//  // handle mqtt message here
//  // Serial.printf_P("mqtt message arrived in %s ... %s\n", topic, (char*)payload);
//
//  // check for mqtt topic
//  // for example a LED switch
//  if(strncmp(topic, "led_switch", 10))
//  {
//    if((char)payload[0] == '1')
//    {
//      digitalWrite(2, LOW);
//      usbSerial.write("LED_ON\n");
//    }
//    else
//    {
//      digitalWrite(2, HIGH);
//      usbSerial.write("LED_OFF\n");
//    }
//  }
//}

//Modbus RTU
bool modbusWrite(Modbus::ResultCode event, uint16_t transactionID, void* data)
{
  // check modbus write here
  if(event != Modbus::EX_SUCCESS)
  {
    eSource = mb.eventSource();  // return slave ID of transaction
    // Serial.printf_P("requestResult: 0x%02X | eventSource: 0x%02X\n", event, eSource);
    // usbSerial.write("requestRes: 0x");
    char usbBuf[16];
    sprintf(usbBuf, "requestRes:0x%02X\n", event);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    return false;
  }
  else if(event == Modbus::EX_SUCCESS)
  {
    pos = (float)((posReg/1023.0)*100.0);  // calc position [%] from returned reg value | range 0 ... 1023
    // Serial.printf_P("position:  %.1f%%\n", pos);
    // Serial.printf_P("statusReg: 0x%04X\n", statusReg);
    char usbBuf[10];
    sprintf(usbBuf, "pos:%.1f%%\n", pos); 
    usbSerial.write(usbBuf, sizeof(usbBuf));
    char usbBuf2[12];
    sprintf(usbBuf2, "stat:0x%04X\n", statusReg);
    usbSerial.write(usbBuf2, sizeof(usbBuf2));
  }
   
  return true;
}

/****************************************
* * * * * * * * * I S R s * * * * * * * * 
****************************************/
void ICACHE_RAM_ATTR onTime()
{
  interrupts++;

  // set flag to poll modbus slave
  flagModbus = true;

  // re-arm timer1 as using TIM_SINGLE
  // - TIM_DIV256 + 156250 ticks ... 0.5s
  // - TIM_DIV256 + 312500 ticks ... 1s
  // - TIM_DIV256 + 625000 ticks ... 2s
  timer1_write(625000); 
}

/****************************************
* * * * * S U B   R O U T I N E S * * * * 
****************************************/
void modbusPollSlave(uint8_t slaveID, uint16_t readAdr, uint16_t* resReg, cbTransaction cb)
{
  if(!mb.slave())
  {
    //mb.readHred(SLAVE_ID, REG_ADDRESS, VARIABLE_NAME, NUM_REGS, CALLBACK);
    mb.readHreg(MODBUS_SLAVE_ID, readAdr, resReg, 1, cb);

    // blocking loop to ensure transaction
    while(mb.slave())
    {
      mb.task();
      delay(10);
    } 
  }
}

 
/****************************************
* * * * * * * * S E T U P * * * * * * * * 
****************************************/
void setup() {
  // put your setup code here, to run once:
  
  // config hardware Serial for Modbus RTU
  // - Rx ... D7
  // - Tx ... D8
  Serial.begin(230400, SERIAL_8N1);  // 230400 baud, 8 data, 1 stop, no parity
  Serial.swap();
  mb.begin(&Serial);
  mb.master();

  delay(100);

  // config SoftwareSerial for debugging via USB
  // - also via Tx Pin
  // - Tx functionality only
  usbSerial.begin(115200, SWSERIAL_8N1);
  
  delay(100);

  myMQTT::mqttInit();

  delay(100);

  // config LED pin as output
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  delay(100);

  // config timer 
  // - TIM_DIV256 + 156250 ticks ... 0.5s
  // - TIM_DIV256 + 312500 ticks ... 1s
  // - TIM_DIV256 + 625000 ticks ... 2s
  // - config timer AFTER ubidots connection because it acts as a blocking mechanism preventing timer form firing
  timer1_attachInterrupt(onTime);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(625000);
}


/****************************************
* * * * * * M A I N   L O O P * * * * * * 
****************************************/
void loop() {
  // put your main code here, to run repeatedly:
  // reconnect in case of disconnect
  if(!ubidotsClient.connected())
  {
    ubidotsClient.reconnect();
    ubidotsClient.ubidotsSubscribe("esp_wireless_v1", "led_switch");
  }

  // check modbus poll flag
  if(flagModbus)
  {
    // poll slave for position
    modbusPollSlave(MODBUS_SLAVE_ID, MODBUS_POS_REG, &posReg, modbusWrite);

    // poll slave for status
    modbusPollSlave(MODBUS_SLAVE_ID, MODBUS_STATUS_REG, &statusReg, modbusWrite);

    // reset flag
    flagModbus = false;
    // set dataChanged flag
    dataChanged = true;
  }

  // check dataChanged flag
  if(dataChanged)
  {
    // add modbus data for publish
    ubidotsClient.add("act_position", (float)pos);
    ubidotsClient.add("status_reg", statusReg);
    
    // publish modbus data
    ubidotsClient.ubidotsPublish(MQTT_DEVICE_LABEL);
    // reset flag
    dataChanged = false;
  }

  // loop ubidotsClient
  ubidotsClient.loop();
}


/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
