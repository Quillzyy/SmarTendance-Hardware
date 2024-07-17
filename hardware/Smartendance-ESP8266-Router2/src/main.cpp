#include <SoftwareSerial.h>

SoftwareSerial router1(D2, D1); // RX, TX

typedef struct struct_message
{
    char uid[32];
    char code[4];
} struct_message;
struct_message payloadRecv;

void setup()
{
    Serial.begin(115200);
    router1.begin(9600);
}

void loop()
{
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

        strncpy(payloadRecv.code, "100", sizeof(payloadRecv.code));

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
