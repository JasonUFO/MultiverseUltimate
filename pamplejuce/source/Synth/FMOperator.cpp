#include "FMOperator.h"
#include <juce_core/juce_core.h>
#include <cmath>

static constexpr double TWO_PI = 6.283185307179586;

void FMOperator::setSampleRate(double sr)
{
    sampleRate = sr > 0.0 ? sr : 44100.0;
}

void FMOperator::noteOn(float baseFrequency)
{
    phaseInc = (static_cast<double>(ratio) * baseFrequency) / sampleRate;
    phase = 0.0;
    feedbackSample = 0.0f;

    // Compute per-sample envelope increments from time parameters
    attackInc = 1.0f / juce::jmax(1.0f, static_cast<float>(attackTime * sampleRate));
    decayInc = (1.0f - sustainLevel) / juce::jmax(1.0f, static_cast<float>(decayTime * sampleRate));
    releaseInc = juce::jmax(sustainLevel, 0.001f) / juce::jmax(1.0f, static_cast<float>(releaseTime * sampleRate));

    envStage = EnvStage::Attack;
    envValue = 0.0f;
}

void FMOperator::noteOff()
{
    if (envStage != EnvStage::Idle)
        envStage = EnvStage::Release;
}

void FMOperator::reset()
{
    phase = 0.0;
    feedbackSample = 0.0f;
    envStage = EnvStage::Idle;
    envValue = 0.0f;
}

float FMOperator::process(float modulationPhase)
{
    // Self-feedback: previous output feeds back into phase (DX7 style)
    float selfFb = feedbackSample * feedback * static_cast<float>(TWO_PI);
    float samplePhase = static_cast<float>(phase * TWO_PI) + modulationPhase + selfFb;
    float sample = std::sin(samplePhase);

    phase += phaseInc;
    if (phase >= 1.0)
        phase -= 1.0;

    feedbackSample = sample;

    return processEnvelope() * sample;
}

bool FMOperator::isActive() const
{
    return envStage != EnvStage::Idle;
}

float FMOperator::processEnvelope()
{
    switch (envStage)
    {
        case EnvStage::Attack:
            envValue += attackInc;
            if (envValue >= 1.0f)
            {
                envValue = 1.0f;
                envStage = EnvStage::Decay;
            }
            break;

        case EnvStage::Decay:
            envValue -= decayInc;
            if (envValue <= sustainLevel)
            {
                envValue = sustainLevel;
                envStage = EnvStage::Sustain;
            }
            break;

        case EnvStage::Sustain:
            envValue = sustainLevel;
            break;

        case EnvStage::Release:
            envValue -= releaseInc;
            if (envValue <= 0.0f)
            {
                envValue = 0.0f;
                envStage = EnvStage::Idle;
            }
            break;

        case EnvStage::Idle:
            envValue = 0.0f;
            break;
    }

    return envValue;
}
