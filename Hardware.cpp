#include "Config.h"

String state = "OFF";
unsigned long g_last_buzz_ms = 0;

void setLed(int s) {
    int w = M5.Display.width();
    int h = M5.Display.height();
    uint16_t color = (s == 1) ? TFT_GREEN : TFT_RED;
    
    M5.Display.fillScreen(color);
    M5.Display.setTextColor(TFT_BLACK, color);
    M5.Display.setCursor(w/2 - 30, h/2 - 10);
    M5.Display.print(s == 1 ? "ON" : "OFF");
}

void buzzState(int inp_state) {
    bool mic_was_enabled = M5.Mic.isEnabled();
    if (mic_was_enabled) M5.Mic.end();

    M5.Speaker.begin();
    M5.Speaker.setVolume(64);  
    if (inp_state == 1) {
        M5.Speaker.tone(2000, 80); delay(100);
    } else {
        M5.Speaker.tone(1500, 60); delay(120);
        M5.Speaker.tone(1500, 60); delay(80);
    }
    M5.Speaker.stop();
    M5.Speaker.end();

    if (mic_was_enabled) M5.Mic.begin();
    g_last_buzz_ms = millis();
}

void setState(int inp_state) {
    digitalWrite(BASE, inp_state);
    setLed(inp_state);
    state = (inp_state == 1) ? "ON" : "OFF";
    buzzState(inp_state);
}

void initHardware() {
    auto cfg = M5.config(); 
    M5.begin(cfg);
    M5.Display.setTextSize(3);
    pinMode(BASE, OUTPUT);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
}