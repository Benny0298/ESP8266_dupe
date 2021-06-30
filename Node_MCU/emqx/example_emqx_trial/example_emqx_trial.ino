#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "HUAWEI_P30"
#define wifi_password "zGmT!@.651926"

#define mqtt_server "p68c3349.en.emqx.cloud"  // MQTT Cloud address
#define test_topic "test"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 11977);
}

void setup_wifi() {
    delay(10);
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("esp_id", "esp_user", "esp_pass")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }

    client.subscribe(test_topic);
}

long lastMsg = 0;
int  randNum = 0;

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 30000) {
        // Wait a few seconds between measurements
        lastMsg = now;

        // random number
        randNum = random(10);
        Serial.print("New Random: ... ");
        Serial.println(String(randNum).c_str());
        client.publish(test_topic, String(randNum).c_str(), true);
    }
}
