#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// WiFi credentials
const char* ssid = "IoTGateway_AP";
const char* password = "raspberrypi123";

// MQTT broker
const char* mqtt_server = "192.168.50.1";

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

// Device info
char deviceId[] = "ESP32-001";

// Static IP configuration
IPAddress local_IP(192, 168, 50, 21);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);

void setup_wifi() {
  Serial.println("Connecting to WiFi...");

  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceId)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    String payload = "{\"device\":\"" + String(deviceId) + "\",\"temp\":" + String(t) + ",\"humidity\":" + String(h) + "}";
    String topic = "sensors/" + String(deviceId) + "/telemetry";
    client.publish(topic.c_str(), payload.c_str());
    Serial.println("Published: " + payload);
  } else {
    Serial.println("Failed to read from DHT sensor");
  }

  delay(5000);
}
