#pragma once

#include <JuceHeader.h>

#if defined(_MSC_VER)
#include "ipps.h"
#endif

#include "Synth/SynthEngine.h"
#include "Sampler/SamplerEngine.h"
#include "Effects/Delay.h"
#include "Effects/Reverb.h"
#include "Presets/PresetManager.h"
#include "DrumSequencer/DrumSequencer.h"
#include "Synth/ModulationMatrix.h"
#include "Sequencer/Sequencer.h"

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

    DrumSequencer& getDrumSequencer() { return drumSequencer; }
    ModulationMatrix& getModulationMatrix() { return modulationMatrix; }
    SamplerEngine& getSamplerEngine() { return samplerEngine; }
    Sequencer& getSequencer() { return sequencer; }
    SynthEngine& getSynthEngine() { return synthEngine; }
    DelayEffect& getDelay() { return delay; }
    ReverbEffect& getReverb() { return reverb; }
    PresetManager& getPresetManager() { return presetManager; }

    void saveNamedPreset(const juce::String& name);
    bool loadPresetAtIndex(int index);

public:
    WaveformType baseWaveform = WaveformType::Saw;
    SynthEngine synthEngine;
    SamplerEngine samplerEngine;
    DelayEffect delay;
    ReverbEffect reverb;
    PresetManager presetManager;
    DrumSequencer drumSequencer;
    ModulationMatrix modulationMatrix;
    Sequencer sequencer;

    // LFO base rates — not yet automated (no UI knobs)
    float baseLfoRates[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float basePitchBend = 0.0f;

    bool sustainPedalDown = false;
    bool sustainedNoteHeld[128] = {};
    juce::UndoManager undoManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
