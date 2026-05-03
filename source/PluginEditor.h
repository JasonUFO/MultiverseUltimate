#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MultiverseTheme.h"
#include "DrumSequencer/DrumSequencerPanel.h"
#include "Synth/ModulationMatrixPanel.h"
#include "Synth/SynthPanel.h"
#include "Sampler/SamplerPanel.h"
#include "Sequencer/ProSequencerPanel.h"
#include "Sequencer/ArpeggiatorPanel.h"
#include "Effects/EffectsPanel.h"
#include "Presets/PresetBrowserPanel.h"
#include "Macros/MacroPanel.h"
#include "Granular/GranularPanel.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (juce::Button*) override;
    void comboBoxChanged (juce::ComboBox*) override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    MultiverseTheme   mvTheme;
    PluginProcessor& processorRef;
    DrumSequencerPanel    drumSequencerPanel;
    ModulationMatrixPanel modulationMatrixPanel;
    SamplerPanel          samplerPanel;
    ProSequencerPanel     proSequencerPanel;
    ArpeggiatorPanel     arpeggiatorPanel;
    SynthPanel            synthPanel;
    EffectsPanel          effectsPanel;
    MacroPanel            macroPanel;
    GranularPanel         granularPanel;
    juce::TabbedComponent tabs;

    // Preset browser
    PresetBrowserPanel presetBrowserPanel;
    juce::TextButton   presetsButton { "Presets" };
    bool               presetsVisible = false;

    // Tooltips
    juce::TooltipWindow tooltipWindow { this, 700 };
    juce::TextButton    helpButton    { "?" };

    // MIDI Learn
    juce::ToggleButton midiLearnButton;
    juce::Label        midiLearnLabel;
    juce::ComboBox     paramSelector;

    void setupTabs();
    void setupMidiLearnButton();
    void updateMidiLearnUI();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
