/****************************************
* Include Libraries
****************************************/

#include "Ubidots.h"

/****************************************
* Define Instances and Constants
****************************************/   
 
const char* UBIDOTS_TOKEN = "BBFF-jDX74fwNBKtjeL0NY1yXsLmavxBboY";  // Put here your Ubidots TOKEN
const char* WIFI_SSID = "HUAWEI_P30";      // Put here your Wi-Fi SSID
const char* WIFI_PASS = "zGmT!@.651926";   // Put here your Wi-Fi password 

Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);

/****************************************
* Auxiliar Functions
****************************************/

// Put here your auxiliar functions

/****************************************
* Main Functions
****************************************/  

void setup() {                       

  Serial.begin(115200);
  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  // ubidots.setDebug(true);  // Uncomment this line for printing debug  messages                     
}

void loop() {

  int value1 = random(1, 10);
  ubidots.add("randNum", value1);// Change for your variable name
  
  bool bufferSent = false;
  bufferSent = ubidots.send(); // Will send data to a device label that matches the device Id

  if (bufferSent) {
  // Do something if values were sent properly
   Serial.print("Value ");
   Serial.print(value1);
   Serial.println(" sent by device");
  }
  delay(5000);
}
