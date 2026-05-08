#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../Macros/MacroManager.h"

class PluginProcessor;

class BottomBar : public juce::Component, private juce::Timer
{
public:
    explicit BottomBar(PluginProcessor& p);
    ~BottomBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    static constexpr int KEYBOARD_H = 64;
    static constexpr int KNOB_SIZE  = 32;

    struct MacroKnobGroup
    {
        juce::Slider     knob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::Label      nameLabel;
    };

    std::array<MacroKnobGroup, MacroManager::NUM_MACROS> macros;
    juce::MidiKeyboardComponent keyboard;

    void timerCallback() override;
    void showAssignmentList(int macroIdx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomBar)
};