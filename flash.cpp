  #include <WiFi.h>
  #include <WebServer.h>
  #include <BLEDevice.h>
  #include <BLEUtils.h>
  #include <BLEServer.h>
  #include <M5Unified.h>
  #include <Rhm38-project-1_inferencing.h>  
  #include <math.h> 

  // constants
  #define BASE 0
  #define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
  #define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
  #define TRIG  25
  #define ECHO  26
  #define SAMPLE_RATE 16000
  #define AUDIO_CHUNK_SIZE 512 //

  // variables
  const char* ssid = "motog5g";
  const char* password = "ajaythumala";
  WebServer server(80);
  String state = "OFF";
  BLECharacteristic *pCharacteristic_RX;
  static int16_t sampleBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
  static int16_t mic_chunk[AUDIO_CHUNK_SIZE]; //
  static bool g_reset_audio_buffer = false;
  static unsigned long g_last_buzz_ms = 0;

  // function delcations if required
  void setLed(int s);
  void buzzState(int inp_state);

  void setState(int inp_state) {
    if (inp_state == 0) {
      digitalWrite(0, LOW);
      setLed(0);
      state = "OFF";
    } 
    if (inp_state == 1) {
      digitalWrite(0, HIGH); 
      setLed(1);
      state = "ON";
    }

    buzzState(inp_state);
    Serial.print("State: ");
    Serial.println(inp_state);
  }

  // LED
  void setLed(int s) {
    if (s == 0) {
      int w = M5.Display.width();
      int h = M5.Display.height();
      M5.Display.fillScreen(TFT_RED);
      M5.Display.setTextColor(TFT_BLACK, TFT_RED);
      M5.Display.setCursor(w/2 - 35, h/2 - 10);
      M5.Display.print("OFF");
    }
    if (s == 1) {
      int w = M5.Display.width();
      int h = M5.Display.height();
      M5.Display.fillScreen(TFT_GREEN);
      M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
      M5.Display.setCursor(w/2 - 30, h/2 - 10);
      M5.Display.print("ON");
    }
  }

  // Bluetooth 
  class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
      std::string value = pChar->getValue();
      if (value.length() > 0) {
        if (value == "ON") { setState(1); }
        if (value == "OFF") { setState(0); }
        Serial.print("Received via BLE: ");
        Serial.println(value.c_str());
      }
    }
  };

  // Ultrasonic
  long readDistanceCM() {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    long duration = pulseIn(ECHO, HIGH, 20000); 
    long distance = duration / 58;  
    return distance;
  }

  // ML voice detection
  float buffer_rms_db(const int16_t* data, size_t len) {
      if (len == 0) return -120.0f;

      double sum_sq = 0.0;
      for (size_t i = 0; i < len; i++) {
          float s = (float)data[i];
          sum_sq += s * s;
      }

      double mean_sq = sum_sq / (double)len;
      double rms = sqrt(mean_sq);

      double norm = rms / 32768.0;
      if (norm <= 1e-9) return -120.0f;   

      float db = 20.0f * log10f((float)norm);
      return db;  
  }

  bool captureAudio() {
      static size_t sample_index = 0;

      if (g_reset_audio_buffer) {
          sample_index = 0;
          g_reset_audio_buffer = false;
      }

      if (!M5.Mic.isEnabled()) {
          return false;
      }

      if (M5.Mic.record(mic_chunk, AUDIO_CHUNK_SIZE, SAMPLE_RATE)) {
          for (size_t i = 0; i < AUDIO_CHUNK_SIZE && sample_index < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
              sampleBuffer[sample_index++] = mic_chunk[i];
          }

          if (sample_index >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
              sample_index = 0;
              return true;
          }
      }

      return false;
  }

  int get_signal_data(size_t offset, size_t length, float *out_ptr) {
      for (size_t i = 0; i < length; i++) {
          out_ptr[i] = (float)sampleBuffer[offset + i] / 32768.0f;
      }
      return 0;
  }

  void displayResults(ei_impulse_result_t result) {    
      float max_confidence = 0;
      int max_index = -1;
      for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
          if (result.classification[i].value > max_confidence) {
              max_confidence = result.classification[i].value;
              max_index = i;
          }
      }
      if (max_confidence > 0.6) {        
          String label = result.classification[max_index].label;
          
          if (label == "on") {
              setState(1);
              // M5.Display.setTextColor(GREEN);
              // M5.Display.println("ON");
          } else if (label == "off") {
              setState(0);
              // M5.Display.setTextColor(RED);
              // M5.Display.println("OFF");
          } else {
              Serial.print("unknown label: ");
              Serial.println(label);
          }
      } else {
          Serial.print("...");
      }
  }

  void buzzState(int inp_state) {
      bool mic_was_enabled = M5.Mic.isEnabled();
      if (mic_was_enabled) {
          M5.Mic.end();
      }

      auto spk_cfg = M5.Speaker.config();
      M5.Speaker.config(spk_cfg);
      M5.Speaker.begin();
      M5.Speaker.setVolume(64);   

      if (inp_state == 1) {
          // single short beep
          M5.Speaker.tone(2000, 80);   
          delay(100);
      } else if (inp_state == 0) {
          M5.Speaker.tone(1500, 60);
          delay(120);
          M5.Speaker.tone(1500, 60);
          delay(80);
      }

      M5.Speaker.stop();
      M5.Speaker.end();

      if (mic_was_enabled) {
          M5.Mic.begin();
      }

      g_last_buzz_ms = millis();
      g_reset_audio_buffer = true;
  }

  void setup() {
    Serial.begin(115200);
    
    // LED / Display
    auto cfg = M5.config(); 
    M5.begin(cfg);
    M5.Display.setTextSize(3);

    // Switch
    pinMode(BASE, OUTPUT);
    // Ultrasonic Sensor
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    
    // Webserver
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(200);
    Serial.println(WiFi.localIP());
    server.on("/on", [](){ setState(1); server.send(200,"text/plain","OK"); });
    server.on("/off", [](){ setState(0); server.send(200,"text/plain","OK"); });
    server.begin();

    // Bluetooth
    BLEDevice::init("M5StickC_BLE");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic_RX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    pCharacteristic_RX->setCallbacks(new MyCallbacks());
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
    Serial.println("BLE UART server started, waiting for writes...");

    M5.Speaker.end();
    auto mic_cfg = M5.Mic.config();
    mic_cfg.sample_rate = SAMPLE_RATE;   // 16000
    // mic_cfg.over_sampling    = 4;          
    // mic_cfg.noise_filter_level = 64;        
    // mic_cfg.magnification    = 8;        
    M5.Mic.config(mic_cfg);
    M5.Mic.begin();
  }

  void loop() {
    server.handleClient();

    // ultrasonic
    long d = readDistanceCM();
    if (d > 0 && d < 5) {
      Serial.println("hand detected");
      if (state == "OFF") setState(1);
      else if (state == "ON") setState(0);
      else Serial.println("Error state is neither ON nor OFF");
    }

    // ML voice detection
    M5.update();
    if (captureAudio()) {
          // Cooldown: skip frames that happen right after a buzz
          const unsigned long COOLDOWN_MS = 300;   
          if (millis() - g_last_buzz_ms < COOLDOWN_MS) {
              Serial.println("Skipped frame during post-buzz cooldown");
              return;
          }

          float db = buffer_rms_db(sampleBuffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
          const float THRESHOLD_DB = -35.0f;

          if (db < THRESHOLD_DB) {
              Serial.printf("Skipped quiet frame: %.1f dBFS\n", db);
          } else {
              Serial.printf("Classifying frame: %.1f dBFS\n", db);

              signal_t signal;
              signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
              signal.get_data = &get_signal_data;
              ei_impulse_result_t result = {0};
              EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
              if (res != EI_IMPULSE_OK) {
                  Serial.printf("Error: %d\n", res);
                  return;
              }
              displayResults(result);
          }
    }

    delay(1);
  }
