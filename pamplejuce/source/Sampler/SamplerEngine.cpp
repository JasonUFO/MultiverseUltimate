#include "SamplerEngine.h"
#include <juce_core/juce_core.h>
#include <climits>

SamplerEngine::SamplerEngine() {}

void SamplerEngine::prepare (double sr, int /*samplesPerBlock*/)
{
    sampleRate = (sr > 0.0 && sr <= 1000000.0) ? sr : 44100.0;
    for (auto& vi : voices)
        vi.voice.prepare (sampleRate);
}

void SamplerEngine::noteOn (int midiNote, float velocity)
{
    const SamplerZone* matchedZone = nullptr;
    int velInt = juce::jlimit (0, 127, static_cast<int> (velocity * 127.0f));

    {
        const juce::ScopedLock sl (zoneLock);
        for (const auto& z : zones)
        {
            if (z->matchesNote (midiNote, velInt))
            {
                matchedZone = z.get();
                break;
            }
        }
    }

    if (matchedZone == nullptr)
        return;

    // Retrigger if already playing this note
    if (auto* existing = findVoiceForNote (midiNote))
    {
        existing->voice.noteOn (matchedZone, midiNote, velocity);
        existing->lastUseTime = ++voiceCounter;
        return;
    }

    auto* vi = findFreeVoice();
    if (vi == nullptr)
        return;

    vi->inUse = true;
    vi->lastUseTime = ++voiceCounter;
    vi->voice.noteOn (matchedZone, midiNote, velocity);
}

void SamplerEngine::noteOff (int midiNote)
{
    for (auto& vi : voices)
        if (vi.inUse && vi.voice.getMidiNote() == midiNote)
            vi.voice.noteOff();
}

void SamplerEngine::allNotesOff()
{
    for (auto& vi : voices)
        if (vi.inUse)
            vi.voice.noteOff();
}

void SamplerEngine::setMasterVolume (float volume)
{
    masterVolume = juce::jlimit (0.0f, 1.0f, volume);
}

void SamplerEngine::addZone (std::shared_ptr<SamplerZone> zone)
{
    const juce::ScopedLock sl (zoneLock);
    zones.push_back (std::move (zone));
}

void SamplerEngine::clearZones()
{
    // Force-stop all voices before freeing zone data
    for (auto& vi : voices)
    {
        vi.voice.forceStop();
        vi.inUse = false;
    }

    const juce::ScopedLock sl (zoneLock);
    zones.clear();
}

int SamplerEngine::getZoneCount() const
{
    const juce::ScopedLock sl (zoneLock);
    return static_cast<int> (zones.size());
}

float SamplerEngine::process()
{
    float output = 0.0f;
    int activeCount = 0;

    for (auto& vi : voices)
    {
        if (!vi.inUse)
            continue;

        if (vi.voice.isActive())
        {
            output += vi.voice.process();
            ++activeCount;
        }
        else
        {
            vi.inUse = false;
        }
    }

    if (activeCount > 1)
        output /= static_cast<float> (activeCount);

    return output * masterVolume;
}

SamplerEngine::VoiceInfo* SamplerEngine::findFreeVoice()
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

    // Voice steal: reset the oldest voice
    if (oldest != nullptr)
    {
        oldest->voice.forceStop();
        oldest->inUse = false;
    }
    return oldest;
}

SamplerEngine::VoiceInfo* SamplerEngine::findVoiceForNote (int midiNote)
{
    for (auto& vi : voices)
        if (vi.inUse && vi.voice.getMidiNote() == midiNote)
            return &vi;
    return nullptr;
}
