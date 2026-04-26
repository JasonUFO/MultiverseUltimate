#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"

class PluginProcessor;

class SynthPanel : public juce::Component
{
public:
    explicit SynthPanel(PluginProcessor& p);
    ~SynthPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    SynthEngine& synthEngine;

    juce::Label modeLabel, waveformLabel;
    juce::ComboBox modeSelector, waveformSelector;

    juce::Label envSectionLabel;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    juce::Label filterSectionLabel;
    juce::Slider cutoffSlider, resonanceSlider;
    juce::Label cutoffLabel, resonanceLabel;

    void setupSlider(juce::Slider& s, double min, double max, double value, double skew = 1.0);
    void setupLabel(juce::Label& l, const juce::String& text);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, resonanceAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthPanel)
};
