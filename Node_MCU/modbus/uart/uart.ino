/****************************************
* * * * * * * U S A G E * * * * * * * * * 
****************************************/


/****************************************
* * * * * * I N C L U D E S * * * * * * * 
****************************************/
#include <SoftwareSerial.h>



/**************************************
* * * * * * * D E F I N E S * * * * * * 
**************************************/
#define RTU_BUF_SIZE 7
#define USB_BUF_SIZE 7




// Table of CRC values
static const uint16_t _auchCRC[] PROGMEM = {
  0x0000, 0xC1C0, 0x81C1, 0x4001, 0x01C3, 0xC003, 0x8002, 0x41C2, 0x01C6, 0xC006, 0x8007, 0x41C7, 0x0005, 0xC1C5, 0x81C4,
  0x4004, 0x01CC, 0xC00C, 0x800D, 0x41CD, 0x000F, 0xC1CF, 0x81CE, 0x400E, 0x000A, 0xC1CA, 0x81CB, 0x400B, 0x01C9, 0xC009,
  0x8008, 0x41C8, 0x01D8, 0xC018, 0x8019, 0x41D9, 0x001B, 0xC1DB, 0x81DA, 0x401A, 0x001E, 0xC1DE, 0x81DF, 0x401F, 0x01DD,
  0xC01D, 0x801C, 0x41DC, 0x0014, 0xC1D4, 0x81D5, 0x4015, 0x01D7, 0xC017, 0x8016, 0x41D6, 0x01D2, 0xC012, 0x8013, 0x41D3,
  0x0011, 0xC1D1, 0x81D0, 0x4010, 0x01F0, 0xC030, 0x8031, 0x41F1, 0x0033, 0xC1F3, 0x81F2, 0x4032, 0x0036, 0xC1F6, 0x81F7,
  0x4037, 0x01F5, 0xC035, 0x8034, 0x41F4, 0x003C, 0xC1FC, 0x81FD, 0x403D, 0x01FF, 0xC03F, 0x803E, 0x41FE, 0x01FA, 0xC03A,
  0x803B, 0x41FB, 0x0039, 0xC1F9, 0x81F8, 0x4038, 0x0028, 0xC1E8, 0x81E9, 0x4029, 0x01EB, 0xC02B, 0x802A, 0x41EA, 0x01EE,
  0xC02E, 0x802F, 0x41EF, 0x002D, 0xC1ED, 0x81EC, 0x402C, 0x01E4, 0xC024, 0x8025, 0x41E5, 0x0027, 0xC1E7, 0x81E6, 0x4026,
  0x0022, 0xC1E2, 0x81E3, 0x4023, 0x01E1, 0xC021, 0x8020, 0x41E0, 0x01A0, 0xC060, 0x8061, 0x41A1, 0x0063, 0xC1A3, 0x81A2,
  0x4062, 0x0066, 0xC1A6, 0x81A7, 0x4067, 0x01A5, 0xC065, 0x8064, 0x41A4, 0x006C, 0xC1AC, 0x81AD, 0x406D, 0x01AF, 0xC06F,
  0x806E, 0x41AE, 0x01AA, 0xC06A, 0x806B, 0x41AB, 0x0069, 0xC1A9, 0x81A8, 0x4068, 0x0078, 0xC1B8, 0x81B9, 0x4079, 0x01BB,
  0xC07B, 0x807A, 0x41BA, 0x01BE, 0xC07E, 0x807F, 0x41BF, 0x007D, 0xC1BD, 0x81BC, 0x407C, 0x01B4, 0xC074, 0x8075, 0x41B5,
  0x0077, 0xC1B7, 0x81B6, 0x4076, 0x0072, 0xC1B2, 0x81B3, 0x4073, 0x01B1, 0xC071, 0x8070, 0x41B0, 0x0050, 0xC190, 0x8191,
  0x4051, 0x0193, 0xC053, 0x8052, 0x4192, 0x0196, 0xC056, 0x8057, 0x4197, 0x0055, 0xC195, 0x8194, 0x4054, 0x019C, 0xC05C,
  0x805D, 0x419D, 0x005F, 0xC19F, 0x819E, 0x405E, 0x005A, 0xC19A, 0x819B, 0x405B, 0x0199, 0xC059, 0x8058, 0x4198, 0x0188,
  0xC048, 0x8049, 0x4189, 0x004B, 0xC18B, 0x818A, 0x404A, 0x004E, 0xC18E, 0x818F, 0x404F, 0x018D, 0xC04D, 0x804C, 0x418C,
  0x0044, 0xC184, 0x8185, 0x4045, 0x0187, 0xC047, 0x8046, 0x4186, 0x0182, 0xC042, 0x8043, 0x4183, 0x0041, 0xC181, 0x8180,
  0x4040, 0x0000
};


