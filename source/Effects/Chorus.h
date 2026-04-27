#pragma once
#include "Effect.h"
#include <JuceHeader.h>

class ChorusEffect : public Effect
{
public:
    ChorusEffect();
    void  prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void  reset() override;

    void  setRate (float hz);    // 0.1 – 5.0 Hz
    void  setDepth(float depth); // 0 – 1
    void  setMix  (float mix);   // 0 – 1

    float getRate()  const { return rate; }
    float getDepth() const { return depth; }
    float getMix()   const { return mix; }

private:
    static constexpr int kBufSize = 8192; // ~186 ms at 44.1 kHz
    float buf[kBufSize] {};
    int   writePos = 0;
    float phase    = 0.0f;

    float rate  = 0.5f;
    float depth = 0.5f;
    float mix   = 0.5f;
};
