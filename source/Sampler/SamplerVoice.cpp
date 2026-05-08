#include "SamplerVoice.h"
#include <cmath>
#include <algorithm>

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
    double srRatio = zone->fileSampleRate / sampleRate;

    // Timestretch mode: pitch changes pitch only, speed changes tempo only
    timestretchActive = zone->timestretchEnabled && zone->loopMode != LoopMode::None;
    if (timestretchActive)
    {
        pitchRate = pitchRatio * srRatio;
        timeRate = (double)zone->speed;
        playbackRate = pitchRate; // read position advances at pitch rate only

        // WSOLA setup
        grainSize = static_cast<int> (0.03 * sampleRate); // 30ms grains
        if (grainSize < 64) grainSize = 64;
        hopSynthesis = grainSize / 4; // 75% overlap
        hopAnalysis = static_cast<int> (hopSynthesis * timeRate);

        // Pre-allocate buffers
        grainBuffer.resize (grainSize + WSOLA_SEARCH * 2, 0.0f);
        outputBuffer.resize (OUTPUT_RING_SIZE, 0.0f);
        hannWindow.resize (grainSize);
        for (int i = 0; i < grainSize; ++i)
            hannWindow[i] = 0.5f * (1.0f - std::cos (2.0f * 3.14159265f * i / grainSize));

        outputReadPos = 0;
        outputWritePos = 0;
        grainsReady = 0;
    }
    else
    {
        playbackRate = pitchRatio * srRatio * (double)zone->speed;
        pitchRate = playbackRate;
        timeRate = 1.0;
    }

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
    timestretchActive = false;
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

float MvSamplerVoice::processTimestretch()
{
    // If output ring has data, read from it
    if (grainsReady > 0)
    {
        float sample = outputBuffer[outputReadPos];
        outputReadPos = (outputReadPos + 1) % OUTPUT_RING_SIZE;
        --grainsReady;

        // Advance readPos by hopAnalysis worth of pitch-rate
        readPos += hopAnalysis * pitchRate / timeRate;

        // Handle looping
        const int effectiveLoopEnd = currentZone->getEffectiveLoopEnd();
        const int loopStart = currentZone->loopStart;
        if (readPos >= effectiveLoopEnd)
        {
            double loopLen = effectiveLoopEnd - loopStart;
            readPos = loopStart + std::fmod (readPos - loopStart, loopLen);
        }
        else if (readPos < loopStart)
        {
            readPos = loopStart;
        }

        return sample;
    }

    // Need to produce a new grain via WSOLA
    const int totalSamples = currentZone->audioData.getNumSamples();
    const int effectiveLoopEnd = currentZone->getEffectiveLoopEnd();
    const int loopStart = currentZone->loopStart;

    // Read grain from source at pitchRate
    std::fill (grainBuffer.begin(), grainBuffer.end(), 0.0f);

    int grainStart = static_cast<int> (readPos) - grainSize / 2;
    grainStart = juce::jmax (loopStart, juce::jmin (grainStart, effectiveLoopEnd - grainSize));

    for (int i = 0; i < grainSize; ++i)
    {
        double srcPos = (grainStart + i) / pitchRate * timeRate + (grainStart / pitchRate * timeRate - grainStart);
        // Simpler: just read at pitched position
        double pos = grainStart + i * 1.0;
        // Wrap within loop
        if (pos >= effectiveLoopEnd)
            pos = loopStart + std::fmod (pos - loopStart, (double)(effectiveLoopEnd - loopStart));
        if (pos < loopStart)
            pos = loopStart;

        grainBuffer[i] = getSampleAt (pos) * hannWindow[i];
    }

    // Cross-correlation to find best overlap position in output ring
    int bestOffset = 0;
    float bestCorr = -1e30f;

    int searchStart = -WSOLA_SEARCH;
    int searchEnd = WSOLA_SEARCH;
    int overlapLen = grainSize / 4;

    // Get existing output samples for correlation
    for (int offset = searchStart; offset <= searchEnd; ++offset)
    {
        int writePos = outputWritePos + offset;
        if (writePos < 0) writePos += OUTPUT_RING_SIZE;
        writePos = writePos % OUTPUT_RING_SIZE;

        float corr = 0.0f;
        for (int j = 0; j < overlapLen; ++j)
        {
            int outIdx = (writePos - overlapLen + j + OUTPUT_RING_SIZE) % OUTPUT_RING_SIZE;
            corr += outputBuffer[outIdx] * grainBuffer[j];
        }
        if (corr > bestCorr)
        {
            bestCorr = corr;
            bestOffset = offset;
        }
    }

    // Write grain into output ring with overlap-add at best position
    int writeStart = (outputWritePos + bestOffset) % OUTPUT_RING_SIZE;
    for (int i = 0; i < grainSize; ++i)
    {
        int idx = (writeStart + i) % OUTPUT_RING_SIZE;
        outputBuffer[idx] += grainBuffer[i];
    }

    outputWritePos = (writeStart + hopSynthesis) % OUTPUT_RING_SIZE;
    grainsReady += hopSynthesis;

    // Read one sample
    float sample = outputBuffer[outputReadPos];
    outputReadPos = (outputReadPos + 1) % OUTPUT_RING_SIZE;
    --grainsReady;

    return sample;
}

float MvSamplerVoice::process()
{
    if (!active || currentZone == nullptr)
        return 0.0f;

    const int totalSamples = currentZone->audioData.getNumSamples();
    if (totalSamples == 0) { active = false; return 0.0f; }

    // --- Timestretch path ---
    if (timestretchActive)
    {
        float sample = processTimestretch();

        // ADSR envelope
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

    // --- Normal varispeed path ---
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