#pragma once
#include "SamplerVoice.h"
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <memory>

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

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& state);

    float process();

    int processBuffer(juce::AudioBuffer<float>& buffer, int numSamples);

private:
    struct VoiceInfo
    {
        MvSamplerVoice voice;
        bool inUse = false;
        int lastUseTime = 0;
    };

    VoiceInfo* findFreeVoice();
    VoiceInfo* findVoiceForNote (int midiNote);

    std::array<VoiceInfo, MAX_SAMPLER_VOICES> voices;
    std::vector<std::shared_ptr<SamplerZone>> zones;

    juce::CriticalSection zoneLock;

    std::vector<std::shared_ptr<SamplerZone>> zonesReadOnly;
    std::atomic<int> zonesVersion{0};
    int lastProcessedVersion = 0;

    void prepareZonesForPlayback();
    void swapZoneBuffers();

    juce::AudioFormatManager formatManager;

    float masterVolume = 0.7f;
    double sampleRate = 44100.0;
    int voiceCounter = 0;
};
