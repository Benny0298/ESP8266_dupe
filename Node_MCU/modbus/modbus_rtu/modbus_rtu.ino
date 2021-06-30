/*
  ModbusRTU ESP8266/ESP32
  Read multiple coils from slave device example
  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
  modified 13 May 2020
  by brainelectronics
  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

// * * * U S A G E * * *
// config softwareSerial for NodeMCU for Actuator Modbus Communication
// NodeMCU Rx ... Pin13 / D7 || actuator Tx ... blue   wire of actuator || logic analyzer ... red   wire
// NodeMCU Tx ... Pin15 / D8 || actuator Rx ... yellow wire of actuator || logic analyzer ... white wire
// 
// Baud   ... 230400
// Data   ... 8 Bit
// Stop   ... 1 Bit
// Parity ... No

// * * * I N C L U D E S * * *
#include <ModbusRTU.h>
#if defined(ESP8266)
 #include <SoftwareSerial.h>
 
 SoftwareSerial S(13, 15);
#endif

// * * * D E F I N E S * * *
#define SLAVE_ID  0x1
#define FIRST_REG 1000
#define REG_COUNT 1

// * * * G L O B A L S * * *
ModbusRTU mb;                // modbus object
uint32_t eSource = 0;        // track event source
uint16_t busType = 0;
uint16_t displayFirmware = 0;
uint16_t id = 0;
uint16_t pos = 0;
uint16_t statusReg = 0;
uint16_t res = 0;

int count = 0;

uint16_t resReg[REG_COUNT];  // array of result registers
bool coils[20];              // for original code example


// * * * callback for master-write to monitor possible errors * * *
bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void* data)
{
  if(event != Modbus::EX_SUCCESS)
  {
    eSource = mb.eventSource();  // return slave ID of transaction
    Serial.printf_P("requestResult: 0x%02X | eventSource: 0x%02X\n", event, eSource);
    // Serial.printf_P("transactionID: 0x%02X\n", transactionId);
  }
  else if(event == Modbus::EX_SUCCESS)
  {
    // // print on success
    // Serial.printf_P("id: %d\n", id);
    // Serial.printf_P("busType: %d\n", busType);
    Serial.printf_P("position: %.1f%%\n", (float)((pos/1024.0)*100));  // calculate position [%] from returned register value | range 0 ... 1023
    // Serial.printf_P("statusReg: 0x%04X\n", statusReg);
  }

  return true;
}


// * * * setup * * *
void setup() {
  Serial.begin(115200);
 #if defined(ESP8266)
  S.begin(230400, SWSERIAL_8N1);  // 230400 baud; 8 data; 1 stop; no parity
  mb.begin(&S);                   // create modbus object from softwareSerial stream
 #elif defined(ESP32)
  Serial1.begin(230400, SERIAL_8N1);
  mb.begin(&Serial1);
 #else
  Serial1.begin(230400, SERIAL_8N1);
  mb.begin(&Serial1);
  mb.setBaudrate(230400);
 #endif
  mb.master();  // set master functionality

  // clear resReg array
  for(int i = 0; i < REG_COUNT; i++)
    resReg[i] = 0;
}


// * * * M A I N   L O O P * * * 
void loop()
{
  /*
  // * * * original example code * * *
  if (!mb.slave()) {
    mb.readCoil(1, 1, coils, 20, cbWrite);
  }
  mb.task();
  yield();
  // * * * original example code * * *
  */


  // // * * * my example * * *
  // // worked to read register 256 for 'actual position' ... value 761 in a range of 1024 amounts to ~74.3% -> concurrent with info on display
  // // task delay(10)
  // // end of loop delay(200)
  // // TODO make communication more stable -> a lot of 0xE4 time-out errors occur (custom error code from modbus-esp8266 lib)
  // trying to read register 1200 for busType -> 0x02 illegal address exception
  // trying to read register 44 for deviceID -> 0x02 illegal address exception
  // trying to read register 1002 for display firmware -> 0x02 illegal address exception
  
  /*
  delay(1000);
  if( (count % 5) == 0)
    Serial.printf_P("loopCount: %d\n", count);

  count++;
  */

  // // poll slave device every 5 loops
  if(count == 0)
  {
    if(!mb.slave())  // check if no transaction is in progress
    { 
      // mb.readHreg, SLAVE_ID, REG_ADDRESS, VARIABLE_NAME, NUM_REGS, CALLBACK);
      mb.readHreg(SLAVE_ID, 256, &pos, 1, cbWrite);  // actual position
      // mb.readHreg(SLAVE_ID, 257, &statusReg, 1, cbWrite);  // status bits register
      while(mb.slave())  // wait while transaction ongoing
      {
        mb.task();
        delay(10);
      }
      
      // // continous output of variables in main loop
      // // TODO print stuff on transaction success in callback
      // Serial.printf_P("actualPos: %.1f\n", (float)resReg[0]);
    }

    // reset count
    count = 0;
  }
  else
  {
    // increment count
    Serial.printf_P("count: %d\n", count);
    count ++;
  }

  // delay(1000);
  // * * * my example * * *

  // delay
  delay(2000);
}
