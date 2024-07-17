#!/bin/bash

echo "Creating folder for Arduino"
if [ ! -d "./arduino" ]; then
    mkdir ./arduino
fi

echo "Copying file for Arduino"
cp --remove-destination ./src/main.cpp ./arduino/arduino.ino

echo "Sedding"
sed -i 's/const uint8_t \*recv_info/const esp_now_recv_info_t \*recv_info/g' ./arduino/arduino.ino

echo "Complete"