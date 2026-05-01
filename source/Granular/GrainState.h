#pragma once

struct GrainState
{
    bool  active        = false;
    float readPos       = 0.0f;  // current read position in source buffer (samples)
    float readSpeed     = 1.0f;  // playback ratio (1 = original pitch)
    float durationSamples = 4096.0f; // total grain length in samples
    float phase         = 0.0f;  // 0..1 through the grain envelope
    float phaseInc      = 0.0f;  // phase increment per sample
    float panL          = 1.0f;
    float panR          = 1.0f;
    bool  reverse       = false;
};
