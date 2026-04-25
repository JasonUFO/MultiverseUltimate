#include "Envelope.h"
#include <cmath>

Envelope::Envelope() : stage(Stage::Idle), value(0.0f)
{
    recomputeIncrements();
}

void Envelope::setSampleRate(float sr)
{
    sampleRate = (sr > 0.0f && sr <= 1000000.0f) ? sr : 44100.0f;
    recomputeIncrements();
}

void Envelope::setParameters(float a, float d, float s, float r)
{
    attack  = juce::jlimit(0.001f, 10.0f, a);
    decay   = juce::jlimit(0.001f, 10.0f, d);
    sustain = juce::jlimit(0.0f,   1.0f,  s);
    release = juce::jlimit(0.001f, 10.0f, r);
    recomputeIncrements();
}

void Envelope::recomputeIncrements()
{
    attackInc  = 1.0f / (attack  * sampleRate);
    decayInc   = (1.0f - sustain) / juce::jmax(1.0f, decay * sampleRate);
    releaseInc = juce::jmax(sustain, 0.001f) / juce::jmax(1.0f, release * sampleRate);
}

void Envelope::triggerAttack()
{
    stage = Stage::Attack;
}

void Envelope::triggerRelease()
{
    stage = Stage::Release;
}

bool Envelope::isActive() const
{
    return stage != Stage::Idle;
}

bool Envelope::isFinished() const
{
    return stage == Stage::Idle && value < 0.001f;
}

float Envelope::process(float input)
{
    switch (stage)
    {
        case Stage::Attack:
            value += attackInc;
            if (value >= 1.0f)
            {
                value = 1.0f;
                stage = Stage::Decay;
            }
            break;

        case Stage::Decay:
            value -= decayInc;
            if (value <= sustain)
            {
                value = sustain;
                stage = Stage::Sustain;
            }
            break;

        case Stage::Sustain:
            value = sustain;
            break;

        case Stage::Release:
            value -= releaseInc;
            if (value <= 0.0f)
            {
                value = 0.0f;
                stage = Stage::Idle;
            }
            break;
            
        case Stage::Idle:
            value = 0.0f;
            break;
    }
    
    return input * value;
}