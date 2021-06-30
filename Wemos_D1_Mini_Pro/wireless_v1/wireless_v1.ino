/****************************************
* * * * * * * U S A G E * * * * * * * * * 
****************************************/
/* 
* WiFi connection via Smartphone Hotspot:
* - SSID ... HUAWEI_P30
* - PW   ... zGmT!@.651926
* 
* config SoftwareSerial for D1 Mini Pro Modbus RTU usage:
* - Baud      ... 230400
* - Data      ... 8 Bit
* - Stop      ... 1 Bit
* - Parity    ... No
* - Slave Adr ... 0x01
* - 
* - D1 Mini Pro Rx ... GPIO 13 / D7 || actuator Tx ... blue   wire of actuator || logic analyzer red wire 
* - D1 Mini Pro Tx ... GPIO 15 / D8 || actuator Rx ... yellow wire of actuator || logic analyzer white wire
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
 * 
 * - execution speed seemingly suffered from splitting into different files
 * 
 * - poll modbus slave regularly BUT only published data IF it has changed
 * --- add means to confirm if data has changed
 * 
 * - make rate of polling variable
 * --- eg if data has not changed in a while re-arm timer1 with bigger value
 * --- eg if a change in data is detected re-arm timer1 with smaller value
 * 
 * - modbus communication may not run perfetly
 * --- looking at serial monitor every ~4th request yields time-out 0xE4
 * 
 * - see if namespace is really neccessary
 * --- update: namespaces have been commented out, code can be compiled but does not work
 * ------ investigate further?
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

/****************************************
* * * * * * * * I S R s * * * * * * * * * 
****************************************/

/****************************************
* * * * * S U B   R O U T I N E S * * * * 
****************************************/
 
/****************************************
* * * * * * * * S E T U P * * * * * * * * 
****************************************/
void setup() {
  // put your setup code here, to run once:
  
  // config hardware Serial for Modbus RTU
  // - 230400 baud, 8 data, 1 stop, no parity
  // - Rx ... D7
  // - Tx ... D8
  Serial.begin(230400, SERIAL_8N1);  
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
  timer1_attachInterrupt(myTimer::onTime);
  timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
  timer1_write(TIMER1_TICKS_ONE_S);
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
    myModbusRTU::modbusPollSlave(MODBUS_SLAVE_ID, MODBUS_POS_REG, &posReg, myModbusRTU::modbusWrite);

    // poll slave for status
    myModbusRTU::modbusPollSlave(MODBUS_SLAVE_ID, MODBUS_STATUS_REG, &statusReg, myModbusRTU::modbusWrite);

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

  // usbSerial.write(interrupts);
  // loop ubidotsClient
  ubidotsClient.loop();
}

/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
