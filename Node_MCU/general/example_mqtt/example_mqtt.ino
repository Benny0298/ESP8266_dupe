#include <ESP8266WiFi.h>
#include <PubSubClient.h>

char* ssid = "HUAWEI_P30";
char* password = "zGmT!@.651926";
const char* mqttServer = "public.cloud.shiftr.io";
const int   mqttPort = 1883;
const char* mqttUser = "public";
const char* mqttPassword = "public";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMillis = 0;



void msgReceived(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Msg: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}



void setup()
{
  Serial.begin(115200);

  while(!Serial)
  {
    
  }

  WiFi.begin(ssid,password);
  
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi ...");
    delay(500);
  }
  Serial.println("Connected to WiFi");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(msgReceived);

  while(!client.connected())
  {
    Serial.println("Connecting to MQTT ...");

    if(client.connect("My_ESP_Client", mqttUser, mqttPassword))
    {
      Serial.println("Connected to MQTT");
    }
    else
    {
      Serial.print("failed with state ... ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  client.subscribe("/hello");
}



void loop() {
  client.loop();

  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    if(client.publish("/hello", "world"))
      Serial.println("publish successful");
  }
}
