#pragma once
#include <JuceHeader.h>

class Filter
{
public:
    enum class OversamplingMode { Off, X2, X4, Auto };

    Filter();

    void setCutoff(float cutoff);
    void setResonance(float resonance);
    void setSampleRate(float sr);
    void setOversamplingMode(OversamplingMode mode);

    float process(float input);
    void reset();

    float getCutoff() const { return cutoff; }
    float getResonance() const { return resonance; }

private:
    using SVFFilter = juce::dsp::StateVariableTPTFilter<float>;

    SVFFilter filter;    // 1x
    SVFFilter filter2x;  // 2x sample rate
    SVFFilter filter4x;  // 4x sample rate

    float cutoff = 20000.0f;
    float resonance = 0.707f;
    float sampleRate = 44100.0f;
    OversamplingMode osMode = OversamplingMode::Off;

    void prepareFilter(SVFFilter& f, double sr);
    float process2x(float input);
    float process4x(float input);
};