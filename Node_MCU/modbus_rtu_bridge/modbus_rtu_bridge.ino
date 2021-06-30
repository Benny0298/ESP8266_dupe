/* * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * M O D B U S   R T U   B R I D G E * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
/* 
 *  
 * written and tested for Node MCU
 * 
 * Modbus RTU communication via Hardware Serial
 * 230400 Baud, 8 Data, 1 Stop, No Parity, Non-Inverted
 * - Tx ... D8
 * - Rx ... D7
 * 
 * Serial communication via Hardware Serial
 * - Tx ... D4
 *  
 */

#include <ModbusMaster.h>


#define MODBUS_SLAVE_ID   0x1
#define MODBUS_POS_REG    0x100
#define MODBUS_STATUS_REG 0x101


// instantiate ModbusMaster object
ModbusMaster node;

uint8_t result = 0;
uint16_t posReg = 0;
uint16_t statusReg = 0;
float pos = 0;
bool ledOut = false;


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  delay(50);

  // Modbus RTU
  // - Tx ... D8
  // - Rx ... D7
  Serial.begin(230400, SERIAL_8N1);  // 230400 baud, 8 data, 1 stop, no parity
  Serial.swap();
  node.begin(1, Serial);

  // Serial with Tx capability only
  // Tx ... D4
  Serial1.begin(9600, SERIAL_8N1);

  delay(50);
}


void loop()
{ 
  // // this works
  // Serial.write("test\n");
  // Serial1.write("test1\n");

  result = node.readHoldingRegisters(0x100, 1);
  if(result == node.ku8MBSuccess)
  {
    posReg = node.getResponseBuffer(0);
    // Serial.print("pos: ");
    // Serial.println((float)((posReg/1024.0)*100));
    Serial1.println(posReg);
    // Serial1.println("test\n");
  }
  else
  {
    // Serial.write("failed\n");
  }
  
  
  
  if(ledOut)
  {
    ledOut = false;
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
  }
  else
  {
    ledOut = true;
    digitalWrite(LED_BUILTIN, LOW);
    delay(150);
  }

  // * * * * FONA GSM  * * * * 
  for(int i = 0; i < 2; i++)
    delay(1500);

  // * * * * FONA LoRa * * * *
  /*
  for(int i = 0; i < 10; i++)
    delay(1500);
  */
}
