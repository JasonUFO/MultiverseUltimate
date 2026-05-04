#include "SamplerVoice.h"
#include <cmath>

void MvSamplerVoice::prepare (double sr)
{
    sampleRate = sr > 0.0 ? sr : 44100.0;
    forceStop();
}

void MvSamplerVoice::noteOn (const SamplerZone* zone, int note, float vel)
{
    currentZone = zone;
    midiNote = note;
    velocity = juce::jlimit (0.0f, 1.0f, vel);
    readPos = 0.0;
    pingPongForward = true;
    active = true;

    double pitchRatio = std::pow (2.0, (midiNote - zone->rootNote + (double)zone->tuning) / 12.0);
    playbackRate = pitchRatio * (zone->fileSampleRate / sampleRate) * (double)zone->speed;

    sustainLevel = 0.7f;
    attackInc  = 1.0f / static_cast<float> (0.005  * sampleRate);
    decayDec   = (1.0f - sustainLevel) / static_cast<float> (0.1   * sampleRate);
    releaseDec = sustainLevel / static_cast<float> (0.3   * sampleRate);

    envStage = EnvStage::Attack;
    envValue = 0.0f;
}

void MvSamplerVoice::noteOff()
{
    if (active && envStage != EnvStage::Idle)
        envStage = EnvStage::Release;
}

void MvSamplerVoice::forceStop()
{
    active = false;
    currentZone = nullptr;
    midiNote = -1;
    envStage = EnvStage::Idle;
    envValue = 0.0f;
}

float MvSamplerVoice::getSampleAt (double pos) const noexcept
{
    const auto& buf = currentZone->audioData;
    int numSamples = buf.getNumSamples();
    if (numSamples == 0) return 0.0f;

    int i0 = static_cast<int> (pos);
    int i1 = i0 + 1;
    float frac = static_cast<float> (pos - i0);

    i0 = juce::jlimit (0, numSamples - 1, i0);
    i1 = juce::jlimit (0, numSamples - 1, i1);

    float out = 0.0f;
    int numChannels = buf.getNumChannels();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* data = buf.getReadPointer (ch);
        out += data[i0] + frac * (data[i1] - data[i0]);
    }
    return out / static_cast<float> (numChannels);
}

float MvSamplerVoice::process()
{
    if (!active || currentZone == nullptr)
        return 0.0f;

    const int totalSamples = currentZone->audioData.getNumSamples();
    if (totalSamples == 0) { active = false; return 0.0f; }

    const int effectiveLoopEnd   = currentZone->getEffectiveLoopEnd();
    const int loopStart          = currentZone->loopStart;
    const int xfadeLen           = currentZone->crossfadeLength;
    const LoopMode loopMode      = currentZone->loopMode;
    const bool hasLoop           = loopMode != LoopMode::None && effectiveLoopEnd > loopStart + 1;

    // --- Read sample with optional crossfade blend ---
    float sample;
    if (hasLoop && xfadeLen > 0 && effectiveLoopEnd - xfadeLen > loopStart)
    {
        double xfadeStart = static_cast<double> (effectiveLoopEnd - xfadeLen);
        if (readPos >= xfadeStart)
        {
            float blend = static_cast<float> ((readPos - xfadeStart) / xfadeLen);
            float s1    = getSampleAt (readPos);
            float s2    = getSampleAt (loopStart + (readPos - xfadeStart));
            sample = s1 * (1.0f - blend) + s2 * blend;
        }
        else
        {
            sample = getSampleAt (readPos);
        }
    }
    else
    {
        sample = getSampleAt (readPos);
    }

    // --- Advance read position ---
    if (loopMode == LoopMode::PingPong && hasLoop)
    {
        readPos += pingPongForward ? playbackRate : -playbackRate;
        if (readPos >= effectiveLoopEnd)
        {
            readPos = 2.0 * effectiveLoopEnd - readPos;
            pingPongForward = false;
        }
        if (readPos <= loopStart)
        {
            readPos = 2.0 * loopStart - readPos;
            pingPongForward = true;
        }
    }
    else
    {
        readPos += playbackRate;
        if (hasLoop && readPos >= effectiveLoopEnd)
        {
            double loopLen = effectiveLoopEnd - loopStart;
            readPos = loopStart + std::fmod (readPos - loopStart, loopLen);
        }
        else if (readPos >= totalSamples)
        {
            active = false;
            return 0.0f;
        }
    }

    // --- ADSR envelope ---
    switch (envStage)
    {
        case EnvStage::Attack:
            envValue += attackInc;
            if (envValue >= 1.0f) { envValue = 1.0f; envStage = EnvStage::Decay; }
            break;
        case EnvStage::Decay:
            envValue -= decayDec;
            if (envValue <= sustainLevel) { envValue = sustainLevel; envStage = EnvStage::Sustain; }
            break;
        case EnvStage::Sustain:
            envValue = sustainLevel;
            break;
        case EnvStage::Release:
            envValue -= releaseDec;
            if (envValue <= 0.0f) { envValue = 0.0f; envStage = EnvStage::Idle; active = false; }
            break;
        case EnvStage::Idle:
            active = false;
            return 0.0f;
    }

    return sample * envValue * velocity;
}
