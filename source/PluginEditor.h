#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DrumSequencer/DrumSequencerPanel.h"
#include "Synth/ModulationMatrixPanel.h"
#include "Synth/SynthPanel.h"
#include "Sampler/SamplerPanel.h"
#include "Sequencer/SequencerPanel.h"

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

 private:
    PluginProcessor& processorRef;
    DrumSequencerPanel drumSequencerPanel;
    ModulationMatrixPanel modulationMatrixPanel;
    SamplerPanel samplerPanel;
    SequencerPanel sequencerPanel;
    SynthPanel synthPanel;
    juce::TabbedComponent tabs;

    void setupTabs();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
