#include <ESP8266WiFi.h>
#include <espnow.h>

// Node MAC Address
uint8_t nodeAddress[] = {0x10, 0x06, 0x1C, 0x87, 0x62, 0x70};

// Variable declarations
typedef struct struct_message
{
  char uid[32];
  char code[4];
} struct_message;

struct_message payloadRecv;

void onRecv(uint8_t *mac, uint8_t *data, uint8_t len);
void onSend(uint8_t *mac, uint8_t status);

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    continue;

  Serial1.begin(115200);
  while (!Serial1)
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
  esp_now_add_peer(nodeAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

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

  // // Send the data by serial to another ESP8266
  // Serial1.write((uint8_t *)&payloadRecv, sizeof(payloadRecv));
  // Serial1.flush();

  // // Wait for callback from the other ESP8266
  // while (Serial1.available() < sizeof(payloadRecv))
  //   continue;

  // Send data back
  strcpy(payloadRecv.code, "100"); // Change after creating serial comms
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