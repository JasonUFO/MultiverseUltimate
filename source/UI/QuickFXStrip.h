#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../NeuKnob.h"

class PluginProcessor;

class QuickFXStrip : public juce::Component
{
public:
    explicit QuickFXStrip(PluginProcessor& p);
    ~QuickFXStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    static constexpr int KNOB_SZ = 28;
    static constexpr int LABEL_H = 12;
    static constexpr int TOGGLE_H = 18;
    static constexpr int COMBO_H = 20;
    static constexpr int SECTION_GAP = 6;

    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttach  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct KnobGroup
    {
        NeuKnob knob;
        std::unique_ptr<SliderAttach> attach;
        juce::Label label { {}, {} };
    };

    // Section enable toggles
    juce::ToggleButton filterModEnable  { "ON" };
    juce::ToggleButton ampModEnable     { "ON" };
    juce::ToggleButton mainFilterEnable { "ON" };

    std::unique_ptr<ButtonAttach> filterModEnableAttach;
    std::unique_ptr<ButtonAttach> ampModEnableAttach;
    std::unique_ptr<ButtonAttach> mainFilterEnableAttach;

    // Filter Modifier knobs
    KnobGroup fmCutoff, fmResonance, fmEnvDepth;

    // Amp Modifier knobs
    KnobGroup amVolume, amPan;

    // Delay knobs
    KnobGroup delayMix, delayTime, delayFeedback;

    // Reverb knobs
    KnobGroup reverbWet, reverbRoom, reverbDamp;

    // Main Filter
    KnobGroup mfCutoff, mfResonance;
    juce::ComboBox mfTypeCombo;
    std::unique_ptr<ComboAttach> mfTypeAttach;

    // Section headers
    juce::Label filterModLabel  { {}, "FILTER MOD" };
    juce::Label ampModLabel     { {}, "AMP MOD" };
    juce::Label delayLabel      { {}, "DELAY" };
    juce::Label reverbLabel     { {}, "REVERB" };
    juce::Label mainFilterLabel { {}, "MAIN FILTER" };

    // Section rectangles (set in resized, used in paint)
    juce::Rectangle<int> filterModRect, ampModRect, delayRect, reverbRect, mainFilterRect;

    void setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& shortName);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuickFXStrip)
};