#include <SoftwareSerial.h>
#include <ModbusRTU.h>




#define MODBUS_SLAVE_ID 0x1
#define MODBUS_POS_REG  0x100
#define MODBUS_STAT_REG 0x101




SoftwareSerial usbSerial(3, 1);
ModbusRTU mb;


uint16_t posReg = 0;
float pos = 0.0;



bool modbusWrite(Modbus::ResultCode event, uint16_t transactionID, void* data)
{
  if(event != Modbus::EX_SUCCESS)
  {
    uint32_t eSource = mb.eventSource();
    usbSerial.write("requestResult: 0x");
    char usbBuf[3];
    sprintf(usbBuf, "%02X\n", event);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    return false;
  }
  else if(event == Modbus::EX_SUCCESS)
  {
    pos = (float)((posReg/1023.0)*100.0);  // calc position [%] from returned reg value | range 0 ... 1023
    char usbBuf[11];
    sprintf(usbBuf, "pos:%.1f%%\n", pos);
    usbSerial.write(usbBuf, sizeof(usbBuf));
  }

  return true;
}






void setup()
{
  // setup hardware seial for modbus rtu interface
  Serial.begin(230400, SERIAL_8N1);
  Serial.swap();
  mb.begin(&Serial);
  mb.master();

  // setup software serial (on usb port) for debugging ... Tx functionality only
  // use 115200 baud because of error probability
  usbSerial.begin(115200, SWSERIAL_8N1);
}






void loop()
{
  /*
  Serial.print("testSerial\n");
  delay(20);
  usbSerial.write("testUSB\n");
  */

  if(!mb.slave())
  {
    mb.readHreg(MODBUS_SLAVE_ID, MODBUS_POS_REG, &posReg, 1, modbusWrite);

    while(mb.slave())
    {
      mb.task();
      delay(10);
    }
  }








  
  delay(500);
}
