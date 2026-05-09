#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../NeuKnob.h"
#include "../Macros/MacroManager.h"
#include "../Synth/EnvelopeDisplay.h"

class PluginProcessor;

// Pitch bend wheel — springs back to center on release
class PitchWheel : public juce::Component
{
public:
    PitchWheel() {}

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    float getValue() const { return value; }
    std::function<void(float)> onValueChange;

private:
    float value = 0.0f;
    int lastY = 0;
};

// Modulation wheel — stays where you put it
class ModWheel : public juce::Component
{
public:
    ModWheel() {}

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

    float getValue() const { return value; }
    std::function<void(float)> onValueChange;

private:
    float value = 0.0f;
};

//==============================================================================
// Sub-tab indices
enum ModSubTab
{
    kEnv1 = 0,
    kEnv2,
    kEnv3,
    kLfo,
    kMacro,
    kQfx,
    kKey,
    kNumSubTabs
};

//==============================================================================
// ENV sub-panel: ADSR knobs + EnvelopeDisplay
class EnvSubPanel : public juce::Component
{
public:
    explicit EnvSubPanel(PluginProcessor& p, int envIndex);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;
    int envIndex; // 0=ENV1(amp), 1=ENV2, 2=ENV3

    struct KnobGroup
    {
        NeuKnob knob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
        juce::Label label { {}, {} };
    };

    KnobGroup attack, decay, sustain, release;
    EnvelopeDisplay envelopeDisplay;

    static constexpr int KNOB_SZ = 52;
};

//==============================================================================
// LFO sub-panel: bank selector (LFO1-8) + Rate/Shape/Sync/SyncDiv + DRAW
class LFOSubPanel : public juce::Component,
                    public juce::Button::Listener,
                    private juce::Timer
{
public:
    explicit LFOSubPanel(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* btn) override;

private:
    PluginProcessor& proc;
    int activeBank = 0; // 0-7 for LFO1-8

    // Bank selector buttons
    std::array<juce::TextButton, 8> bankButtons;

    // Controls (re-attached on bank switch)
    NeuKnob rateKnob;
    juce::ComboBox shapeCombo;
    juce::ToggleButton syncButton { "Sync" };
    juce::ComboBox syncDivCombo;
    juce::TextButton drawButton { "DRAW" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> syncDivAttach;

    juce::Label rateLabel { {}, "Rate" };
    juce::Label shapeLabel { {}, "Shape" };

    // LFO shape preview
    juce::Path wavePath;
    bool wavePathDirty = true;
    void rebuildWavePath();
    void timerCallback() override;

    void switchBank(int newBank);
    void showShapeEditor();

    static constexpr int KNOB_SZ = 52;
};

//==============================================================================
// Macro sub-panel: 8 macro knobs + name + value labels
class MacroSubPanel : public juce::Component, private juce::Timer
{
public:
    explicit MacroSubPanel(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    struct MacroKnobGroup
    {
        juce::Slider knob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::Label nameLabel;
        juce::Label valueLabel;
    };

    std::array<MacroKnobGroup, MacroManager::NUM_MACROS> macros;
    void timerCallback() override;
    void showAssignmentList(int macroIdx);

    static constexpr int KNOB_SIZE = 48;
};

//==============================================================================
// Quick FX sub-panel: horizontal layout of all QuickFX controls
class QuickFXSubPanel : public juce::Component
{
public:
    explicit QuickFXSubPanel(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    static constexpr int KNOB_SZ = 40;
    static constexpr int LABEL_H = 14;
    static constexpr int TOGGLE_H = 20;
    static constexpr int COMBO_H = 22;

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttach  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct KnobGroup
    {
        NeuKnob knob;
        std::unique_ptr<SliderAttach> attach;
        juce::Label label { {}, {} };
    };

    // Enable toggles
    juce::ToggleButton filterModEnable  { "ON" };
    juce::ToggleButton ampModEnable     { "ON" };
    juce::ToggleButton mainFilterEnable { "ON" };
    std::unique_ptr<ButtonAttach> filterModEnableAttach;
    std::unique_ptr<ButtonAttach> ampModEnableAttach;
    std::unique_ptr<ButtonAttach> mainFilterEnableAttach;

    // Filter Mod
    KnobGroup fmCutoff, fmResonance, fmEnvDepth;
    // Amp Mod
    KnobGroup amVolume, amPan;
    // Delay
    KnobGroup delayMix, delayTime, delayFeedback;
    // Reverb
    KnobGroup reverbWet, reverbRoom, reverbDamp;
    // Main Filter
    KnobGroup mfCutoff, mfResonance;
    juce::ComboBox mfTypeCombo;
    std::unique_ptr<ComboAttach> mfTypeAttach;

    // Section headers
    juce::Label filterModLabel  { {}, "FLT MOD" };
    juce::Label ampModLabel     { {}, "AMP MOD" };
    juce::Label delayLabel      { {}, "DELAY" };
    juce::Label reverbLabel     { {}, "REVERB" };
    juce::Label mainFilterLabel { {}, "FILTER" };

    void setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& shortName);
};

//==============================================================================
// Keyboard sub-panel: pitch/mod wheels + MIDI keyboard
class KeyboardSubPanel : public juce::Component
{
public:
    explicit KeyboardSubPanel(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;
    PitchWheel pitchWheel;
    ModWheel   modWheel;
    juce::MidiKeyboardComponent keyboard;

    static constexpr int WHEEL_W    = 28;
    static constexpr int KEYBOARD_H  = 64;

    void sendPitchBend(float value);
    void sendModWheel(float value);
};

//==============================================================================
// ModBar: bottom modulation bar with sub-tabs
class ModBar : public juce::Component,
               public juce::Button::Listener,
               private juce::Timer
{
public:
    explicit ModBar(PluginProcessor& p);
    ~ModBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    static constexpr int MOD_BAR_H = 160;
    static constexpr int SUB_TAB_H = 28;

private:
    PluginProcessor& proc;

    // Sub-tab buttons
    std::array<juce::TextButton, kNumSubTabs> subTabButtons;
    int activeSubTab = kMacro; // default to MACRO

    // Sub-panels
    EnvSubPanel      env1Panel;
    EnvSubPanel      env2Panel;
    EnvSubPanel      env3Panel;
    LFOSubPanel      lfoPanel;
    MacroSubPanel    macroPanel;
    QuickFXSubPanel  quickFXPanel;
    KeyboardSubPanel keyboardPanel;

    std::array<juce::Component*, kNumSubTabs> subPanels;

    void switchSubTab(int newIndex);
    void buttonClicked(juce::Button* btn) override;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModBar)
};