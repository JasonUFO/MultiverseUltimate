#include "Filter.h"
#include <juce_core/juce_core.h>

Filter::Filter()
{
    filter.setType(juce::dsp::StateVariableTPTFilter<float>::Type::lowpass);
    filter.setCutoffFrequency(20000.0f);
    filter.setResonance(0.0f);
}

void Filter::setCutoff(float c)
{
    cutoff = juce::jlimit(20.0f, 20000.0f, c);
    filter.setCutoffFrequency(cutoff);
}

void Filter::setResonance(float r)
{
    resonance = juce::jlimit(0.0f, 1.0f, r);
    filter.setResonance(resonance);
}

void Filter::setSampleRate(float sr)
{
    sampleRate = sr;
    reset();
}

float Filter::process(float input)
{
    return filter.processSample(0, input);
}

void Filter::reset()
{
    filter.reset();
}