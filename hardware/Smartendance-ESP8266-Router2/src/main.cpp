#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Credentials
// #define WIFI_SSID "SHELL"
// #define WIFI_PASS "123456789"
// #define WIFI_SSID "Yes"
// #define WIFI_PASS "nico nico nii"
#define WIFI_SSID "yaa"
#define WIFI_PASS "bbbbbbbb"
#define MQTT_HOST "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "Smartendance_Router_2"
#define MQTT_PUB_TOPIC "SmarTendance/ESP32/AttendanceFinal"
#define MQTT_SUB_TOPIC "SmarTendance/ESP32/AttendanceFinal/Response"

SoftwareSerial router1(D2, D1); // RX, TX
WiFiClient router2Client;
PubSubClient router2MQTT(router2Client);
bool mqttReceived = false;

typedef struct struct_message
{
    char uid[32];
    char code[4];
} struct_message;
struct_message payloadRecv;

void onMQTTRecv(char *topic, byte *payload, unsigned int length);

void setup()
{
    Serial.begin(115200);
    router1.begin(9600);

    // Connect to Wi-Fi
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.print("Connected to WiFi: ");
    Serial.println(WiFi.SSID());

    // Connect to MQTT
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.print("Connecting to MQTT Broker: ");
    Serial.print(MQTT_HOST);
    router2MQTT.setServer(MQTT_HOST, MQTT_PORT);
    router2MQTT.setCallback(onMQTTRecv);
    while (!router2MQTT.connected())
    {
        if (router2MQTT.connect(MQTT_CLIENT_ID))
        {
            Serial.println("");
            Serial.print(millis());
            Serial.print(" -> ");
            Serial.print("Connected to MQTT Broker: ");
            Serial.println(MQTT_HOST);
            router2MQTT.subscribe(MQTT_SUB_TOPIC);
        }
        else
        {
            Serial.print(".");
            delay(500);
        }
    }
}

void loop()
{
    mqttReceived = false;
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print("Reconnecting to WiFi");
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
    }
    if (!router2MQTT.connected())
    {
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.println("Reconnecting to MQTT Broker");
        while (!router2MQTT.connected())
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                return;
            }
            else if (router2MQTT.connect(MQTT_CLIENT_ID))
            {
                Serial.print(millis());
                Serial.print(" -> ");
                Serial.print("Connected to MQTT Broker: ");
                Serial.println(MQTT_HOST);
                router2MQTT.subscribe(MQTT_SUB_TOPIC);
            }
            else
            {
                Serial.print(".");
                delay(500);
            }
        }
    }
    if (router1.available())
    {
        router1.readBytes((char *)&payloadRecv, sizeof(payloadRecv));
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print("Received serial in router2: ");
        Serial.print(sizeof(payloadRecv));
        Serial.print(" bytes - ");
        Serial.print(payloadRecv.uid);
        Serial.print(" - ");
        Serial.println(payloadRecv.code);

        // Publish to MQTT
        if (router2MQTT.publish(MQTT_PUB_TOPIC, payloadRecv.uid, true))
        {
            Serial.print(millis());
            Serial.print(" -> ");
            Serial.print("Published to MQTT Broker: ");
            Serial.print(sizeof(payloadRecv));
            Serial.print(" bytes - ");
            Serial.print(payloadRecv.uid);
            Serial.print(" - ");
            Serial.println(payloadRecv.code);

            // Wait for response
            unsigned long start = millis();
            while (millis() - start < 2000)
            {
                router2MQTT.loop();
                if (mqttReceived)
                {
                    strncpy(payloadRecv.code, "100", sizeof(payloadRecv.code)); // Change!
                    break;
                }
            }
            if (!mqttReceived)
            {
                Serial.print(millis());
                Serial.print(" -> ");
                Serial.print("Failed to receive from MQTT Broker: ");
                Serial.print(sizeof(payloadRecv));
                Serial.print(" bytes - ");
                Serial.print(payloadRecv.uid);
                Serial.print(" - ");
                Serial.println(payloadRecv.code);
                strncpy(payloadRecv.code, "201", sizeof(payloadRecv.code));
            }
            mqttReceived = false;
        }
        else
        {
            Serial.print(millis());
            Serial.print(" -> ");
            Serial.print("Failed to publish to MQTT Broker: ");
            Serial.print(sizeof(payloadRecv));
            Serial.print(" bytes - ");
            Serial.print(payloadRecv.uid);
            Serial.print(" - ");
            Serial.println(payloadRecv.code);
            strncpy(payloadRecv.code, "201", sizeof(payloadRecv.code));
        }

        // Send back to serial router1
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print("Send serial to router1: ");
        Serial.print(sizeof(payloadRecv));
        Serial.print(" bytes - ");
        Serial.print(payloadRecv.uid);
        Serial.print(" - ");
        Serial.println(payloadRecv.code);
        router1.write((uint8_t *)&payloadRecv, sizeof(payloadRecv));
    }
}

void onMQTTRecv(char *topic, byte *payload, unsigned int length)
{
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.print("Received from MQTT Broker: ");
    Serial.print(length);
    Serial.print(" bytes - ");
    Serial.print(topic);
    Serial.print(" - ");
    Serial.println((char *)payload);
    mqttReceived = true;
}