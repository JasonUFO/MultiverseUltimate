#pragma once
#include "Effect.h"
#include <juce_dsp/juce_dsp.h>

class DelayEffect : public Effect
{
public:
    DelayEffect();
    
    void prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void reset() override;
    
    void setTime(float seconds);
    void setFeedback(float feedback);
    void setMix(float mix);
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> delayLine;
    float time = 0.5f;
    float feedback = 0.3f;
    float mix = 0.5f;
    int delayInSamples = 0;
};