#pragma once
#include <JuceHeader.h>
#include "../MidiLearnSlider.h"

class PluginProcessor;

class EffectsPanel : public juce::Component
{
public:
    explicit EffectsPanel(PluginProcessor& p);
    ~EffectsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    juce::Label delaySectionLabel;
    MidiLearnSlider delayTimeSlider, delayFeedbackSlider, delayMixSlider;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayMixLabel;

    juce::Label reverbSectionLabel;
    MidiLearnSlider reverbRoomSlider, reverbDampSlider, reverbWetSlider;
    juce::Label reverbRoomLabel, reverbDampLabel, reverbWetLabel;

    // New reverb controls
    MidiLearnSlider reverbPreDelaySlider, reverbLFDampSlider, reverbWidthSlider;
    juce::Label reverbPreDelayLabel, reverbLFDampLabel, reverbWidthLabel;
    juce::ToggleButton reverbFreezeButton;
    juce::Label reverbFreezeLabel;

    void setupSlider(juce::Slider& s, double min, double max, double value, double skew = 1.0);
    void setupLabel(juce::Label& l, const juce::String& text);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttach, delayFeedbackAttach, delayMixAttach;
    std::unique_ptr<SliderAttachment> reverbRoomAttach, reverbDampAttach, reverbWetAttach;
    std::unique_ptr<SliderAttachment> reverbPreDelayAttach, reverbLFDampAttach, reverbWidthAttach;
    std::unique_ptr<ButtonAttachment> reverbFreezeAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};
