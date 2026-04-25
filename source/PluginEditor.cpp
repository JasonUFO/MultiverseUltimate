#include "PluginEditor.h"

 PluginEditor::PluginEditor (PluginProcessor& p)
     : AudioProcessorEditor (p), processorRef (p),
       drumSequencerPanel (p.getDrumSequencer()),
       modulationMatrixPanel (modulationMatrix)
 {
     addAndMakeVisible (drumSequencerPanel);
     addAndMakeVisible (modulationMatrixPanel);
 
     setSize (1200, 800);
 }

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawText (JucePlugin_Name, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (6);
    auto separator = area.removeFromLeft (area.getWidth() / 2);
    drumSequencerPanel.setBounds (area);
    modulationMatrixPanel.setBounds (separator);
}
