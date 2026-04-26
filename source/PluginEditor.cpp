#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p), processorRef (p),
      drumSequencerPanel (p.getDrumSequencer()),
      modulationMatrixPanel (p.getModulationMatrix()),
      samplerPanel (p.getSamplerEngine()),
      sequencerPanel (p.getSequencer()),
      synthPanel (p),
      effectsPanel (p),
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
    tabs.addTab ("Synth",      juce::Colours::darkgrey, &synthPanel,            false);
    tabs.addTab ("Drums",      juce::Colours::darkgrey, &drumSequencerPanel,    false);
    tabs.addTab ("Modulation", juce::Colours::darkgrey, &modulationMatrixPanel, false);
    tabs.addTab ("Sampler",    juce::Colours::darkgrey, &samplerPanel,          false);
    tabs.addTab ("Sequencer",  juce::Colours::darkgrey, &sequencerPanel,        false);
    tabs.addTab ("Effects",    juce::Colours::darkgrey, &effectsPanel,          false);
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
