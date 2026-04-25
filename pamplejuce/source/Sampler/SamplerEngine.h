#pragma once
#include "SamplerVoice.h"
#include <array>
#include <vector>
#include <memory>
#include <juce_core/juce_core.h>

constexpr int MAX_SAMPLER_VOICES = 16;

class SamplerEngine
{
public:
    SamplerEngine();

    void prepare (double sampleRate, int samplesPerBlock);

    void noteOn (int midiNote, float velocity);
    void noteOff (int midiNote);
    void allNotesOff();
    void setMasterVolume (float volume);

    // Zone management — called from UI thread only
    void addZone (std::shared_ptr<SamplerZone> zone);
    void clearZones();
    int getZoneCount() const;

    float process();

private:
    struct VoiceInfo
    {
        SamplerVoice voice;
        bool inUse = false;
        int lastUseTime = 0;
    };

    VoiceInfo* findFreeVoice();
    VoiceInfo* findVoiceForNote (int midiNote);

    std::array<VoiceInfo, MAX_SAMPLER_VOICES> voices;
    std::vector<std::shared_ptr<SamplerZone>> zones;
    juce::CriticalSection zoneLock;

    float masterVolume = 0.7f;
    double sampleRate = 44100.0;
    int voiceCounter = 0;
};
