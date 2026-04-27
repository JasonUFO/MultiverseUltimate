#include "EQ.h"
#include <cmath>

void EQEffect::prepare(double sr, int)
{
    sampleRate = sr;
    updateCoeffs();
    reset();
}

float EQEffect::process(float input)
{
    if (bypassed) return input;
    float s = lowShelf.process(input);
    s       = midPeak .process(s);
    return    highShelf.process(s);
}

void EQEffect::reset()
{
    lowShelf .reset();
    midPeak  .reset();
    highShelf.reset();
}

void EQEffect::setLowGain (float dB) { lowGain  = juce::jlimit(-12.f, 12.f, dB); updateCoeffs(); }
void EQEffect::setMidGain (float dB) { midGain  = juce::jlimit(-12.f, 12.f, dB); updateCoeffs(); }
void EQEffect::setHighGain(float dB) { highGain = juce::jlimit(-12.f, 12.f, dB); updateCoeffs(); }

void EQEffect::updateCoeffs()
{
    if (sampleRate <= 0.0) return;
    makeLowShelf (lowShelf,  sampleRate, 250.0f,  lowGain);
    makePeak     (midPeak,   sampleRate, 1000.0f, 1.0f, midGain);
    makeHighShelf(highShelf, sampleRate, 4000.0f, highGain);
}

// Audio EQ Cookbook — low shelf, S=1 (shelf slope = 1)
void EQEffect::makeLowShelf(Biquad& bq, double sr, float fc, float gainDB)
{
    const float A  = std::pow(10.f, gainDB / 40.f);
    const float w0 = juce::MathConstants<float>::twoPi * fc / static_cast<float>(sr);
    const float cs = std::cos(w0);
    const float alpha = std::sin(w0) / std::sqrt(2.f);
    const float sq    = 2.f * std::sqrt(A) * alpha;

    const float a0 = (A+1.f) + (A-1.f)*cs + sq;
    bq.b0 =  A * ((A+1.f) - (A-1.f)*cs + sq) / a0;
    bq.b1 = 2.f*A * ((A-1.f) - (A+1.f)*cs)   / a0;
    bq.b2 =  A * ((A+1.f) - (A-1.f)*cs - sq) / a0;
    bq.a1 = -2.f * ((A-1.f) + (A+1.f)*cs)    / a0;
    bq.a2 =  ((A+1.f) + (A-1.f)*cs - sq)      / a0;
}

// Audio EQ Cookbook — peaking EQ
void EQEffect::makePeak(Biquad& bq, double sr, float fc, float Q, float gainDB)
{
    const float A     = std::pow(10.f, gainDB / 40.f);
    const float w0    = juce::MathConstants<float>::twoPi * fc / static_cast<float>(sr);
    const float cs    = std::cos(w0);
    const float alpha = std::sin(w0) / (2.f * Q);

    const float a0 = 1.f + alpha / A;
    bq.b0 = (1.f + alpha * A) / a0;
    bq.b1 = (-2.f * cs)       / a0;
    bq.b2 = (1.f - alpha * A) / a0;
    bq.a1 = (-2.f * cs)       / a0;
    bq.a2 = (1.f - alpha / A) / a0;
}

// Audio EQ Cookbook — high shelf, S=1
void EQEffect::makeHighShelf(Biquad& bq, double sr, float fc, float gainDB)
{
    const float A  = std::pow(10.f, gainDB / 40.f);
    const float w0 = juce::MathConstants<float>::twoPi * fc / static_cast<float>(sr);
    const float cs = std::cos(w0);
    const float alpha = std::sin(w0) / std::sqrt(2.f);
    const float sq    = 2.f * std::sqrt(A) * alpha;

    const float a0 = (A+1.f) - (A-1.f)*cs + sq;
    bq.b0 =  A * ((A+1.f) + (A-1.f)*cs + sq) / a0;
    bq.b1 = -2.f*A * ((A-1.f) + (A+1.f)*cs)  / a0;
    bq.b2 =  A * ((A+1.f) + (A-1.f)*cs - sq) / a0;
    bq.a1 =  2.f * ((A-1.f) - (A+1.f)*cs)    / a0;
    bq.a2 =  ((A+1.f) - (A-1.f)*cs - sq)      / a0;
}
