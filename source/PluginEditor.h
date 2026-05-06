#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CyberpunkTheme.h"
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
#include "Layers/LayersPanel.h"
#include "Performance/PerformancePanel.h"

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
    CyberpunkTheme   mvTheme;
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
    LayersPanel           layersPanel;
    PerformancePanel      performancePanel;
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

    // Built-in keyboard
    juce::MidiKeyboardComponent keyboard;
    static constexpr int KEYBOARD_H = 64;

    // Quick randomize
    juce::TextButton randomizeButton { "RAND" };
    void showRandomizeMenu();
    void randomizeParams(const juce::StringArray& prefixes, bool filterBoring = true);

    // UI scale
    juce::ComboBox scaleCombo;

    // Global quality (oversampling)
    juce::ComboBox qualCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualAttachment;

    // FX Mode (audio input passthrough)
    juce::TextButton fxModeButton { "FX" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fxModeAttachment;

    void setupTabs();
    void setupMidiLearnButton();
    void updateMidiLearnUI();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
