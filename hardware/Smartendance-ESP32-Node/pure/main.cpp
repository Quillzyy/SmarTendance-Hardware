#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>

// Slave MAC Address
uint8_t routerAddress[] = {0x8C, 0xAA, 0xB5, 0xF8, 0x9B, 0x9C};

// Variable declarations
typedef struct struct_message
{
  char uid[32];
  char code[4];
} struct_message;

esp_now_peer_info_t peerInfo;

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status);
void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  memcpy(peerInfo.peer_addr, routerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  // Callback
  esp_now_register_send_cb(onSend);
  esp_now_register_recv_cb(onRecv);
}

void loop()
{
  struct_message payloadSend;
  // Insert the data
  strcpy(payloadSend.uid, "as12as12");
  strcpy(payloadSend.code, "200");

  // Serialize the data
  Serial.println("");
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.print("Sending: ");
  Serial.print(sizeof(payloadSend));
  Serial.print(" bytes - ");
  Serial.print(payloadSend.uid);
  Serial.print(" - ");
  Serial.println(payloadSend.code);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(routerAddress, (uint8_t *)&payloadSend, sizeof(payloadSend));
  delay(5000);
}

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void onRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  struct_message payloadRecv;
  memcpy(&payloadRecv, data, sizeof(payloadRecv));
  Serial.print(millis());
  Serial.print(" -> ");
  Serial.print("Received: ");
  Serial.print(data_len);
  Serial.print(" bytes - ");
  Serial.print(payloadRecv.uid);
  Serial.print(" - ");
  Serial.println(payloadRecv.code);
}