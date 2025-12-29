#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Hardware Pins
#define BASE 0
#define TRIG 25
#define ECHO 26

// Audio / ML Settings
#define SAMPLE_RATE 16000
#define AUDIO_CHUNK_SIZE 512

// WiFi Credentials
extern const char* ssid;
extern const char* password;

// BLE UUIDs
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

#endif