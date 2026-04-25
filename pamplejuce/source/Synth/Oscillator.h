#pragma once
#include <juce_dsp/juce_dsp.h>

enum class WaveformType
{
    Sine,
    Saw,
    Square,
    Triangle,
    Noise,
    Sample
};

class Oscillator
{
public:
    Oscillator();
    void setFrequency(float freq);
    void setWaveform(WaveformType type);
    void setPulseWidth(float width);
    float process();
    void reset();
    void setSampleRate(float sr);

private:
    float frequency = 440.0f;
    float phase = 0.0f;
    float pulseWidth = 0.5f;
    WaveformType waveform = WaveformType::Sine;
    float sampleRate = 44100.0f;
};