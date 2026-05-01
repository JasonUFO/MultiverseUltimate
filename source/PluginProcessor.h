#pragma once

#include <JuceHeader.h>

#if defined(_MSC_VER)
#include "ipps.h"
#endif

#include "Synth/SynthEngine.h"
#include "Sampler/SamplerEngine.h"
#include "Effects/Delay.h"
#include "Effects/Reverb.h"
#include "Effects/Chorus.h"
#include "Effects/Distortion.h"
#include "Effects/EQ.h"
#include "Effects/Compressor.h"
#include "Presets/PresetManager.h"
#include "DrumSequencer/DrumSequencer.h"
#include "Synth/ModulationMatrix.h"
#include "Sequencer/Sequencer.h"
#include "Sequencer/ProSequencer.h"
#include "Sequencer/Arpeggiator.h"
#include "Sequencer/PatternEngine.h"
#include "Macros/MacroManager.h"
#include "Granular/GranularEngine.h"

enum class EffectID { Chorus = 0, Distortion = 1, EQ = 2, Compressor = 3, Delay = 4, Reverb = 5 };

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Public so panels can attach sliders/buttons directly
    juce::AudioProcessorValueTreeState apvts;

    DrumSequencer&  getDrumSequencer()   { return drumSequencer; }
    ModulationMatrix& getModulationMatrix() { return modulationMatrix; }
    SamplerEngine&  getSamplerEngine()   { return samplerEngine; }
    Sequencer&      getSequencer()       { return sequencer; }
    ProSequencer&   getProSequencer()    { return proSequencer; }
    Arpeggiator&    getArpeggiator()     { return arpeggiator; }
    PatternEngine&   getPatternEngine()    { return patternEngine; }
    SynthEngine&    getSynthEngine()     { return synthEngine; }
    DelayEffect&    getDelay()           { return delay; }
    ReverbEffect&   getReverb()          { return reverb; }
    PresetManager&  getPresetManager()   { return presetManager; }
    MacroManager&     getMacroManager()    { return macroManager; }
    GranularEngine&   getGranularEngine() { return granularEngine; }

    // Effect chain ordering (6 effects, packed as nibbles in a uint32)
    int  getChainSlot(int pos) const noexcept
    {
        return static_cast<int>((effectChainOrder.load(std::memory_order_relaxed) >> (pos * 4)) & 0xFu);
    }
    void swapChainSlots(int a, int b) noexcept;

    void saveNamedPreset (const juce::String& name);
    bool loadPresetAtIndex (int index);

public:
    WaveformType baseWaveform = WaveformType::Saw;
    MacroManager      macroManager;
    GranularEngine    granularEngine;
    SynthEngine       synthEngine;
    SamplerEngine     samplerEngine;
    DelayEffect       delay;
    ReverbEffect      reverb;
    ChorusEffect      chorus[2];
    DistortionEffect  distortion[2];
    EQEffect          eq[2];
    CompressorEffect  compressor[2];
    // Effect chain order: 6 nibbles packed in uint32, default Chorus→Distortion→EQ→Compressor→Delay→Reverb
    std::atomic<uint32_t> effectChainOrder { 0x543210u };
    PresetManager     presetManager;
    DrumSequencer     drumSequencer;
    ModulationMatrix  modulationMatrix;
    Sequencer         sequencer;
    ProSequencer      proSequencer;
    Arpeggiator       arpeggiator;
    PatternEngine     patternEngine;

    // LFO base rates — not yet automated (no UI knobs)
    float baseLfoRates[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float basePitchBend = 0.0f;
    float baseFilterModAmount = 0.0f;

    bool sustainPedalDown = false;
    bool sostenutoPedalDown = false;
    bool sustainedNoteHeld[128] = {};
    bool sostenutoNoteHeld[128] = {};
    juce::UndoManager undoManager;

    // MIDI Learn
    bool midiLearnActive = false;
    int learnParameterIndex = -1; // index in apvts parameters, -1 means none selected
    struct MidiMapping
    {
        juce::String paramID;
        enum Type { None, CC, PitchWheel, ChannelPressure } type = None;
        int controllerNumber = 0; // for CC: 0-127
        int midiChannel = 0; // 0-15, 0 means omni
    };
    std::vector<MidiMapping> midiMappings;

    // MIDI Learn methods
    void startMidiLearnForParameter(int parameterIndex);
    void stopMidiLearn();
    void handleMidiForLearn(const juce::MidiMessage& message);
    void applyMidiMapping(const juce::MidiMessage& message);
    juce::String getParameterIDFromIndex(int index) const;
    int getParameterIndexFromID(const juce::String& paramID) const;
    void updateMidiMappingsInState(juce::ValueTree& stateTree) const;
    void loadMidiMappingsFromState(const juce::ValueTree& stateTree);

    bool dawWasPlaying = false;
    float envFollowerLevel = 0.0f;

private:
    float applyChainEffect(int effectID, float sample, int ch);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