/****************************************
* * * * * * * G L O B A L S * * * * * * * 
****************************************/
SoftwareSerial usbSerial(3, 1);

const uint8_t zeroByte = 0x00;

char rtuBuf[RTU_BUF_SIZE];
char usbBuf[USB_BUF_SIZE];
char* buf;

uint16_t check = 0;
uint16_t result = 0;
int count = 0;

bool newData = false;


/****************************************
* * * * * P R O T O T Y P E S * * * * * * 
****************************************/
void recData();
void handleData();
void clearBuf();
void readHreg(uint8_t slaveID, uint16_t readAddr, uint16_t* resReg);
uint16_t crc16(char *buf, int len);


/****************************************
* * * * * * * * S E T U P * * * * * * * * 
****************************************/
void setup() {
  // put your setup code here, to run once:
  
  // config hardware serial for modbus interface on pins D7/D8
  Serial.begin(230400, SERIAL_8N1);
  Serial.swap();
  // config software serial for debugging via usb ... Tx functionality only
  usbSerial.begin(115200, SWSERIAL_8N1);

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  for(int i = 0; i < sizeof(rtuBuf); i++)
    rtuBuf[i] = '\0';

  for(int i = 0; i < sizeof(usbBuf); i++)
    usbBuf[i] = '\0';

  delay(100);
  if(Serial.available() > 0)
  {
    Serial.readBytes(rtuBuf, sizeof(rtuBuf));
    for(int i = 0; i < sizeof(rtuBuf); i++)
      rtuBuf[i] = '\0';
  }
}




/****************************************
* * * * * * M A I N   L O O P * * * * * * 
****************************************/
void loop() {
  // put your main code here, to run repeatedly:
  // rtuSerial.write("test");
  
  // usbSerial.write("test\n");
  count++;

  /*
  Serial.write(0x01);
  Serial.write(0x03);
  Serial.write(0x01);
  Serial.write(zeroByte);
  Serial.write(zeroByte);
  Serial.write(0x01);
  Serial.write(0x85);
  Serial.write(0xF6);
  */

  
  
  // sprintf(rtuBuf, "%02X%02X%02X%02X%02X%02X", 0x01, 0x03, 0x01, 0x00, 0x00, 0x01);
  // check = crc16(&rtuBuf[0], sizeof(rtuBuf));
  // usbSerial.write("test\n");

  readHreg(1, 256, &result);
  recData();
  handleData();

  // end of loop
  delay(500);
}


/****************************************
* * * * * S U B   R O U T I N E S * * * * 
****************************************/
void recData()
{
  int recCount = 0;
  
  if(Serial.available() > 0)
  {
    // delay(5);
    recCount = Serial.readBytes(rtuBuf, sizeof(rtuBuf));
    newData = true;
  }
}


void handleData()
{
  if(newData == true)
  {
    // do stuff with received data
    // usbSerial.write(rtuBuf, sizeof(rtuBuf));
    
    uint8_t addr      = 0;
    uint8_t opCode    = 0;
    uint8_t numData   = 0;
    uint8_t dataLow   = 0;
    uint8_t dataHigh  = 0;
    uint8_t checkLow  = 0;
    uint8_t checkHigh = 0;

    addr      = (uint8_t) rtuBuf[0];
    opCode    = (uint8_t) rtuBuf[1];
    numData   = (uint8_t) rtuBuf[2];
    dataHigh  = (uint8_t) rtuBuf[3];
    dataLow   = (uint8_t) rtuBuf[4];
    checkHigh = (uint8_t) rtuBuf[5];
    checkLow  = (uint8_t) rtuBuf[6];

    // calc actual data
    uint16_t data = (uint16_t)dataHigh*256 + (uint16_t)dataLow;
    float pos = (float)(data / 1023.0)*100.0;
    sprintf(usbBuf, "%.1f", pos);
    usbSerial.write("pos:");
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");
    clearBuf();

    /*
    usbSerial.write("addr:");
    sprintf(usbBuf, "%d", addr);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");

    usbSerial.write("opCode:");
    sprintf(usbBuf, "%d", opCode);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");

    usbSerial.write("numData:");
    sprintf(usbBuf, "%d", numData);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");
    
    usbSerial.write("data:");
    sprintf(usbBuf, "%d%d", dataHigh, dataLow);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");

    usbSerial.write("check:");
    sprintf(usbBuf, "%d%d", checkHigh, checkLow);
    usbSerial.write(usbBuf, sizeof(usbBuf));
    usbSerial.write("\n");
    */
    
    
    clearBuf();
    // usbSerial.write(usbBuf, sizeof(usbBuf));
    
    newData = false;
  }
}



