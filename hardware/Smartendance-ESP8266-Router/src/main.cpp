#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SoftwareSerial.h>

// Node MAC Address
uint8_t nodeAddress[] = {0x10, 0x06, 0x1C, 0x87, 0x62, 0x70};

// Variable declarations
typedef struct struct_message
{
  char uid[32];
  char code[4];
} struct_message;

struct_message payloadRecv;
SoftwareSerial router2(D2, D1); // RX, TX

void onRecv(uint8_t *mac, uint8_t *data, uint8_t len);
void onSend(uint8_t *mac, uint8_t status);

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Initialize software serial for ESP8266
  router2.begin(9600);
  while (!router2)
    continue;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_add_peer(nodeAddress, ESP_NOW_ROLE_MAX, 1, NULL, 0);

  // Callback
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onRecv);
  esp_now_register_send_cb(onSend);
}

void loop()
{
}

void onRecv(uint8_t *mac, uint8_t *data, uint8_t len)
{
  memcpy(&payloadRecv, data, sizeof(payloadRecv));
  Serial.println("");
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.print("Received: ");
  Serial.print(len);
  Serial.print(" bytes - ");
  Serial.print(payloadRecv.uid);
  Serial.print(" - ");
  Serial.println(payloadRecv.code);

  // Send to uid and code to the second ESP8266
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.print("Sent serial: ");
  Serial.print(sizeof(payloadRecv));
  Serial.print(" bytes - ");
  Serial.print(payloadRecv.uid);
  Serial.print(" - ");
  Serial.println(payloadRecv.code);
  router2.write((uint8_t *)&payloadRecv, sizeof(payloadRecv));

  unsigned long startTime = millis();
  bool confirmation = false;
  while (millis() - startTime < 1000)
  {
    if (router2.available())
    {
      router2.readBytes((char *)&payloadRecv, sizeof(payloadRecv));
      Serial.print(millis());
      Serial.print(" -> ");
      Serial.print("Received serial to router2: ");
      Serial.print(sizeof(payloadRecv));
      Serial.print(" bytes - ");
      Serial.print(payloadRecv.uid);
      Serial.print(" - ");
      Serial.println(payloadRecv.code);
      confirmation = true;
    }
  }
  if (!confirmation)
  {
    // Change code into 201
    strcpy(payloadRecv.code, "201");
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.print("Not received serial");
    Serial.print(sizeof(payloadRecv));
  }

  // Send data back
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.print("Sending: ");
  Serial.print(sizeof(payloadRecv));
  Serial.print(" bytes - ");
  Serial.print(payloadRecv.uid);
  Serial.print(" - ");
  Serial.println(payloadRecv.code);
  esp_now_send(mac, (uint8_t *)&payloadRecv, sizeof(payloadRecv));
}

void onSend(uint8_t *mac, uint8_t status)
{
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.println(status == 0 ? "Send Success" : "Send Fail");
}