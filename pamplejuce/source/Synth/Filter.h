#pragma once
#include <juce_dsp/juce_dsp.h>

class Filter
{
public:
    Filter();
    
    void setCutoff(float cutoff);
    void setResonance(float resonance);
    void setSampleRate(float sr);
    
    float process(float input);
    void reset();
    
    float getCutoff() const { return cutoff; }
    float getResonance() const { return resonance; }

private:
    juce::dsp::StateVariableTPTFilter<float> filter;
    float cutoff = 20000.0f;
    float resonance = 0.0f;
    float sampleRate = 44100.0f;
};