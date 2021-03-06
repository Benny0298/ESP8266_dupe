// #define USEDHT22 1

#include <FS.h>

#include <MQTTClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#ifdef USEDHT22
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#endif


#include "sha256.h"
#include "ConnectionStringHelper.h"

/* ----------------------------------------------------------------------------- *
 * This Sample Code is provided for  the purpose of illustration only and is not * 
 * intended  to be used in a production  environment.  THIS SAMPLE  CODE AND ANY * 
 * RELATED INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER * 
 * EXPRESSED OR IMPLIED, INCLUDING  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF * 
 * MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.                      * 
 * ----------------------------------------------------------------------------- */

/*
 * 
 * Demonstrates connecting an ESP8266 to an Azure IoT hub using a third party MQTT stack.
 * See: https://azure.microsoft.com/en-us/documentation/articles/iot-hub-mqtt-support/#using-the-mqtt-protocol-directly
 * 
 */

const uint INTERVAL = 20;             // SAS token validity period in minutes
const int REFRESHPERCENTAGE = 80;     // Percentage of SAS token validity used when reauthentication is attempted
const int MISSEDTOOMANY = 10;         // Reconnect to MQTT when consecutively missed this many messages 
const int BLINKER = BUILTIN_LED;
const int BLINKTIME = 700;
const int BLINKOFF = HIGH;
const int BLINKON = LOW;              // Set to BLINKOFF to disable LED

#ifdef USEDHT22
#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
#endif

WiFiUDP ntpUDP;
WiFiClientSecure net;
NTPClient timeClient(ntpUDP);
MQTTClient client(256);
ConnectionStringHelper *csHelper = NULL;
String pubTopic;
String subTopic;
int32_t refreshTime = 0;
bool shouldSaveConfig = false;

//
// Code included to assist debugging
/*
extern "C"
{
void dump_buffer(uint8_t length, unsigned char *buffer)
{
    char hex[4];
    
    for (int i = 0; i < length; i += 16)
    {
        int j;
        uint8_t chunk = (length - i < 16)? length - i : 16;
        
        for (j = i; j < i + chunk; j++)
        {
            sprintf(hex, " %02x", buffer[j]);
            Serial.print(hex);
        }
        
        Serial.print("  ");

        if (chunk < 16)
        {
            for (j = chunk; j < 16; j++)
                Serial.print("   ");
        }
        
        for (j = i; j < i + chunk; j++)
        {
            Serial.write((isprint(buffer[j]))? buffer[j] : '.');
        }
        
        Serial.println();
    }
}
}
*/

//void softwareReboot()
//{
//  Serial.println("Rebooting...");
//  wdt_enable(WDTO_250MS);
//  while(true);
//}

void saveConfigCallback() 
{
  Serial.println("Config needs to be saved");
  shouldSaveConfig = true;
}

void configModeCallback(WiFiManager *wifiManager)
{
  Serial.println("Entering configuration mode");
  Serial.println(WiFi.softAPIP());
}

void setupWiFi()
{
  bool forceConfig = false;
  Serial.println("Mounting SPIFFS");

  if (SPIFFS.begin())
  {
    if (SPIFFS.exists("/config.txt"))
    {
      Serial.println("Opening config.txt");
  
      File configFile = SPIFFS.open("/config.txt", "r");
  
      if (configFile)
      {
        Serial.println("Found config.txt");
  
        size_t size = configFile.size();
        char *configData = new char[size];
  
        configFile.readBytes(configData, size);
        
        csHelper = new ConnectionStringHelper(configData);
        Serial.print("Connection string = ");
        Serial.println(configData);
        delete [] configData;
        configFile.close();
      }
      else
      {
        Serial.println("Failed to open config.txt");
        forceConfig = true;   // May not help any
      }
    }
    else
    {
      Serial.println("File config.txt not found");
      forceConfig = true;
    }
  }
  else
  {
    Serial.println("Failed to mount SPIFFS");
    forceConfig = true;

  }

  WiFiManagerParameter connectionStringParm("connectionString", "Connection String", NULL, 150);
  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.addParameter(&connectionStringParm);

  Serial.println("Connecting or configuring");

  bool connected = false;
  
  if (forceConfig)
  {
    // Don't bother trying to connect we need more info
    connected = wifiManager.startConfigPortal("HUAWEI_P30", "zGmT!@.651926");
  }
  else
  {
    connected = wifiManager.autoConnect("HUAWEI_P30", "zGmT!@.651926");  // my wifi config here
  }

  if (!connected)
  {
    Serial.println("Failed to connect to Wi-Fi");
    ESP.reset();
  }

  Serial.println(WiFi.SSID());
  
  Serial.println();
  Serial.print("IP Address = ");
  Serial.println(WiFi.localIP());

  if (shouldSaveConfig)
  {
    Serial.println("Saving config.txt");

    File configFile = SPIFFS.open("/config.txt", "w");

    if (!configFile)
    {
      Serial.println("Failed to open config.txt");
    }

    Serial.println(configFile.write((const uint8_t *)connectionStringParm.getValue(), strlen(connectionStringParm.getValue()) + 1));
    configFile.close();

    // Sanity check
    if (!SPIFFS.exists("/config.txt"))
    {
      Serial.println("WTF... where is my new file?");
    }

    csHelper = new ConnectionStringHelper(connectionStringParm.getValue());
    Serial.println("Saved config.txt");
  }
}

