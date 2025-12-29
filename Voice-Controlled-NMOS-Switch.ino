#include "Config.h"
#include "Hardware.h"
#include "Connectivity.h"
#include "ML_Inference.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize each module
    initHardware();   // From Hardware.cpp
    initWiFi();       // From Connectivity.cpp
    initBLE();        // From Connectivity.cpp
    initMic();        // From ML_Inference.cpp
    
    Serial.println("System Ready.");
}

void loop() {
    // 1. Handle Web Requests
    server.handleClient();
    
    // 2. Check Physical Sensors
    checkUltrasonic();
    
    // 3. Process Voice AI
    processAudioML();
    
    delay(1);
}