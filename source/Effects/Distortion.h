#pragma once
#include "Effect.h"
#include <JuceHeader.h>

class DistortionEffect : public Effect
{
public:
    void prepare(double sr, int) override;
    float process(float input) override;
    void reset() override;

    void setDrive(float drive); // 1 – 100
    void setTone (float tone);  // 0 – 1  (LP cutoff: 0=dark, 1=bright)
    void setMix  (float mix);   // 0 – 1

    float getDrive() const { return drive; }
    float getTone()  const { return tone; }
    float getMix()   const { return mix; }

private:
    float drive   = 10.0f;
    float tone    = 0.7f;
    float mix     = 0.5f;
    float lpState = 0.0f;
    float lpCoeff = 0.0f;

    void updateLPCoeff();
};
