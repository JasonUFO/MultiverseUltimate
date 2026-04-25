#pragma once
#include "Effect.h"
#include <JuceHeader.h>

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

    // Getters for state persistence
    float getRoomSize() const { return params.roomSize; }
    float getDamping() const { return params.damping; }
    float getWetLevel() const { return params.wetLevel; }
    float getDryLevel() const { return params.dryLevel; }
    
private:
    juce::Reverb reverb;
    juce::Reverb::Parameters params;
};