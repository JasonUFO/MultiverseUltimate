#include "Reverb.h"
#include <cmath>

static constexpr float kMaxPreDelayMs = 200.0f;

ReverbEffect::ReverbEffect()
{
    params.roomSize   = 0.5f;
    params.damping    = 0.5f;
    params.wetLevel   = 0.3f;
    params.dryLevel   = 0.0f;   // dry is mixed manually so LF damping only affects wet
    params.width      = 1.0f;
    params.freezeMode = false;
}

void ReverbEffect::prepare(double sr, int samplesPerBlock)
{
    sampleRate = sr;

    const int maxPreDelaySamples = static_cast<int>(sr * kMaxPreDelayMs / 1000.0) + 1;
    preDelayBufL.assign(maxPreDelaySamples, 0.0f);
    preDelayBufR.assign(maxPreDelaySamples, 0.0f);
    preDelayWritePos = 0;

    const int workSize = juce::jmax(samplesPerBlock, 1);
    workBufL.assign(workSize, 0.0f);
    workBufR.assign(workSize, 0.0f);

    hpPrevInL = hpPrevInR = 0.0f;
    hpPrevOutL = hpPrevOutR = 0.0f;
    updateHPCoeff();

    reverb.setSampleRate(sr);
    reverb.setParameters(params);
}

void ReverbEffect::reset()
{
    reverb.reset();
    std::fill(preDelayBufL.begin(), preDelayBufL.end(), 0.0f);
    std::fill(preDelayBufR.begin(), preDelayBufR.end(), 0.0f);
    preDelayWritePos = 0;
    hpPrevInL = hpPrevInR = 0.0f;
    hpPrevOutL = hpPrevOutR = 0.0f;
}

void ReverbEffect::updateHPCoeff()
{
    // Map lfDampAmount (0..1) to cutoff: 20 Hz → 300 Hz (exponential sweep)
    const float fc = 20.0f * std::pow(15.0f, lfDampAmount);
    const float w  = 2.0f * juce::MathConstants<float>::pi * fc
                     / static_cast<float>(juce::jmax(sampleRate, 1.0));
    hpA = std::exp(-w);
}

void ReverbEffect::applyPreDelay(float* left, float* right, int numSamples)
{
    if (preDelaySamples == 0) return;

    const int bufSize = static_cast<int>(preDelayBufL.size());
    for (int i = 0; i < numSamples; ++i)
    {
        int readPos = preDelayWritePos - preDelaySamples;
        if (readPos < 0) readPos += bufSize;

        const float inL = left[i];
        const float inR = right[i];
        left[i]  = preDelayBufL[readPos];
        right[i] = preDelayBufR[readPos];
        preDelayBufL[preDelayWritePos] = inL;
        preDelayBufR[preDelayWritePos] = inR;
        preDelayWritePos = (preDelayWritePos + 1) % bufSize;
    }
}

// Mono fallback (used when channel count == 1)
float ReverbEffect::process(float input)
{
    if (bypassed) return input;

    float wet = input;
    if (preDelaySamples > 0)
    {
        const int bufSize = static_cast<int>(preDelayBufL.size());
        int readPos = preDelayWritePos - preDelaySamples;
        if (readPos < 0) readPos += bufSize;
        wet = preDelayBufL[readPos];
        preDelayBufL[preDelayWritePos] = input;
        preDelayWritePos = (preDelayWritePos + 1) % bufSize;
    }

    reverb.processMono(&wet, 1);

    // LF damping on wet
    const float g       = (1.0f + hpA) * 0.5f;
    const float filtered = hpA * hpPrevOutL + g * wet - g * hpPrevInL;
    hpPrevInL  = wet;
    hpPrevOutL = filtered;

    return filtered + dryLevel * input;
}

void ReverbEffect::processBlock(float* left, float* right, int numSamples)
{
    if (bypassed) return;

    const int n = juce::jmin(numSamples, static_cast<int>(workBufL.size()));

    // Save dry signal before pre-delay
    std::copy(left,  left  + n, workBufL.data());
    std::copy(right, right + n, workBufR.data());

    // Apply pre-delay in-place (delayed signal feeds the reverb)
    applyPreDelay(left, right, n);

    // Stereo reverb — wet only (params.dryLevel = 0, width applied internally)
    reverb.processStereo(left, right, n);

    // LF damping on wet + add dry
    const float g = (1.0f + hpA) * 0.5f;
    for (int i = 0; i < n; ++i)
    {
        const float filtL = hpA * hpPrevOutL + g * left[i]  - g * hpPrevInL;
        hpPrevInL  = left[i];
        hpPrevOutL = filtL;
        left[i]    = filtL + dryLevel * workBufL[i];

        const float filtR = hpA * hpPrevOutR + g * right[i] - g * hpPrevInR;
        hpPrevInR  = right[i];
        hpPrevOutR = filtR;
        right[i]   = filtR + dryLevel * workBufR[i];
    }
}

void ReverbEffect::setRoomSize(float size)
{
    params.roomSize = juce::jlimit(0.0f, 1.0f, size);
    reverb.setParameters(params);
}

void ReverbEffect::setDamping(float damp)
{
    params.damping = juce::jlimit(0.0f, 1.0f, damp);
    reverb.setParameters(params);
}

void ReverbEffect::setWetLevel(float wet)
{
    params.wetLevel = juce::jlimit(0.0f, 1.0f, wet);
    reverb.setParameters(params);
}

void ReverbEffect::setDryLevel(float dry)
{
    dryLevel = juce::jlimit(0.0f, 1.0f, dry);
    // params.dryLevel stays 0; dry is mixed manually after LF damping
}

void ReverbEffect::setPreDelay(float ms)
{
    preDelayMs      = juce::jlimit(0.0f, kMaxPreDelayMs, ms);
    preDelaySamples = static_cast<int>(sampleRate * preDelayMs / 1000.0f);
    preDelaySamples = juce::jmin(preDelaySamples,
                                  static_cast<int>(preDelayBufL.size()) - 1);
}

void ReverbEffect::setLFDamping(float amount)
{
    lfDampAmount = juce::jlimit(0.0f, 1.0f, amount);
    updateHPCoeff();
}

void ReverbEffect::setWidth(float width)
{
    params.width = juce::jlimit(0.0f, 1.0f, width);
    reverb.setParameters(params);
}

void ReverbEffect::setFreeze(bool freeze)
{
    params.freezeMode = freeze;
    reverb.setParameters(params);
}
