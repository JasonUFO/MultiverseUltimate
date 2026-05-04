#include "SamplerEngine.h"
#include <climits>

SamplerEngine::SamplerEngine()
{
    formatManager.registerBasicFormats();
}

void SamplerEngine::prepare (double sr, int samplesPerBlock)
{
    sampleRate = (sr > 0.0 && sr <= 1000000.0) ? sr : 44100.0;
    for (auto& vi : voices)
        vi.voice.prepare (sampleRate);

    zonesVersion.store(0, std::memory_order_relaxed);
    lastProcessedVersion = 0;
}

void SamplerEngine::prepareZonesForPlayback()
{
    int currentVersion = zonesVersion.load(std::memory_order_acquire);
    if (currentVersion != lastProcessedVersion)
    {
        swapZoneBuffers();
        lastProcessedVersion = currentVersion;
    }
}

void SamplerEngine::swapZoneBuffers()
{
    zonesReadOnly.clear();
    {
        juce::ScopedLock sl(zoneLock);
        for (const auto& z : zones)
            zonesReadOnly.push_back(z);
    }
}

void SamplerEngine::noteOn (int midiNote, float velocity)
{
    prepareZonesForPlayback();

    const SamplerZone* matchedZone = nullptr;
    int velInt = juce::jlimit (0, 127, static_cast<int> (velocity * 127.0f));

    for (const auto& z : zonesReadOnly)
    {
        if (z->matchesNote (midiNote, velInt))
        {
            matchedZone = z.get();
            break;
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
    {
        juce::ScopedLock sl (zoneLock);
        zones.push_back (std::move (zone));
    }
    zonesVersion.fetch_add(1, std::memory_order_release);
}

void SamplerEngine::clearZones()
{
    // Force-stop all voices before freeing zone data
    for (auto& vi : voices)
    {
        vi.voice.forceStop();
        vi.inUse = false;
    }

    {
        juce::ScopedLock sl (zoneLock);
        zones.clear();
    }
    zonesVersion.fetch_add(1, std::memory_order_release);
}

int SamplerEngine::getZoneCount() const
{
    // This is only for UI - can use lock
    juce::ScopedLock sl (zoneLock);
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

int SamplerEngine::processBuffer(juce::AudioBuffer<float>& buffer, int numSamples)
{
    prepareZonesForPlayback();

    int activeCount = 0;
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (auto& vi : voices)
    {
        if (!vi.inUse) continue;

        if (vi.voice.isActive())
        {
            for (int i = 0; i < numSamples; ++i)
            {
                float s = vi.voice.process();
                left[i] += s;
                right[i] += s;
            }
            ++activeCount;
        }
        else
        {
            vi.inUse = false;
        }
    }

    if (activeCount > 1)
    {
        float scale = 1.0f / static_cast<float>(activeCount);
        for (int i = 0; i < numSamples; ++i)
        {
            left[i] *= scale;
            right[i] *= scale;
        }
    }

    float vol = masterVolume;
    for (int i = 0; i < numSamples; ++i)
    {
        left[i] *= vol;
        right[i] *= vol;
    }

    return activeCount;
}

juce::ValueTree SamplerEngine::getState() const
{
    juce::ValueTree state ("SamplerEngine");
    const juce::ScopedLock sl (zoneLock);
    for (const auto& zone : zones)
    {
        juce::ValueTree zoneNode ("Zone");
        zoneNode.setProperty ("filePath",       zone->filePath,                       nullptr);
        zoneNode.setProperty ("name",           zone->name,                           nullptr);
        zoneNode.setProperty ("loNote",         zone->loNote,                         nullptr);
        zoneNode.setProperty ("hiNote",         zone->hiNote,                         nullptr);
        zoneNode.setProperty ("loVel",          zone->loVel,                          nullptr);
        zoneNode.setProperty ("hiVel",          zone->hiVel,                          nullptr);
        zoneNode.setProperty ("rootNote",       zone->rootNote,                       nullptr);
        zoneNode.setProperty ("tuning",          zone->tuning,                         nullptr);
        zoneNode.setProperty ("speed",           zone->speed,                          nullptr);
        zoneNode.setProperty ("loopMode",       static_cast<int> (zone->loopMode),   nullptr);
        zoneNode.setProperty ("loopStart",      zone->loopStart,                      nullptr);
        zoneNode.setProperty ("loopEnd",        zone->loopEnd,                        nullptr);
        zoneNode.setProperty ("crossfadeLength",zone->crossfadeLength,               nullptr);
        state.appendChild (zoneNode, nullptr);
    }
    return state;
}

void SamplerEngine::setState (const juce::ValueTree& state)
{
    clearZones();

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto zoneNode = state.getChild (i);
        if (! zoneNode.hasType ("Zone"))
            continue;

        juce::String filePath = zoneNode.getProperty ("filePath", "");
        if (filePath.isEmpty())
            continue;

        juce::File file (filePath);
        if (! file.existsAsFile())
            continue;

        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
        if (reader == nullptr)
            continue;

        auto zone = std::make_shared<SamplerZone>();
        zone->filePath       = filePath;
        zone->name           = zoneNode.getProperty ("name", file.getFileNameWithoutExtension());
        zone->loNote         = (int) zoneNode.getProperty ("loNote",          0);
        zone->hiNote         = (int) zoneNode.getProperty ("hiNote",        127);
        zone->loVel          = (int) zoneNode.getProperty ("loVel",           0);
        zone->hiVel          = (int) zoneNode.getProperty ("hiVel",         127);
        zone->rootNote       = (int) zoneNode.getProperty ("rootNote",       60);
        zone->tuning         = (float) zoneNode.getProperty ("tuning",          0.0f);
        zone->speed          = (float) zoneNode.getProperty ("speed",           1.0f);
        zone->loopMode       = static_cast<LoopMode> ((int) zoneNode.getProperty ("loopMode", 0));
        zone->loopStart      = (int) zoneNode.getProperty ("loopStart",       0);
        zone->loopEnd        = (int) zoneNode.getProperty ("loopEnd",         0);
        zone->crossfadeLength= (int) zoneNode.getProperty ("crossfadeLength", 0);
        zone->fileSampleRate = reader->sampleRate;

        int numSamples  = (int) reader->lengthInSamples;
        int numChannels = (int) reader->numChannels;
        zone->audioData.setSize (numChannels, numSamples);
        reader->read (&zone->audioData, 0, numSamples, 0, true, true);

        addZone (std::move (zone));
    }
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
