#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Config.h"

// Forward declarations of functions from Hardware.cpp
void setState(int s);

WebServer server(80);
const char* ssid = "motog5g";
const char* password = "ajaythumala";

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        std::string value = pChar->getValue();
        if (value == "ON") setState(1);
        else if (value == "OFF") setState(0);
    }
};

void initWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(200);
    server.on("/on", [](){ setState(1); server.send(200,"text/plain","OK"); });
    server.on("/off", [](){ setState(0); server.send(200,"text/plain","OK"); });
    server.begin();
}

void initBLE() {
    BLEDevice::init("M5StickC_BLE");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pRX->setCallbacks(new MyCallbacks());
    pService->start();
    BLEDevice::getAdvertising()->start();
}