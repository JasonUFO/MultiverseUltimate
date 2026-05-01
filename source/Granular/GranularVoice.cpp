#include "GranularVoice.h"

void GranularVoice::prepare (double sr)
{
    sampleRate = sr;
    for (auto& g : grains)
        g = GrainState{};
    active       = false;
    noteReleased = false;
    adsrStage    = ADSRStage::Idle;
    adsrLevel    = 0.0f;
    spawnTimer   = 0.0f;
}

void GranularVoice::noteOn (int midiNote, float vel)
{
    note         = midiNote;
    velocity     = vel;
    pitchRatio   = std::exp2f ((midiNote - 60) / 12.0f);
    active       = true;
    noteReleased = false;
    adsrStage    = ADSRStage::Attack;
    adsrLevel    = 0.0f;
    spawnTimer   = 0.0f;

    for (auto& g : grains)
        g.active = false;
}

void GranularVoice::noteOff ()
{
    noteReleased = true;
    if (adsrStage != ADSRStage::Idle)
        adsrStage = ADSRStage::Release;
}

int GranularVoice::findFreeGrain() const noexcept
{
    for (int i = 0; i < MAX_GRAINS; ++i)
        if (!grains[i].active)
            return i;
    // Voice steal: return oldest (first active)
    return 0;
}

void GranularVoice::spawnGrain (const float* srcL, const float* srcR, int srcLength,
                                 float position, float grainSizeSec, float spray,
                                 float pitchScatter, bool reverse, float stereoSpread)
{
    if (srcLength <= 0) return;
    int idx = findFreeGrain();
    GrainState& g = grains[idx];

    float positionOffset = (rng.nextFloat() * 2.0f - 1.0f) * spray;
    float centrePos = juce::jlimit(0.0f, 1.0f, position + positionOffset);
    g.readPos = centrePos * static_cast<float>(srcLength - 1);

    float scatter = (rng.nextFloat() * 2.0f - 1.0f) * pitchScatter;
    g.readSpeed   = pitchRatio * std::exp2f (scatter / 12.0f);
    if (reverse)
        g.readSpeed = -std::abs (g.readSpeed);

    g.durationSamples = grainSizeSec * static_cast<float>(sampleRate);
    g.phase    = 0.0f;
    g.phaseInc = (g.durationSamples > 0.0f) ? 1.0f / g.durationSamples : 1.0f;
    g.reverse  = reverse;

    float pan = (rng.nextFloat() * 2.0f - 1.0f) * stereoSpread;
    g.panL = juce::jlimit(0.0f, 1.0f, 1.0f - pan);
    g.panR = juce::jlimit(0.0f, 1.0f, 1.0f + pan);

    g.active = true;
    juce::ignoreUnused (srcR);
}

float GranularVoice::applyEnvelope (GrainEnvShape shape, float phase) const noexcept
{
    switch (shape)
    {
        case GrainEnvShape::Gaussian:
        {
            float x = phase - 0.5f;
            return std::exp (-8.0f * x * x);
        }
        case GrainEnvShape::Hann:
            return 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * phase));
        case GrainEnvShape::Trapezoid:
        {
            if (phase < 0.2f) return phase / 0.2f;
            if (phase > 0.8f) return (1.0f - phase) / 0.2f;
            return 1.0f;
        }
        case GrainEnvShape::Triangle:
            return 1.0f - std::abs (2.0f * phase - 1.0f);
        default:
            return 1.0f;
    }
}

void GranularVoice::updateADSRCoeffs (float attackSec, float decaySec,
                                       float sustainLvl, float releaseSec)
{
    auto sr = static_cast<float>(sampleRate);
    adsrAttackCoeff  = (attackSec  > 0.0f) ? 1.0f / (attackSec  * sr) : 1.0f;
    adsrDecayCoeff   = (decaySec   > 0.0f) ? 1.0f / (decaySec   * sr) : 1.0f;
    adsrReleaseCoeff = (releaseSec > 0.0f) ? 1.0f / (releaseSec * sr) : 1.0f;
    adsrSustainLevel = sustainLvl;
}

void GranularVoice::processBlock (float* outL, float* outR, int numSamples,
                                   const float* srcL, const float* srcR, int srcLength,
                                   float position, float grainSizeSec,
                                   float spray, float density,
                                   float pitchScatter, GrainEnvShape envShape,
                                   bool reverse, float stereoSpread,
                                   float attackSec, float decaySec,
                                   float sustainLevel, float releaseSec)
{
    if (!active) return;

    updateADSRCoeffs (attackSec, decaySec, sustainLevel, releaseSec);

    const float spawnInterval = (density > 0.0f)
        ? static_cast<float>(sampleRate) / density
        : static_cast<float>(sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        // Advance ADSR
        switch (adsrStage)
        {
            case ADSRStage::Attack:
                adsrLevel += adsrAttackCoeff;
                if (adsrLevel >= 1.0f) { adsrLevel = 1.0f; adsrStage = ADSRStage::Decay; }
                break;
            case ADSRStage::Decay:
                adsrLevel -= adsrDecayCoeff * (1.0f - adsrSustainLevel);
                if (adsrLevel <= adsrSustainLevel) { adsrLevel = adsrSustainLevel; adsrStage = ADSRStage::Sustain; }
                break;
            case ADSRStage::Sustain:
                adsrLevel = adsrSustainLevel;
                break;
            case ADSRStage::Release:
                adsrLevel -= adsrReleaseCoeff * adsrLevel;
                if (adsrLevel < 0.0001f)
                {
                    adsrLevel = 0.0f;
                    adsrStage = ADSRStage::Idle;
                    active = false;
                    return;
                }
                break;
            case ADSRStage::Idle:
                active = false;
                return;
        }

        // Spawn new grain if timer elapsed
        spawnTimer -= 1.0f;
        if (spawnTimer <= 0.0f)
        {
            spawnGrain (srcL, srcR, srcLength, position, grainSizeSec,
                        spray, pitchScatter, reverse, stereoSpread);
            spawnTimer += spawnInterval;
        }

        // Process all active grains
        float sumL = 0.0f, sumR = 0.0f;
        for (auto& g : grains)
        {
            if (!g.active) continue;

            // Interpolate source sample
            int   pos0 = static_cast<int>(g.readPos);
            float frac = g.readPos - static_cast<float>(pos0);
            int   pos1 = pos0 + 1;

            pos0 = juce::jlimit (0, srcLength - 1, pos0);
            pos1 = juce::jlimit (0, srcLength - 1, pos1);

            float sampL = srcL[pos0] + frac * (srcL[pos1] - srcL[pos0]);
            float sampR = (srcR != nullptr) ? (srcR[pos0] + frac * (srcR[pos1] - srcR[pos0])) : sampL;

            float env = applyEnvelope (envShape, g.phase);
            sumL += sampL * env * g.panL;
            sumR += sampR * env * g.panR;

            // Advance grain
            g.readPos += g.readSpeed;
            g.phase   += g.phaseInc;

            if (g.phase >= 1.0f || g.readPos < 0.0f || g.readPos >= static_cast<float>(srcLength))
                g.active = false;
        }

        outL[i] += sumL * adsrLevel * velocity;
        outR[i] += sumR * adsrLevel * velocity;
    }
}
