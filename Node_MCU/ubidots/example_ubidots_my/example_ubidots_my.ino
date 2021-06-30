/****************************************
 * 
 * Include Libraries
 * 
 ****************************************/
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <stdio.h>



/****************************************
 * 
 * Define Constants
 * 
 ****************************************/
#define WIFISSID "HUAWEI_P30"
#define PASSWORD "zGmT!@.651926"
#define TOKEN "BBFF-jDX74fwNBKtjeL0NY1yXsLmavxBboY"
#define LABEL_VARIABLE_LED "esp8266_led_switch"
#define LABEL_VARIABLE_WEB2BOARD "web2board"
#define LABEL_DEVICE "my-new-device"
#define MQTT_CLIENT_NAME "my_esp8266"

#define TLS_CERT_PATH "../industrial.pem"



// char mqttBroker[] = "things.ubidots.com";
char mqttBroker[] = "industrial.api.ubidots.com";
char payload[200];
char topic[70];

// space to store values to send
char str_box_temp[6];
char str_temp[6];
char str_lat[6];
char str_lng[6];



/****************************************
 * Certificate
 ****************************************/
 /*
const char * root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "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\n" \
  "-----END CERTIFICATE-----\n";
 */
/****************************************
 * 
 * Initialize constructors for objects
 * 
 ****************************************/
ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
// WiFiClientSecure ubidots;
PubSubClient client(ubidots);




/****************************************
 * 
 * Auxiliar Functions
 * 
 ****************************************/
 /****************************************
 * Callback Handler 
 ****************************************/
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived from[");
  Serial.print(topic);
  Serial.println("] ... ");
  for(int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

 /****************************************
 * Reconnect 
 ****************************************/
void reconnect()
{
  // topics to subscribe to
  char topicLED[70];
  char topicWeb2Board[70];
  // construct strings
  sprintf(&topicLED[0], "/v1.6/devices/%s/%s/lv", LABEL_DEVICE, LABEL_VARIABLE_LED);
  sprintf(&topicWeb2Board[0], "/v1.6/devices/%s/%s/lv", LABEL_DEVICE, LABEL_VARIABLE_WEB2BOARD);
  
  // loop until connected
  while(!client.connected())
  {
    Serial.println("Attempting MQTT Connection ... ");

    // * * * * * *  try to open certificate * * * * * * *
    /*
    File ca = SPIFFS.open("../industrial.pem", "r");
    if(!ca)
    {
      Serial.println("failed to open ca file");
      while(1)
      {
        delay(100);
      }
    }
    else
      Serial.println("success opening ca file");
    */
    // * * * * * *  try to open certificate * * * * * * *
    
    // attempt to connect
    // blank password because it is not needed since we use a private token
    // connect(char* clientName, char* user, char* password)
    if(client.connect(MQTT_CLIENT_NAME, TOKEN, ""))
    {
      Serial.println("connected");
      /*
      if(ubidots.loadCACert(ca))
        Serial.println("loaded");
      else
        Serial.println("not loaded");
      */
    } else {
      Serial.print("failed rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // wait 3 seconds
      delay(3000);
    }
  }

  // subscribe to topics of choice
  bool ret = client.subscribe(&topicLED[0]);
  ret = client.subscribe(&topicWeb2Board[0]);
}



/****************************************
 * Main Functions
 ****************************************/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  /*
  SPIFFS.begin();
  delay(1000);
  if(!SPIFFS.begin())
    Serial.println("failed to mount file system");
  */
  
  pinMode(A0, INPUT);
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  Serial.println();
  Serial.println();
  Serial.print("Waiting for WiFi ... ");

  while(WiFiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  // client.setServer(mqttBroker, 8883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected())
  {
    reconnect();
  }

  // Serial.println("test1");

  // read from A0 to emulate random number
  float temperature = analogRead(A0);
  // example longitude and latitude of vienna
  float lat = 48.21;
  float lng = 16.36;

  // 4 is minimum width, 2 is precision, float is copied to str_temp
  dtostrf(temperature, 4, 2, str_temp);
  dtostrf(lat, 4, 2, str_lat);
  dtostrf(lng, 4, 2, str_lng);

  // build new topic string
  sprintf(topic, "%s", "");  // clean topic content
  sprintf(topic, "%s%s", "/v1.6/devices/", LABEL_DEVICE);

  // build payload string as a JSON dictionary according to MQTT API
  // result will loke like this:
  // {"temperature":{"value":20, "context":{"lat": 6.21, "lng":-1.2}}}
  // sprintf(payload, "%s", "");  // clean payload content
  // sprintf(payload, "{\%s\":", VARIABLE_LABEL);  // adds the variable label
  // sprintf(payload, "%s {\"value\": %s", payload, str_temp);  // adds the value
  // sprintf(payload, "%s, \"context\":{\"lat\": %s, \"lng\": %s}", payload, str_lat, str_lng);  // adds coordinates
  // sprintf(payload, "%s } }", payload);  // closes the dictionary brackets

  // publish to topic
  // client.publish(topic, payload);
  // loop and delay
  client.loop();
  delay(2000);
}
