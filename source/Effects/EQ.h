#pragma once
#include "Effect.h"
#include <JuceHeader.h>

// 3-band EQ: low shelf @ 250 Hz, peak @ 1 kHz, high shelf @ 4 kHz.
// All gain ranges: -12 to +12 dB.  Biquad coefficients from Audio EQ Cookbook.
class EQEffect : public Effect
{
public:
    void prepare(double sr, int) override;
    float process(float input) override;
    void reset() override;

    void setLowGain (float dB);
    void setMidGain (float dB);
    void setHighGain(float dB);

    float getLowGain()  const { return lowGain; }
    float getMidGain()  const { return midGain; }
    float getHighGain() const { return highGain; }

private:
    struct Biquad
    {
        float b0 = 1.f, b1 = 0.f, b2 = 0.f, a1 = 0.f, a2 = 0.f;
        float x1 = 0.f, x2 = 0.f, y1 = 0.f, y2 = 0.f;

        float process(float x)
        {
            float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
            x2 = x1; x1 = x; y2 = y1; y1 = y;
            return y;
        }
        void reset() { x1 = x2 = y1 = y2 = 0.f; }
    };

    Biquad lowShelf, midPeak, highShelf;
    float lowGain  = 0.f;
    float midGain  = 0.f;
    float highGain = 0.f;

    void updateCoeffs();
    void makeLowShelf (Biquad& b, double sr, float fc, float gainDB);
    void makePeak     (Biquad& b, double sr, float fc, float Q, float gainDB);
    void makeHighShelf(Biquad& b, double sr, float fc, float gainDB);
};
