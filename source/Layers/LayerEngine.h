#pragma once
#include <JuceHeader.h>
#include "../Synth/SynthEngine.h"
#include "../Granular/GranularEngine.h"
#include "../Sampler/SamplerEngine.h"

enum class LayerEngineType
{
    Synth,
    Granular,
    Sampler,
    Off
};

class LayerEngine
{
public:
    LayerEngine(int layerIndex);
    ~LayerEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock);

    // Engine type
    void setEngineType(LayerEngineType type);
    LayerEngineType getEngineType() const { return engineType; }

    // Audio processing
    int processBlock(juce::AudioBuffer<float>& buffer, int numSamples);

    // MIDI
    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void allNotesOff();

    // Modulation
    void setModulationValue(int target, float value);

    // Layer controls
    void setLevel(float level) { this->level = juce::jlimit(0.0f, 1.0f, level); }
    float getLevel() const { return level; }

    void setPan(float pan) { this->pan = juce::jlimit(-1.0f, 1.0f, pan); }
    float getPan() const { return pan; }

    void setMute(bool mute) { this->mute = mute; }
    bool isMuted() const { return mute; }

    void setSolo(bool solo) { this->solo = solo; }
    bool isSoloed() const { return solo; }

    // Access to underlying engines (for UI)
    SynthEngine* getSynthEngine() { return synthEngine.get(); }
    GranularEngine* getGranularEngine() { return granularEngine.get(); }
    SamplerEngine* getSamplerEngine() { return samplerEngine.get(); }

    // State persistence
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& tree);

private:
    int layerIndex = 0;
    LayerEngineType engineType = LayerEngineType::Synth;

    std::unique_ptr<SynthEngine> synthEngine;
    std::unique_ptr<GranularEngine> granularEngine;
    std::unique_ptr<SamplerEngine> samplerEngine;

    float level = 1.0f;
    float pan = 0.0f;
    bool mute = false;
    bool solo = false;

    // Voice allocation (equal share)
    int voiceShare = 2; // 16 / 8 layers = 2 voices per layer for synth

    void applyPan(juce::AudioBuffer<float>& buffer, int numSamples);
};
