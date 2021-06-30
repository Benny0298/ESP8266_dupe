#pragma once
#ifndef _myModbusRTU_h
#define _myModbusRTU_h

/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include <Arduino.h>
#include <ModbusRTU.h>

/****************************************
* * * * * * * D E F I N E S * * * * * * * 
****************************************/
// Modbus RTU
#define MODBUS_SLAVE_ID   0x1
#define MODBUS_POS_REG    0x100
#define MODBUS_STATUS_REG 0x101


extern SoftwareSerial usbSerial;
extern uint32_t eSource;
extern uint16_t posReg;
extern uint16_t statusReg;
extern float pos;
/****************************************
* * * * * P R O T O T Y P E S * * * * * * 
****************************************/
namespace myModbusRTU
{
  bool modbusWrite(Modbus::ResultCode event, uint16_t transactionID, void* data);

  void modbusPollSlave(uint8_t slaveID, uint16_t readAdr, uint16_t* resReg, cbTransaction cb);
} // end of namespace
/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
#endif