void setupMQTT(uint interval)
{
  long now = timeClient.getEpochTime();
  long epochTime = now + (interval * 60);
  String mqttUserName = csHelper->getKeywordValue("hostname") + "/" + csHelper->getKeywordValue("deviceid");
  String mqttClientId = csHelper->getKeywordValue("deviceid");
  String mqttServer = csHelper->getKeywordValue("hostname");
  String mqttPassword = "";

  
  csHelper->generatePassword(mqttServer, mqttClientId, csHelper->getKeywordValue("SharedAccessKey"), epochTime, mqttPassword);
  refreshTime = now + (interval * 60 * REFRESHPERCENTAGE / 100);
  Serial.print("Time is now ");
  Serial.println(now, DEC);
  Serial.print("SAS token expires at ");
  Serial.println(epochTime, DEC);
  Serial.print("Will refresh at ");
  Serial.println(refreshTime, DEC);
  
//  Serial.print("MQTT_Server=");
//  Serial.println(mqttServer);
//  Serial.print("MQTT_ClientId=");
//  Serial.println(mqttClientId);
//  Serial.print("MQTT_UserName=");
//  Serial.println(mqttUserName);
//  Serial.print("MQTT_Password=");
//  Serial.println(mqttPassword);

  uint8_t ret;
  client.begin(mqttServer.c_str(), 8883, net);
  client.setOptions(240, 0, 1000);
  client.onMessage(messageReceived);

  int counter = 20;
  
  // If we can't connect in 20 attempts then reboot
  while (!client.connect(mqttClientId.c_str(), mqttUserName.c_str(), mqttPassword.c_str()))
  {
    Serial.print(".");
    delay(1000);    

    if (0 >= --counter)
    {
      ESP.reset();
    }
  }

  Serial.println("MQTT connected");
  
  pubTopic = "devices/" + mqttClientId + "/messages/events/";
  subTopic = "devices/" + mqttClientId + "/messages/devicebound/#";

  if (!client.subscribe(subTopic.c_str(), 1))
    Serial.println("Subscribe failed");
}

void setup() 
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Start Test");

  pinMode(BLINKER, OUTPUT);
  digitalWrite(BLINKER, BLINKOFF);

  setupWiFi();

  timeClient.begin();

  // Cannot continue until we have a clock update
  while (!timeClient.update())
  {
    Serial.println("Initial clock update failed");
    delay(250);
  }
  delay(1000);
  
  setupMQTT(INTERVAL);
}

void loop() {

  static uint32_t counter = 0;
  static uint32_t lastMsg = 0;
  static uint32_t missedMessageCount = 0;
  static uint32_t ledOffAt = 0;
  uint32_t currMillis;

#ifdef USEDHT22
  double temp;
  double humidity;
  char convBuf[20];
#endif
  
  while (!timeClient.update())
  {
    Serial.println("NTP update failed");
    delay(250);
  }
  
  client.loop();

  // Refresh the SAS token if necessary
  if (timeClient.getEpochTime() > refreshTime)
  {
    Serial.println("Reconnecting to refresh SAS token");

    client.disconnect();
    setupMQTT(INTERVAL);
    missedMessageCount = 0;
  }

  currMillis = millis();

  if (ledOffAt && currMillis >= ledOffAt)
  {
    digitalWrite(BLINKER, BLINKOFF);
    ledOffAt = 0;
  }
  
  // Check for wrap (around every 50 days)
  if (currMillis < lastMsg)
    lastMsg = 0;

  // Send a message about every three seconds
  if (currMillis - lastMsg > 3000)
  {
    lastMsg = currMillis;

    String msg;
    bool skipMsg = false;

#ifdef USEDHT22
    temp = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temp) || isnan(humidity))
    {
      Serial.println("Bad reading from DHT22 - message skipped");
      skipMsg = true;
    }
    else
    {
      msg = "{ \"deviceid\": " + String(csHelper->getKeywordValue("deviceid")) +
            " \"status\": { \"temperature\": " + dtostrf(dht.convertCtoF(temp), 4, 2, convBuf) +
            " \"humidity\": " + dtostrf(humidity, 4, 2, convBuf) +
            " } }";
      Serial.println(msg);
    }
#else    
    msg = "{ \"counter\": " + String(counter++) + ", \"epoch\": " + String(timeClient.getEpochTime()) + ", \"millis\": " + String(currMillis) + " }";
#endif  
    if (!skipMsg)
    {  
      if (!client.publish(pubTopic.c_str(), msg.c_str(), false, 1))
      {
        Serial.println("Publish failed");
  
        if (++missedMessageCount > MISSEDTOOMANY)
        {
          missedMessageCount = 0;
          client.disconnect();
          setupMQTT(INTERVAL);
        }
      }
      else
      {
        missedMessageCount = 0;
        digitalWrite(BLINKER, BLINKON);
        ledOffAt = currMillis + BLINKTIME;
      }
    }
  }
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("Message received");
  Serial.println("\tTopic: " + topic + "\n\tPayload: " + payload);
}
