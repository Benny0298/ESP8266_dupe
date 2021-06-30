/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "HUAWEI_P30"
#define WLAN_PASS       "zGmT!@.651926"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // use port 1883 for standard communication || use port 8883 for TLS/SSL encyption
#define AIO_USERNAME    "MrPinkward"
#define AIO_KEY         "aio_cxhn01FzdsfWCtpnQggxf7aPRPhM"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
// WiFiClient client;
// or... use WiFiClientSecure for SSL
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// io.adafruit.com SHA1 ingerprint
// taken from most recent adafruit example File>Examples>Adafruit MQTT Library>adafruitio_secure_esp8266
static const char* fingerprint PROGMEM = "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF";

/****************************** Feeds ***************************************/

// Setup a feed called 'randnum' for publishing.
// Setup a feed called 'text'    for publishing
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish randnum   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/randnum");
Adafruit_MQTT_Publish board2web = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/board2web");

// Setup a feed called 'esp8266_led_switch' for subscribing to changes.
// Setup a feed called 'text'               for subscribing to changes
Adafruit_MQTT_Subscribe esp8266_led_switch = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/esp8266-led-switch");
Adafruit_MQTT_Subscribe web2board          = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/web2board");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  
  //on-board LED
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  Serial.begin(115200);
  delay(100);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // check the fingerprint of io.adafruit.com's SSL cert
  client.setFingerprint(fingerprint);

  // Setup MQTT subscription for onoff feed.
  // Setup MQTT subscription for text  feed.
  mqtt.subscribe(&esp8266_led_switch);
  mqtt.subscribe(&web2board);
}

int count   = 0;

void loop() {

  // random number
  int randNum = 0;
  char myBuffer[20];
  char textBuffer[20];
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.  
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &esp8266_led_switch) {
      Serial.print(F("Received LED: "));
      Serial.println((char *)esp8266_led_switch.lastread);
      if(strncmp((char *)esp8266_led_switch.lastread,"ON",2))
        digitalWrite(2, HIGH);
      else if(strncmp((char *)esp8266_led_switch.lastread,"OFF",3))
        digitalWrite(2, LOW);
    }
    if (subscription == &web2board) {
      Serial.print(F("Received Text: "));
      Serial.println((char *)web2board.lastread);
    }
  }

  // // Now we can publish stuff!
  // // generate random number
  randNum = random(10);
  Serial.print(F("Sending randnum val "));
  Serial.print(randNum);
  Serial.print("...");
  if (! randnum.publish(randNum)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  // generate text
  sprintf(&myBuffer[0], "board2web%d\0", count);
  strncpy(&textBuffer[0],&myBuffer[0], 20);
  Serial.print(F("Sending Text: "));
  Serial.print(&textBuffer[0]);
  Serial.print("...");
  if(!board2web.publish(&textBuffer[0])){
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK"));
  }

  count++;

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
