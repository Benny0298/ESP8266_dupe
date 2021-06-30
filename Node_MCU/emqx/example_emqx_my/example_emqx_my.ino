#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "HUAWEI_P30"; // Enter your WiFi name
const char *password = "zGmT!@.651926";  // Enter WiFi password
const char *mqtt_broker = "p68c3349.en.emqx.cloud";
const int mqtt_port = 11977;
const char *topic = "esp/test";
const char *mqtt_username = "esp8266";
const char *mqtt_password = "test1234";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    //connecting to a mqtt broker
    while (!client.connected()) {
        String client_id = "esp8266-client-";
        client_id += String(WiFi.macAddress());
        const char * c_id = client_id.c_str();
        Serial.println(c_id);
        Serial.println("Connecting to public emqx mqtt broker.....");
        if (client.connect(c_id, mqtt_username, mqtt_password)) {
            Serial.println("My emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // publish and subscribe
    client.publish(topic, "hello emqx");
    client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

int count = 0;

void loop() {
    client.loop();
    delay(5000);
    Serial.print("alive");
    Serial.println(count);
    count++;
    // client.publish(topic, "test");
}
