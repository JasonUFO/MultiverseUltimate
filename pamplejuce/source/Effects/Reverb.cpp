#include "Reverb.h"

ReverbEffect::ReverbEffect()
{
    params.roomSize = 0.5f;
    params.damping = 0.5f;
    params.wetLevel = 0.3f;
    params.dryLevel = 0.6f;
    params.freezeMode = false;
}

void ReverbEffect::prepare(double sr, int)
{
    sampleRate = sr;
    reverb.setSampleRate(sampleRate);
    reverb.setParameters(params);
}

float ReverbEffect::process(float input)
{
    if (bypassed) return input;
    reverb.processMono(&input, 1);
    return input;
}

void ReverbEffect::reset()
{
    reverb.reset();
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
    params.dryLevel = juce::jlimit(0.0f, 1.0f, dry);
    reverb.setParameters(params);
}