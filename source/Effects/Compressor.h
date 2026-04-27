#pragma once
#include "Effect.h"
#include <JuceHeader.h>

// Feed-forward compressor: dB-domain envelope follower, gain computer, makeup gain.
class CompressorEffect : public Effect
{
public:
    void prepare(double sr, int) override;
    float process(float input) override;
    void reset() override;

    void setThreshold(float dB);   // -60 to 0
    void setRatio    (float ratio); // 1 to 20
    void setAttack   (float ms);    // 0.1 to 100
    void setRelease  (float ms);    // 10 to 1000
    void setMakeup   (float dB);    // 0 to 24

    float getThreshold() const { return threshold; }
    float getRatio()     const { return ratio; }
    float getAttack()    const { return attackMs; }
    float getRelease()   const { return releaseMs; }
    float getMakeup()    const { return makeup; }

private:
    float threshold = -20.f;
    float ratio     =   4.f;
    float attackMs  =  10.f;
    float releaseMs = 100.f;
    float makeup    =   0.f;

    float envelope     = -144.f; // dB
    float attackCoeff  = 0.f;
    float releaseCoeff = 0.f;

    void updateCoeffs();
};
