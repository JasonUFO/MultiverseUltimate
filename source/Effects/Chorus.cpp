#include "Chorus.h"
#include <cmath>

ChorusEffect::ChorusEffect() {}

void ChorusEffect::prepare(double sr, int)
{
    sampleRate = sr;
    reset();
}

float ChorusEffect::process(float input)
{
    if (bypassed) return input;

    buf[writePos] = input;

    // LFO modulates delay: 1 ms centre + depth * 10 ms swing
    const float lfoVal   = std::sin(phase);
    const float delaySmp = static_cast<float>(sampleRate) * 0.001f
                         + depth * static_cast<float>(sampleRate) * 0.010f
                           * (lfoVal * 0.5f + 0.5f);

    const int   d0   = static_cast<int>(delaySmp);
    const float frac = delaySmp - static_cast<float>(d0);
    int r0 = writePos - d0;     if (r0 < 0) r0 += kBufSize;
    int r1 = writePos - d0 - 1; if (r1 < 0) r1 += kBufSize;
    const float delayed = buf[r0] * (1.0f - frac) + buf[r1] * frac;

    writePos = (writePos + 1) % kBufSize;
    phase   += juce::MathConstants<float>::twoPi * rate
               / static_cast<float>(juce::jmax(sampleRate, 1.0));
    if (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    return mix * delayed + (1.0f - mix) * input;
}

void ChorusEffect::reset()
{
    std::fill(std::begin(buf), std::end(buf), 0.0f);
    writePos = 0;
    phase    = 0.0f;
}

void ChorusEffect::setRate (float hz) { rate  = juce::jlimit(0.1f, 5.0f, hz); }
void ChorusEffect::setDepth(float d)  { depth = juce::jlimit(0.0f, 1.0f, d); }
void ChorusEffect::setMix  (float m)  { mix   = juce::jlimit(0.0f, 1.0f, m); }
