/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include "myModbusRTU.h"

/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
ModbusRTU mb;

/****************************************
* * * * * * F U N C T I O N S * * * * * * 
****************************************/
namespace myModbusRTU
{
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
} // enf of namespace

/****************************************
* * * * * * * * * E O F * * * * * * * * * 
****************************************/
// EOF
