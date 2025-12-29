#include "Config.h"
#include <math.h>

int16_t sampleBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static int16_t mic_chunk[AUDIO_CHUNK_SIZE];
bool g_reset_audio_buffer = false;

// ... [Insert captureAudio(), buffer_rms_db(), and get_signal_data() here] ...

void processAudioML() {
    M5.update();
    if (captureAudio()) {
        if (millis() - g_last_buzz_ms < 300) return;

        float db = buffer_rms_db(sampleBuffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
        if (db > -35.0f) {
            signal_t signal;
            signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
            signal.get_data = &get_signal_data;
            ei_impulse_result_t result = {0};
            if (run_classifier(&signal, &result, false) == EI_IMPULSE_OK) {
                // Logic to call setState based on labels
            }
        }
    }
}