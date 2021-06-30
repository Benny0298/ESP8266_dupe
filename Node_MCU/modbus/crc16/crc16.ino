#include <SoftwareSerial.h>

char* t = (char*)"DEADBEEF";


SoftwareSerial usbSerial(3,1);


uint16_t ModRTU_CRC(char * buf, int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;  
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  usbSerial.begin(115200, SWSERIAL_8N1);
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println(ModRTU_CRC(t, strlen(t)), HEX);
  char usbBuf[10];

  uint8_t buf[6] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01};
  for(int i = 0; i < 6; i++)
  {
    sprintf(&usbBuf[0], "%02X", buf[i]);
    usbSerial.write(&usbBuf[0], strlen(usbBuf));
    // usbSerial.write(buf[i]);
  }
  usbSerial.write('\n');

  /*
  Serial.println("");
  Serial.println(ModRTU_CRC((char*)&buf[0], sizeof(buf)), HEX);
  */


  uint16_t check = ModRTU_CRC((char*)&buf[0], sizeof(buf));
  uint8_t checkHigh = (uint8_t)(check / 256);
  uint8_t checkLow = (uint8_t)check;

  sprintf(&usbBuf[0], "%02X", checkLow);
  usbSerial.write(&usbBuf[0], strlen(usbBuf));
  sprintf(&usbBuf[0], "%02X", checkHigh);
  usbSerial.write(&usbBuf[0], strlen(usbBuf));
  usbSerial.write('\n');

  /*
  char serBuf[2] = "";
  sprintf(&serBuf[0], "%02X", checkLow);
  Serial.write(&serBuf[0], sizeof(serBuf));
  Serial.write('\n');
  */
  
  // Serial.print(checkLow, HEX);
  // Serial.println(checkHigh, HEX);
  /*
  uint8_t serBuf[10];
  sprintf((char*)serBuf, "%02X", checkLow);
  Serial.write(serBuf, strlen((char*)serBuf));
  sprintf((char*)serBuf, "%02X", checkHigh);
  Serial.write(serBuf, strlen((char*)serBuf));
  Serial.write("\n");
  */

  
  delay(500);
}
