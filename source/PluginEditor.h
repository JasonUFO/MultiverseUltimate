#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MultiverseFlatTheme.h"
#include "DrumSequencer/DrumSequencerPanel.h"
#include "Synth/ModulationMatrixPanel.h"
#include "Synth/SynthPanel.h"
#include "Sampler/SamplerPanel.h"
#include "Sequencer/ProSequencerPanel.h"
#include "Sequencer/ArpeggiatorPanel.h"
#include "Effects/EffectsPanel.h"
#include "Presets/LibrarianPanel.h"
#include "Granular/GranularPanel.h"
#include "Layers/LayersPanel.h"
#include "Performance/PerformancePanel.h"
#include "UI/BottomBar.h"
#include "UI/QuickFXStrip.h"
#include "Routing/RoutingPanel.h"

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
    MultiverseFlatTheme   mvTheme;
    PluginProcessor& processorRef;
    DrumSequencerPanel    drumSequencerPanel;
    ModulationMatrixPanel modulationMatrixPanel;
    SamplerPanel          samplerPanel;
    ProSequencerPanel     proSequencerPanel;
    ArpeggiatorPanel     arpeggiatorPanel;
    SynthPanel            synthPanel;
    EffectsPanel          effectsPanel;
    GranularPanel         granularPanel;
    LayersPanel           layersPanel;
    PerformancePanel      performancePanel;
    RoutingPanel          routingPanel;
    juce::TabbedComponent tabs;

    // Librarian panel (permanent left sidebar)
    LibrarianPanel librarianPanel;

    // Bottom bar (macros + keyboard)
    BottomBar           bottomBar;

    // Right FX strip (placeholder for Phase 4)
    QuickFXStrip         quickFXStrip;

    // Preset navigation (header)
    juce::TextButton   prevPresetButton  { "<" };
    juce::Label        presetNameLabel;
    juce::TextButton   nextPresetButton  { ">" };
    juce::TextButton   favoriteButton    { "\xe2\x98\x85" };  // ★
    juce::TextButton   backButton        { "\xe2\x97\x80" };  // ◀
    juce::TextButton   forwardButton     { "\xe2\x96\xb6" };  // ▶

    void updatePresetNameLabel();
    void updateFavoriteButtonColor();
    void navigatePresetPrev();
    void navigatePresetNext();
    void cycleFavorite();

    // Tooltips
    juce::TooltipWindow tooltipWindow { this, 700 };
    bool tooltipsEnabled = true;

    // Menu button (☰)
    juce::TextButton menuButton { "\xe2\x98\xb0" };
    void showMainMenu();

    // Quick randomize
    juce::TextButton randomizeButton { "RAND" };
    void showRandomizeMenu();
    void randomizeParams(const juce::StringArray& prefixes, bool filterBoring = true);

    // Hidden controls (kept for APVTS attachments + menu access)
    juce::ComboBox scaleCombo;
    juce::ComboBox qualCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualAttachment;
    juce::TextButton fxModeButton { "FX" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fxModeAttachment;

    // MIDI Learn (via menu + CallOutBox)
    bool midiLearnActive = false;
    juce::Component::SafePointer<juce::ComboBox> midiLearnCallout;
    void showMidiLearnCallout();

    void setupTabs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};