#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

typedef struct struct_message
{
  char uid[32];
  char code[32];
} struct_message;

struct_message payloadRecv;

#define WIFI_SSID "RAPFAMILY"
#define WIFI_PASS "18072001"

#define MQTT_HOST ""
#define MQTT_SUB_TOPIC "esp/test"
#define MQTT_PORT 1883
#define CLIENT_NAME "ESP8266Client"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial1.begin(9600);
}

void loop()
{
}

void connectWiFi()
{
  // Initialize WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi");
}

void connectMQTT()
{
  Serial.print("Connecting to MQTT Broker");
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqttClient.connected())
  {
    if (mqttClient.connect(CLIENT_NAME))
    {
      Serial.print("Connected to MQTT Broker");
    }
    else
    {
      Serial.print("Failed to connect to MQTT Broker");
      delay(5000);
    }
  }
}