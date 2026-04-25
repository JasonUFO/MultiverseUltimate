#pragma once

#include <JuceHeader.h>

#if (MSVC)
#include "ipps.h"
#endif

#include "Synth/SynthEngine.h"
#include "Effects/Delay.h"
#include "Effects/Reverb.h"
#include "Presets/PresetManager.h"
#include "DrumSequencer/DrumSequencer.h"

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
    DelayEffect delay;
    ReverbEffect reverb;
    PresetManager presetManager;
    DrumSequencer drumSequencer;
    float masterVolume = 0.7f;

public:
    DrumSequencer& getDrumSequencer() { return drumSequencer; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