void clearBuf()
{
  for(int i = 0; i < (RTU_BUF_SIZE - 1); i++)
    rtuBuf[i] = '\0';
  
  for(int i = 0; i < (USB_BUF_SIZE - 1); i++)
    usbBuf[i] = '\0';
}


void readHreg(uint8_t slaveID, uint16_t readAddr, uint16_t* resReg)
{
  /*
  Serial.write(0x01);
  Serial.write(0x03);
  Serial.write(0x01);
  Serial.write(zeroByte);
  Serial.write(zeroByte);
  Serial.write(0x01);
  Serial.write(0x85);
  Serial.write(0xF6);
  */

  // usbSerial.write("test");

  /*
  uint8_t buf[6] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01};
  for(int i = 0; i < 6; i++)
  {
    sprintf(&usbBuf[i], "%d", buf[i]);
  }
  // usbSerial.write(usbBuf, sizeof(usbBuf));
  // usbSerial.write("\n");
  */

  /*
  uint16_t check = crc16((char*)&usbBuf[0], sizeof(usbBuf));
  uint8_t checkHigh = (uint8_t) (check / 256);
  uint8_t checkLow = (uint8_t) check;
  */
  
  // usbBuf[0] = checkLow;
  // usbBuf[1] = checkHigh;
  // usbSerial.write(usbBuf, sizeof(usbBuf));
  // usbSerial.write("\n");

  // sprintf(usbBuf, "%d", checkLow);
  // usbSerial.write(usbBuf, sizeof(usbBuf));
  // usbSerial.write("\n");

  // sprintf(usbBuf, "%d", checkHigh);
  // usbSerial.write(usbBuf, sizeof(usbBuf));
  // usbSerial.write("\n");
  
  /*
  sprintf(usbBuf, "%d", checkLow);
  usbSerial.write(usbBuf, sizeof(usbBuf));
  sprintf(usbBuf, "%d", checkHigh);
  usbSerial.write(usbBuf, sizeof(usbBuf));
  */

  // slave ID
  Serial.write(slaveID);
  // opCode
  Serial.write(0x03);

  // readAddr
  if(readAddr > 255)
  {
    uint8_t highAddr = (uint8_t) (readAddr / 256);
    Serial.write(highAddr);
    // low byte
    readAddr = (uint8_t)readAddr;
    Serial.write(readAddr);
  }
  else
  {
    Serial.write(readAddr); 
  }

  // num registers to read
  Serial.write(0x00);
  Serial.write(0x01);
  // crc16 check sum
  uint8_t crcBuf[6] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01};
  for(int i = 0; i < 6; i++)
  {
    sprintf(usbBuf, "%02X", crcBuf[i]);
    usbSerial.write(usbBuf, strlen(usbBuf));
  }
  usbSerial.write('\n');
  
  uint16_t check = crc16((char*)&crcBuf[0], sizeof(crcBuf));
  uint8_t checkHigh = (uint8_t)(check / 256);
  uint8_t checkLow = (uint8_t)check;

  usbSerial.write(133);
  usbSerial.write(246);
  usbSerial.write('\n');

  char thisBuf[10];
  sprintf(&thisBuf[0], "%02X", checkLow);
  Serial.write(&thisBuf[0], strlen(thisBuf));
  sprintf(&thisBuf[0], "%02X", checkHigh);
  Serial.write(&thisBuf[0], strlen(thisBuf));

  /*
  sprintf(usbBuf, "%02X", checkLow);
  usbSerial.write(usbBuf, sizeof(usbBuf));
  sprintf(usbBuf, "%02X", checkHigh);
  usbSerial.write(usbBuf, sizeof(usbBuf));*/

  //Serial.write(0x85);
  //Serial.write(0xF6);

  //Serial.write(checkLow);
  //Serial.write(checkHigh);
}



uint16_t crc16(char *, int len)
{
  uint16_t crc = 0xFFF;

  for(int pos = 0; pos < len; pos++)
  {
    crc ^= (uint16_t)buf[pos];

    for(int i = 8; i != 0; i--)
    {
      if((crc & 0x0001 != 0))
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
        crc >>=1;
    }
  }

  return crc;
}
