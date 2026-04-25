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

private:
    SynthEngine synthEngine;
    SamplerEngine samplerEngine;
    DelayEffect delay;
    ReverbEffect reverb;
    PresetManager presetManager;
    DrumSequencer drumSequencer;
    ModulationMatrix modulationMatrix;
    Sequencer sequencer;  // melodic sequencer
    float masterVolume = 0.7f;

    // Base parameters for modulation
    float baseFilterCutoff = 20000.0f;
    float baseFilterResonance = 0.707f;
    float baseLfoRates[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    // Sustain pedal state (CC 64)
    bool sustainPedalDown = false;
    bool sustainedNoteHeld[128] = {};

public:
    DrumSequencer& getDrumSequencer() { return drumSequencer; }
    ModulationMatrix& getModulationMatrix() { return modulationMatrix; }
    SamplerEngine& getSamplerEngine() { return samplerEngine; }
    Sequencer& getSequencer() { return sequencer; }
    SynthEngine& getSynthEngine() { return synthEngine; }

    float getBaseFilterCutoff() const { return baseFilterCutoff; }
    float getBaseFilterResonance() const { return baseFilterResonance; }
    void setBaseFilterCutoff(float v) { baseFilterCutoff = juce::jlimit(20.0f, 20000.0f, v); }
    void setBaseFilterResonance(float v) { baseFilterResonance = juce::jlimit(0.1f, 10.0f, v); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
