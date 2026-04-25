#include "FMVoice.h"
#include <cmath>

void FMVoice::setSampleRate(double sr)
{
    for (auto& op : operators)
        op.setSampleRate(sr);
}

void FMVoice::setAlgorithm(int index)
{
    if (index >= 0 && index < FM_ALGORITHM_COUNT)
        algorithmIndex = index;
}

void FMVoice::setOperatorParams(int opIndex,
                                float ratio,
                                float level,
                                float feedback,
                                float attack,
                                float decay,
                                float sustain,
                                float release)
{
    if (opIndex < 0 || opIndex >= FM_OP_COUNT)
        return;

    auto& op = operators[static_cast<size_t>(opIndex)];
    op.ratio = ratio;
    op.level = level;
    op.feedback = feedback;
    op.attackTime = attack;
    op.decayTime = decay;
    op.sustainLevel = sustain;
    op.releaseTime = release;
}

void FMVoice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    float freq = 440.0f * std::pow(2.0f, static_cast<float>(note - 69) / 12.0f);
    for (auto& op : operators)
        op.noteOn(freq);
}

void FMVoice::noteOff()
{
    for (auto& op : operators)
        op.noteOff();
}

bool FMVoice::isActive() const
{
    // Active as long as any carrier is still running
    const auto& alg = FM_ALGORITHMS[algorithmIndex];
    for (size_t i = 0; i < FM_OP_COUNT; ++i)
    {
        if (alg.isCarrier[i] && operators[i].isActive())
            return true;
    }
    return false;
}

float FMVoice::process()
{
    const auto& alg = FM_ALGORITHMS[algorithmIndex];
    float opOutput[FM_OP_COUNT] = {};

    // Process operators from 3 down to 0: modulators are always higher-indexed
    for (int i = static_cast<int>(FM_OP_COUNT) - 1; i >= 0; --i)
    {
        auto idx = static_cast<size_t>(i);
        float modPhase = 0.0f;
        int mask = alg.modulatedBy[idx];
        for (int m = 0; m < static_cast<int>(FM_OP_COUNT); ++m)
        {
            if (mask & (1 << m))
            {
                auto midx = static_cast<size_t>(m);
                modPhase += opOutput[midx] * operators[midx].level;
            }
        }
        opOutput[idx] = operators[idx].process(modPhase);
    }

    // Sum carrier outputs and normalise
    float output = 0.0f;
    int carrierCount = 0;
    for (size_t i = 0; i < FM_OP_COUNT; ++i)
    {
        if (alg.isCarrier[i])
        {
            output += opOutput[i] * operators[i].level;
            ++carrierCount;
        }
    }

    if (carrierCount > 1)
        output /= static_cast<float>(carrierCount);

    return output * velocity;
}
