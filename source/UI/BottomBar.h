#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../SkinManager.h"
#include "../NeuKnob.h"
#include "../Macros/MacroManager.h"
#include "../Synth/EnvelopeDisplay.h"
#include "../Synth/ModulationMatrix.h"

class PluginProcessor;

// ── Pitch wheel — springs back to center, with vivid skin-aware rendering ──
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

// ── Mod wheel — stays where released, vivid skin-aware rendering ──
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
// Custom sub-tab button
class SubTabButton : public juce::Component
{
public:
    SubTabButton (const juce::String& buttonText) : text (buttonText) {}

    void paint (juce::Graphics& g) override
    {
        MultiverseFlatTheme::drawSubTabButton (g, getLocalBounds().toFloat(), text, isActive, isHover);
    }

    void mouseEnter (const juce::MouseEvent&) override { isHover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { isHover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override   { if (onClick) onClick(); }

    juce::String text;
    bool isActive = false;
    bool isHover = false;
    std::function<void()> onClick;
};

//==============================================================================
// LFO bank button
class BankButton : public juce::Component
{
public:
    BankButton (const juce::String& buttonText, int lfoIndex_)
        : text (buttonText), lfoIndex (lfoIndex_) {}

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float corner = bounds.getHeight() / 2.0f;
        const Skin& s = MultiverseFlatTheme::skin();

        if (isActive)
        {
            auto col = NeuKnob::getModSourceColour (lfoIndex < 4
                            ? static_cast<ModSourceType>(lfoIndex)
                            : static_cast<ModSourceType>(11 + lfoIndex));
            g.setColour (col.withAlpha (0.15f));
            g.fillRoundedRectangle (bounds, corner);
            g.setColour (col.withAlpha (0.5f));
            g.drawRoundedRectangle (bounds, corner, 1.0f);
        }
        else if (isHover)
        {
            g.setColour (s.bgHover);
            g.fillRoundedRectangle (bounds, corner);
            g.setColour (s.borderLight);
            g.drawRoundedRectangle (bounds, corner, 1.0f);
        }
        else
        {
            g.setColour (s.bgRaised);
            g.fillRoundedRectangle (bounds, corner);
            g.setColour (s.borderLight.withAlpha (0.5f));
            g.drawRoundedRectangle (bounds, corner, 1.0f);
        }

        g.setFont (juce::Font (juce::FontOptions{}.withHeight (10.0f)));
        g.setColour (isActive ? NeuKnob::getModSourceColour (lfoIndex < 4
                                    ? static_cast<ModSourceType>(lfoIndex)
                                    : static_cast<ModSourceType>(11 + lfoIndex))
                               : s.textSecondary);
        g.drawFittedText (text, getLocalBounds().reduced (4, 0),
                          juce::Justification::centred, 1);
    }

    void mouseEnter (const juce::MouseEvent&) override { isHover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { isHover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override   { if (onClick) onClick(); }

    juce::String text;
    int lfoIndex;
    bool isActive = false;
    bool isHover = false;
    std::function<void()> onClick;
};

//==============================================================================
// Sub-tab indices — KEY removed (always visible)
enum ModSubTab
{
    kEnv1 = 0,
    kEnv2,
    kEnv3,
    kLfo,
    kMacro,
    kQfx,
    kNumSubTabs   // was kKey — keyboard is now always visible
};

//==============================================================================
// ENV sub-panel
class EnvSubPanel : public juce::Component
{
public:
    explicit EnvSubPanel(PluginProcessor& p, int envIndex);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;
    int envIndex;

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
// LFO sub-panel
class LFOSubPanel : public juce::Component,
                    private juce::Timer
{
public:
    explicit LFOSubPanel(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    PluginProcessor& proc;
    int activeBank = 0;

    std::array<BankButton, 8> bankButtons {{
        BankButton{"1", 0}, BankButton{"2", 1}, BankButton{"3", 2}, BankButton{"4", 3},
        BankButton{"5", 4}, BankButton{"6", 5}, BankButton{"7", 6}, BankButton{"8", 7}
    }};

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

    juce::Path wavePath;
    bool wavePathDirty = true;
    void rebuildWavePath();
    void timerCallback() override;

    void switchBank(int newBank);
    void showShapeEditor();

    juce::Rectangle<int> dragArea;

    static constexpr int KNOB_SZ = 52;
};

//==============================================================================
// Macro sub-panel
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
// Quick FX sub-panel
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

    juce::ToggleButton filterModEnable  { "ON" };
    juce::ToggleButton ampModEnable     { "ON" };
    juce::ToggleButton mainFilterEnable { "ON" };
    std::unique_ptr<ButtonAttach> filterModEnableAttach;
    std::unique_ptr<ButtonAttach> ampModEnableAttach;
    std::unique_ptr<ButtonAttach> mainFilterEnableAttach;

    KnobGroup fmCutoff, fmResonance, fmEnvDepth;
    KnobGroup amVolume, amPan;
    KnobGroup delayMix, delayTime, delayFeedback;
    KnobGroup reverbWet, reverbRoom, reverbDamp;
    KnobGroup mfCutoff, mfResonance;
    juce::ComboBox mfTypeCombo;
    std::unique_ptr<ComboAttach> mfTypeAttach;

    juce::Label filterModLabel  { {}, "FLT MOD" };
    juce::Label ampModLabel     { {}, "AMP MOD" };
    juce::Label delayLabel      { {}, "DELAY" };
    juce::Label reverbLabel     { {}, "REVERB" };
    juce::Label mainFilterLabel { {}, "FILTER" };

    void setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& shortName);
};

//==============================================================================
// Keyboard strip — always-visible bottom strip with pitch/mod wheels + keyboard
class KeyboardStrip : public juce::Component
{
public:
    explicit KeyboardStrip(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;

    // Expose height so PluginEditor can query it
    static constexpr int STRIP_H = 110;

private:
    PluginProcessor& proc;
    PitchWheel pitchWheel;
    ModWheel   modWheel;
    juce::MidiKeyboardComponent keyboard;

    void sendPitchBend(float value);
    void sendModWheel(float value);
};

//==============================================================================
// ModBar: modulation bar with sub-tabs (no KEY tab — keyboard is always visible)
class ModBar : public juce::Component,
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

    // KEY removed from sub-tabs
    std::array<SubTabButton, kNumSubTabs> subTabButtons {{
        SubTabButton{"ENV1"}, SubTabButton{"ENV2"}, SubTabButton{"ENV3"},
        SubTabButton{"LFO"},  SubTabButton{"MACRO"}, SubTabButton{"QFX"}
    }};
    int activeSubTab = kMacro;

    EnvSubPanel      env1Panel;
    EnvSubPanel      env2Panel;
    EnvSubPanel      env3Panel;
    LFOSubPanel      lfoPanel;
    MacroSubPanel    macroPanel;
    QuickFXSubPanel  quickFXPanel;

    std::array<juce::Component*, kNumSubTabs> subPanels;

    void switchSubTab(int newIndex);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModBar)
};