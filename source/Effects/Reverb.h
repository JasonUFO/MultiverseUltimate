#pragma once
#include "Effect.h"
#include <JuceHeader.h>

class ReverbEffect : public Effect
{
public:
    ReverbEffect();

    void prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void  processBlock(float* left, float* right, int numSamples);
    void  reset() override;

    void setRoomSize(float size);
    void setDamping(float damping);     // HF damping
    void setWetLevel(float wet);
    void setDryLevel(float dry);
    void setPreDelay(float ms);         // 0–200 ms
    void setLFDamping(float amount);    // 0–1
    void setWidth(float width);         // 0–1
    void setFreeze(bool freeze);

    float getRoomSize()   const { return params.roomSize; }
    float getDamping()    const { return params.damping; }
    float getWetLevel()   const { return params.wetLevel; }
    float getDryLevel()   const { return dryLevel; }
    float getPreDelay()   const { return preDelayMs; }
    float getLFDamping()  const { return lfDampAmount; }
    float getWidth()      const { return params.width; }
    bool  getFreeze()     const { return params.freezeMode; }

private:
    juce::Reverb reverb;
    juce::Reverb::Parameters params;

    float dryLevel     = 0.6f;
    float preDelayMs   = 0.0f;
    float lfDampAmount = 0.0f;

    // Pre-delay circular buffers (pre-allocated, no RT allocation)
    std::vector<float> preDelayBufL, preDelayBufR;
    int preDelayWritePos = 0;
    int preDelaySamples  = 0;

    // Work buffers for saving dry signal in processBlock
    std::vector<float> workBufL, workBufR;

    // LF damping: one-pole highpass on wet signal only
    float hpA        = 1.0f;    // pole coefficient
    float hpPrevInL  = 0.0f, hpPrevInR  = 0.0f;
    float hpPrevOutL = 0.0f, hpPrevOutR = 0.0f;

    void updateHPCoeff();
    void applyPreDelay(float* left, float* right, int numSamples);
};
