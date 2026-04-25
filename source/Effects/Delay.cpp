#include "Delay.h"

DelayEffect::DelayEffect() {}

void DelayEffect::prepare(double sr, int samplesPerBlock)
{
    sampleRate = (sr > 0.0 && sr <= 1000000.0) ? sr : 44100.0;

    // Must set max before prepare(); allow up to 2 seconds at the actual sample rate
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 2.0));

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    delayLine.prepare(spec);

    delayInSamples = static_cast<int>(sampleRate * time);
    delayLine.setDelay(static_cast<float>(delayInSamples));
    delayLine.reset();
}

float DelayEffect::process(float input)
{
    if (bypassed) return input;
    
    auto delayed = delayLine.popSample(0);
    delayLine.pushSample(0, input + delayed * feedback);
    
    return mix * delayed + (1.0f - mix) * input;
}

void DelayEffect::reset()
{
    delayLine.reset();
}

void DelayEffect::setTime(float seconds)
{
    time = juce::jlimit(0.0f, 1.0f, seconds);
    delayInSamples = static_cast<int>(sampleRate * time);
    delayLine.setDelay(static_cast<float>(delayInSamples));
}

void DelayEffect::setFeedback(float fb)
{
    feedback = juce::jlimit(0.0f, 0.95f, fb);
}

void DelayEffect::setMix(float m)
{
    mix = juce::jlimit(0.0f, 1.0f, m);
}