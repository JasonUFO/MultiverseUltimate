#pragma once
#include <JuceHeader.h>
#include "MacroManager.h"
#include "../MultiverseTheme.h"

class PluginProcessor;

class MacroPanel : public juce::Component, private juce::Timer
{
public:
    explicit MacroPanel(PluginProcessor& p);
    ~MacroPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    juce::Rectangle<int> macroSectionRect;

    struct MacroKnobGroup
    {
        juce::Slider     knob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::Label      nameLabel;
        juce::TextButton assignButton;
    };

    std::array<MacroKnobGroup, MacroManager::NUM_MACROS> macros;

    void timerCallback() override;
    void showAssignmentList(int macroIdx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroPanel)
};
