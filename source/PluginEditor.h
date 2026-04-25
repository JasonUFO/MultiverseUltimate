#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DrumSequencer/DrumSequencerPanel.h"
#include "Synth/ModulationMatrix.h"
#include "Synth/ModulationMatrixPanel.h"

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
     ModulationMatrix modulationMatrix;
     ModulationMatrixPanel modulationMatrixPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
