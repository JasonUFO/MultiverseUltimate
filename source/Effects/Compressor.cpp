#include "Compressor.h"
#include <cmath>

void CompressorEffect::prepare(double sr, int)
{
    sampleRate = sr;
    updateCoeffs();
    reset();
}

float CompressorEffect::process(float input)
{
    if (bypassed) return input;

    const float levelDB = 20.f * std::log10(std::abs(input) + 1e-9f);

    // Envelope follower (dB domain)
    if (levelDB > envelope)
        envelope = attackCoeff  * envelope + (1.f - attackCoeff)  * levelDB;
    else
        envelope = releaseCoeff * envelope + (1.f - releaseCoeff) * levelDB;

    // Gain computer: amount below threshold that we need to apply
    float gainDB = 0.f;
    if (envelope > threshold)
        gainDB = threshold + (envelope - threshold) / ratio - envelope;

    gainDB += makeup;

    return input * std::pow(10.f, gainDB / 20.f);
}

void CompressorEffect::reset()
{
    envelope = -144.f;
}

void CompressorEffect::setThreshold(float dB) { threshold = juce::jlimit(-60.f,   0.f, dB); }
void CompressorEffect::setRatio    (float r)   { ratio     = juce::jlimit(  1.f,  20.f,  r); }
void CompressorEffect::setAttack   (float ms)  { attackMs  = juce::jlimit(0.1f, 100.f, ms); updateCoeffs(); }
void CompressorEffect::setRelease  (float ms)  { releaseMs = juce::jlimit(10.f, 1000.f, ms); updateCoeffs(); }
void CompressorEffect::setMakeup   (float dB)  { makeup    = juce::jlimit(  0.f,  24.f, dB); }

void CompressorEffect::updateCoeffs()
{
    if (sampleRate <= 0.0) return;
    const float fs = static_cast<float>(sampleRate);
    attackCoeff  = std::exp(-1.f / (fs * attackMs  * 0.001f));
    releaseCoeff = std::exp(-1.f / (fs * releaseMs * 0.001f));
}
