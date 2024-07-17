#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>

// Router MAC Address
uint8_t routerAddress[] = {0x8C, 0xAA, 0xB5, 0xF8, 0x9B, 0x9C};

// Variable declarations
typedef struct struct_message
{
    char uid[32];
    char code[4];
} struct_message;

esp_now_peer_info_t peerInfo;

const byte SS_PIN = 5;
const byte RST_PIN = 0;
MFRC522 mfrc522(SS_PIN, RST_PIN);

const int I2C_SDA = 21;
const int I2C_SCL = 22;

LiquidCrystal_I2C lcd(0x27, 16, 2);

bool messageReceived = false;
struct_message payloadRecv;

// Function declarations
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status);
void onRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len);
void parseCode(String code, String uid);
String readUID();
bool waitForConfirmation();
void initializeText();

void setup()
{
    // Init Serial Monitor
    Serial.begin(115200);
    while (!Serial)
        continue;

    // Init I2C LCD
    Wire.begin(I2C_SDA, I2C_SCL);
    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Init MFRC522
    SPI.begin();
    mfrc522.PCD_Init();

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

    // Initialize LCD
    initializeText();
}

void loop()
{
    String uid = readUID();
    // uid = "1234567890"; // Dummy UID
    if (uid.length() > 0)
    {
        // Send UID to router
        struct_message payloadSend;
        strcpy(payloadSend.uid, uid.c_str());
        strcpy(payloadSend.code, "200");
        // strcpy(payloadSend.code, "100"); // Dummy 100

        // Serialize the data and give time in front
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

        // Wait for 3 seconds to receive reply
        time_t start = millis();
        while (millis() - start < 3000)
        {
            if (messageReceived)
            {
                messageReceived = false;
                parseCode(payloadRecv.code, payloadRecv.uid);
                break;
            }
            delay(1);
        }
        parseCode("201", uid);
        delay(2000);
        initializeText();
    }
    else
    {
        messageReceived = false;
    }
    delay(500);
}

void onSend(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    Serial.print(millis());
    Serial.print(" -> ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Send Success" : "Send Fail");
}

void onRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    messageReceived = true;

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

void parseCode(String code, String uid)
{
    if (code == "100")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("Successful!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - Successful!");
    }
    else if (code == "101")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("User not found!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - User not found!");
    }
    else if (code == "102")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("Course not found!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - Course not found!");
    }
    else if (code == "103")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("Already attended!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - Already attended!");
    }
    else if (code == "104")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("Role not found!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - Unknown error!");
    }
    else if (code == "201")
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Smartendance");
        lcd.setCursor(0, 1);
        lcd.print("Connection error!");
        Serial.print(millis());
        Serial.print(" -> ");
        Serial.print(uid);
        Serial.println(" - Connection error!");
    }
    delay(1000);
}

String readUID()
{
    String uid = "";
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
        return uid;

    for (byte i = 0; i < mfrc522.uid.size; i++)
        uid += String(mfrc522.uid.uidByte[i], HEX);

    mfrc522.PICC_HaltA();

    return uid;
}

void initializeText()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Smartendance");
    lcd.setCursor(0, 1);
    lcd.print("Scan the card!");
    delay(1000);
}
