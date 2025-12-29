#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include <Model.h>

void initMic();
void processAudioML();

// Functions used by the Edge Impulse signal_t (must be public)
int get_signal_data(size_t offset, size_t length, float *out_ptr);

#endif