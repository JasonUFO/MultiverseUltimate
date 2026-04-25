#pragma once
#include "Effect.h"
#include <juce_audio_basics/juce_audio_basics.h>

class ReverbEffect : public Effect
{
public:
    ReverbEffect();
    
    void prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void reset() override;
    
    void setRoomSize(float size);
    void setDamping(float damping);
    void setWetLevel(float wet);
    void setDryLevel(float dry);
    
private:
    juce::Reverb reverb;
    juce::Reverb::Parameters params;
};