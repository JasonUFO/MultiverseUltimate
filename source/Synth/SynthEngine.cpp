#include "SynthEngine.h"
#include <cmath>

SynthEngine::SynthEngine() {}

void SynthEngine::prepare(double sr, int)
{
    if (sr <= 0.0 || sr > 1000000.0)
        sr = 44100.0;

    sampleRate = static_cast<float>(sr);

    for (auto& vi : voices)
        vi.voice.setSampleRate(static_cast<float>(sr));

    for (auto& vi : fmVoices)
        vi.voice.setSampleRate(sr);
}

void SynthEngine::noteOn(int note, float velocity)
{
    if (synthMode == SynthMode::FM)
    {
        auto* vi = findFMVoiceForNote(note);
        if (vi != nullptr)
        {
            vi->voice.noteOff();
            vi->inUse = false;
        }

        vi = findFreeFMVoice();
        if (vi != nullptr)
        {
            vi->voice.noteOn(note, velocity);
            vi->inUse = true;
            vi->lastUseTime = voiceCounter++;
        }
        return;
    }

    auto* vi = findVoiceForNote(note);
    if (vi != nullptr)
    {
        vi->voice.noteOff();
        vi->inUse = false;
    }

    vi = findFreeVoice();
    if (vi != nullptr)
    {
        vi->voice.noteOn(note, velocity);
        vi->inUse = true;
        vi->lastUseTime = voiceCounter++;
    }
}

void SynthEngine::noteOff(int note)
{
    if (synthMode == SynthMode::FM)
    {
        auto* vi = findFMVoiceForNote(note);
        if (vi != nullptr)
            vi->voice.noteOff();
        return;
    }

    auto* vi = findVoiceForNote(note);
    if (vi != nullptr)
        vi->voice.noteOff();
}

void SynthEngine::allNotesOff()
{
    for (auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive())
            vi.voice.noteOff();
    }

    for (auto& vi : fmVoices)
    {
        if (vi.inUse && vi.voice.isActive())
            vi.voice.noteOff();
    }
}

void SynthEngine::setMasterVolume(float volume)
{
    masterVolume = juce::jlimit(0.0f, 1.0f, volume);
}

void SynthEngine::setPitchBend(float cents)
{
    pitchBend = cents;
}

void SynthEngine::setEnvelopeParams(float a, float d, float s, float r)
{
    for (auto& vi : voices)
    {
        vi.voice.setEnvelopeParams(a, d, s, r);
    }
}

void SynthEngine::setFilterParams(float cutoff, float resonance)
{
    for (auto& vi : voices)
    {
        vi.voice.setFilterParams(cutoff, resonance);
    }
}

void SynthEngine::setWaveform(WaveformType type)
{
    for (auto& vi : voices)
    {
        vi.voice.setWaveform(type);
    }
}

float SynthEngine::process()
{
    float output = 0.0f;
    int activeCount = 0;

    if (synthMode == SynthMode::FM)
    {
        for (auto& vi : fmVoices)
        {
            if (vi.inUse && vi.voice.isActive())
            {
                output += vi.voice.process();
                ++activeCount;
            }
            else if (vi.inUse && !vi.voice.isActive())
            {
                vi.inUse = false;
            }
        }
    }
    else
    {
        for (auto& vi : voices)
        {
            if (vi.inUse && vi.voice.isActive())
            {
                output += vi.voice.process();
                ++activeCount;
            }
            else if (vi.inUse && !vi.voice.isActive())
            {
                vi.inUse = false;
            }
        }
    }

    if (activeCount > 0)
        output /= static_cast<float>(activeCount);

    return output * masterVolume;
}

int SynthEngine::getActiveVoiceCount() const
{
    if (synthMode == SynthMode::FM)
    {
        int count = 0;
        for (const auto& vi : fmVoices)
        {
            if (vi.inUse && vi.voice.isActive())
                ++count;
        }
        return count;
    }

    int count = 0;
    for (const auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive())
            ++count;
    }
    return count;
}

void SynthEngine::setSynthMode(SynthMode mode)
{
    if (mode == synthMode)
        return;

    allNotesOff();
    synthMode = mode;
}

void SynthEngine::setFMAlgorithm(int index)
{
    if (index < 0 || index >= FM_ALGORITHM_COUNT)
        return;

    fmAlgorithmIndex = index;
    for (auto& vi : fmVoices)
        vi.voice.setAlgorithm(index);
}

void SynthEngine::setFMOperatorParams(int opIndex,
                                      float ratio,
                                      float level,
                                      float feedback,
                                      float attack,
                                      float decay,
                                      float sustain,
                                      float release)
{
    for (auto& vi : fmVoices)
        vi.voice.setOperatorParams(opIndex, ratio, level, feedback, attack, decay, sustain, release);
}

SynthEngine::VoiceInfo* SynthEngine::findFreeVoice()
{
    VoiceInfo* oldest = nullptr;
    int oldestTime = INT_MAX;

    for (auto& vi : voices)
    {
        if (!vi.inUse)
            return &vi;

        if (vi.lastUseTime < oldestTime)
        {
            oldestTime = vi.lastUseTime;
            oldest = &vi;
        }
    }

    return oldest;
}

SynthEngine::VoiceInfo* SynthEngine::findVoiceForNote(int note)
{
    for (auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive() && vi.voice.getMidiNote() == note)
            return &vi;
    }
    return nullptr;
}

SynthEngine::FMVoiceInfo* SynthEngine::findFreeFMVoice()
{
    FMVoiceInfo* oldest = nullptr;
    int oldestTime = INT_MAX;

    for (auto& vi : fmVoices)
    {
        if (!vi.inUse)
            return &vi;

        if (vi.lastUseTime < oldestTime)
        {
            oldestTime = vi.lastUseTime;
            oldest = &vi;
        }
    }

    return oldest;
}

SynthEngine::FMVoiceInfo* SynthEngine::findFMVoiceForNote(int note)
{
    for (auto& vi : fmVoices)
    {
        if (vi.inUse && vi.voice.isActive() && vi.voice.getMidiNote() == note)
            return &vi;
    }
    return nullptr;
}