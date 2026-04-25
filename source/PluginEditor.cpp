#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p), processorRef (p),
      drumSequencerPanel (p.getDrumSequencer()),
      modulationMatrixPanel (p.getModulationMatrix()),
      samplerPanel (p.getSamplerEngine()),
      sequencerPanel (p.getSequencer()),
      tabs (juce::TabbedButtonBar::TabsAtTop)
{
    setupTabs();
    addAndMakeVisible(tabs);
    setSize (1200, 800);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::setupTabs()
{
    tabs.addTab ("Drums", juce::Colours::darkgrey, &drumSequencerPanel, true);
    tabs.addTab ("Modulation", juce::Colours::darkgrey, &modulationMatrixPanel, true);
    tabs.addTab ("Sampler", juce::Colours::darkgrey, &samplerPanel, true);
    tabs.addTab ("Sequencer", juce::Colours::darkgrey, &sequencerPanel, true);
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    tabs.setBounds (getLocalBounds().reduced (0));
}
