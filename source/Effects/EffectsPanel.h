#pragma once
#include <JuceHeader.h>

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
    juce::Slider delayTimeSlider, delayFeedbackSlider, delayMixSlider;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayMixLabel;

    juce::Label reverbSectionLabel;
    juce::Slider reverbRoomSlider, reverbDampSlider, reverbWetSlider;
    juce::Label reverbRoomLabel, reverbDampLabel, reverbWetLabel;

    void setupSlider(juce::Slider& s, double min, double max, double value, double skew = 1.0);
    void setupLabel(juce::Label& l, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};
