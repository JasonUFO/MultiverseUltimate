#pragma once
#include <JuceHeader.h>
#include "../Synth/SynthEngine.h"
#include "../Granular/GranularEngine.h"
#include "../Sampler/SamplerEngine.h"
#include "LayerEffectChain.h"

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

    // MIDI (midiChannel: 1-16; 0 = omni)
    void noteOn(int midiNote, float velocity, int midiChannel = 0);
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

    // Note range filter (0-127)
    void setNoteRange(int lo, int hi) { loNote = juce::jlimit(0,127,lo); hiNote = juce::jlimit(0,127,hi); }
    int  getLoNote() const { return loNote; }
    int  getHiNote() const { return hiNote; }

    // Velocity range filter (0-127)
    void setVelocityRange(int lo, int hi) { loVel = juce::jlimit(0,127,lo); hiVel = juce::jlimit(0,127,hi); }
    int  getLoVel() const { return loVel; }
    int  getHiVel() const { return hiVel; }

    // MIDI channel filter (0 = all channels)
    void setMidiChannelFilter(int ch) { midiChannelFilter = juce::jlimit(0,16,ch); }
    int  getMidiChannelFilter() const { return midiChannelFilter; }

    // Per-layer effect chain
    LayerEffectChain& getEffectChain() { return effectChain; }

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

    // Note / velocity / MIDI channel range
    int loNote = 0, hiNote = 127;
    int loVel  = 0, hiVel  = 127;
    int midiChannelFilter = 0; // 0 = all

    LayerEffectChain effectChain;

    // Voice allocation (equal share)
    int voiceShare = 2; // 16 / 8 layers = 2 voices per layer for synth

    void applyPan(juce::AudioBuffer<float>& buffer, int numSamples);
};
