#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include "secrets.h"
// #include <SPI.h>

/* RC522 SENSOR SETUP */
const byte SS_PIN = 5;
const byte RST_PIN = 0;
MFRC522 mfrc522(SS_PIN, RST_PIN);

/* WIFI CREDENTIALS */
const char* WIFI_SSID = wifiSSID;
const char* WIFI_PASS = wifiPass;

/* MQTT BROKER CREDENTIALS */
const char* MQTT_HOST = mqttHost;
const int MQTT_PORT = mqttPort;
const char* CLIENT_NAME = clientName;
const char* MQTT_PUB_TOPIC = pubTopic;
const char* MQTT_SUB_TOPIC = subTopic;

const int I2C_SDA = 21;
const int I2C_SCL = 22;

bool messageReceived = false;

/* I2C LCD Setup */
LiquidCrystal_I2C lcd(0x27, 16, 2); // Address 0x27, 16 columns and 2 rows

/* INSTANCE AN OBJECT FROM 'WiFiClientSecure' and 'PubSubClient' CLASSES */
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* FUNCTION DECLARATIONS */
void connectWiFi();
String readUID();
void connectMqttBroker();
bool publishUID(const char* uid);
bool waitForConfirmation();
void initializeText();
void callback(char* topic, byte* payload, unsigned int length);


void callback(char* topic, byte* payload, unsigned int length) {
  char status[4];
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  for (int i = 0; i < 3; i++) {
    status[i] = (char)payload[i];
  }
  status[3] = '\0';
  Serial.print("Message: ");
  Serial.println(status);

  messageReceived = true;

  // Check if status is 100, 101, 102, 103, or 104
  if(strcmp(status, "100") == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Successful!");
    lcd.setCursor(0, 1);
    lcd.print("Attended");
  }
  else if(strcmp(status, "101") == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("User not found");
  }
  else if(strcmp(status, "102") == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Course not found");
  }
  else if(strcmp(status, "103") == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Already attended");
  }
  else if(strcmp(status, "104") == 0){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Role not found");
  }
  delay(2000);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println();
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected to: ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}


String readUID() {
  String uid = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    for (byte i = 0; i < mfrc522.uid.size; i++)
      uid += String(mfrc522.uid.uidByte[i], HEX);
    mfrc522.PICC_HaltA();
  }
  return uid;
}


void connectMqttBroker() {
  while (!mqttClient.connected() && WiFi.status() == WL_CONNECTED && !mqttClient.subscribe(MQTT_SUB_TOPIC)){
    Serial.println("Connecting to MQTT broker: " + String(MQTT_HOST));
    if (mqttClient.connect(CLIENT_NAME)) {
      Serial.println("Connected to MQTT broker: " + String(MQTT_HOST));
      mqttClient.subscribe(MQTT_SUB_TOPIC);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(": Connection to MQTT broker failed!");
      Serial.println("Try again in 3 seconds.");
      /* Wait 3 seconds before retrying */
      delay(3000);
    }
  }
}


bool publishUID(const char* uid) {
  if (mqttClient.publish(MQTT_PUB_TOPIC, uid)) {
    Serial.print("UID successfully published on topic [");
    Serial.print(MQTT_PUB_TOPIC);
    Serial.print("]: ");
    Serial.println(uid);

    // Wait for confirmation
    if(waitForConfirmation()) {
      return true;
    }
    Serial.println("Failed to receive confirmation");
    return false;
  }
  else {
    Serial.println("Failed to publish UID");
    return false;
  }
}

bool waitForConfirmation() {
  // Wait for 3 seconds to receive reply in subTopic
  time_t start = millis();
  while (millis() - start < 3000) {
    mqttClient.loop();
    if (messageReceived) {
      messageReceived = false;
      return true;
    }
  }
  return false;
}


void setup() {
  Serial.setDebugOutput(true);
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);
  SPI.begin();
  mfrc522.PCD_Init();

  connectWiFi();
  
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectMqttBroker();

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connected to WiFi");
  lcd.setCursor(0, 1);
  lcd.print("IP: " + WiFi.localIP().toString());
  delay(2000);
  lcd.clear();

  // Subscribe to subTopic
  mqttClient.setCallback(callback);
  mqttClient.subscribe(MQTT_SUB_TOPIC);
  initializeText();
}


void initializeText(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SmarTendance");
  lcd.setCursor(0, 1);
  lcd.print("Scan the card");
  delay(1000);
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to WiFi...");
    delay(2000);
    lcd.clear();
  }

  if (!mqttClient.connected() || !mqttClient.subscribe(MQTT_SUB_TOPIC)) {
    connectMqttBroker();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to MQTT...");
    delay(2000);
    lcd.clear();
  }
  else {
    mqttClient.loop();
  }
  
  String uid = readUID();
  if (uid.length() > 0) {
    bool success = publishUID(uid.c_str());

    if(!success) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Failed!");
      lcd.setCursor(0,1);
      lcd.print("Try again");
      delay(2000);
    }
  }
  initializeText();
}
