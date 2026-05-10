#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../SkinManager.h"
#include "../NeuKnob.h"

class PluginProcessor;

class GlobalPanel : public juce::Component
{
public:
    explicit GlobalPanel (PluginProcessor& p);
    ~GlobalPanel() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    // Voice Mode
    juce::Label voiceModeLabel;
    juce::ComboBox voiceModeSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttach;

    // Portamento
    NeuKnob portamentoSlider;
    juce::Label portamentoLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portamentoAttach;

    // Porta Always
    juce::ToggleButton portaAlwaysButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> portaAlwaysAttach;

    // MPE
    juce::ToggleButton mpeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mpeAttach;

    // Unison
    juce::Label unisonVoicesLabel, unisonDetuneLabel, unisonWidthLabel, unisonSpreadLabel;
    juce::ComboBox unisonVoicesBox;
    NeuKnob unisonDetuneSlider, unisonWidthSlider;
    juce::ComboBox unisonSpreadSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> unisonVoicesAttach, unisonSpreadAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonDetuneAttach, unisonWidthAttach;

    // Chord/Strum
    juce::ToggleButton chordEnableButton;
    juce::ComboBox chordShapeSelector;
    NeuKnob chordStrumSlider;
    juce::Label chordShapeLabel, chordStrumLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chordEnableAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> chordShapeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chordStrumAttach;

    // Scale (Key + Scale selector)
    juce::Label scaleKeyLabel, scaleNameLabel;
    juce::ComboBox scaleKeySelector, scaleNameSelector;

    // Quality (moved from header)
    juce::Label qualityLabel;
    juce::ComboBox qualitySelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualityAttach;

    // FX Mode (moved from header)
    juce::ToggleButton fxModeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fxModeAttach;

    // Skin selector
    juce::Label skinLabel;
    juce::ComboBox skinSelector;
    void skinChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};