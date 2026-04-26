#pragma once
#include <JuceHeader.h>
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
    WaveformType getWaveform() const { return waveform; }

private:
    static float generateSample(WaveformType t, float phase, float pulseWidth);

    float frequency = 440.0f;
    float phase = 0.0f;
    float pulseWidth = 0.5f;
    WaveformType waveform = WaveformType::Sine;
    WaveformType prevWaveform = WaveformType::Sine;
    float crossfadeAlpha = 1.0f;
    static constexpr float crossfadeRate = 1.0f / 256.0f;
    float sampleRate = 44100.0f;
};