#include "Distortion.h"
#include <cmath>

void DistortionEffect::prepare(double sr, int)
{
    sampleRate = sr;
    updateLPCoeff();
    reset();
}

float DistortionEffect::process(float input)
{
    if (bypassed) return input;

    // Soft-clip: tanh(drive * input), scaled back to ~[-1,1]
    const float driven = std::tanh(drive * input * 0.1f);

    // Tone: one-pole LP  (tone=0 → dark, tone=1 → bright)
    lpState = lpCoeff * lpState + (1.0f - lpCoeff) * driven;

    return mix * lpState + (1.0f - mix) * input;
}

void DistortionEffect::reset()
{
    lpState = 0.0f;
}

void DistortionEffect::setDrive(float d) { drive = juce::jlimit(1.0f, 100.0f, d); }
void DistortionEffect::setMix  (float m) { mix   = juce::jlimit(0.0f,   1.0f, m); }

void DistortionEffect::setTone(float t)
{
    tone = juce::jlimit(0.0f, 1.0f, t);
    updateLPCoeff();
}

void DistortionEffect::updateLPCoeff()
{
    // Map tone [0,1] → cutoff [200, 8000] Hz
    const float hz = 200.0f * std::pow(40.0f, tone);
    const float dt = (sampleRate > 0.0) ? (1.0f / static_cast<float>(sampleRate)) : (1.0f / 44100.0f);
    const float rc = 1.0f / (juce::MathConstants<float>::twoPi * hz);
    lpCoeff = rc / (rc + dt);
}
