#pragma once
#include <JuceHeader.h>
#include "../Macros/MacroManager.h"
#include "../MultiverseFlatTheme.h"

class PluginProcessor;

class PerformancePanel : public juce::Component, private juce::Timer
{
public:
    explicit PerformancePanel(PluginProcessor& p);
    ~PerformancePanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& proc;

    // Info strip
    juce::Label titleLabel;
    juce::Label bpmLabel;

    // Large macro knobs
    struct MacroGroup
    {
        juce::Slider knob;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        juce::Label  nameLabel;
    };
    std::array<MacroGroup, MacroManager::NUM_MACROS> macros;

    // XY Pad — maps macro1 (X) and macro2 (Y)
    class XYPad : public juce::Component
    {
    public:
        explicit XYPad(PluginProcessor& p);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void updateFromAPVTS();
    private:
        PluginProcessor& proc;
        float dotX = 0.5f, dotY = 0.5f;
        void updateFromMouse(juce::Point<float> pos);
    };

    XYPad xyPad;

    juce::Rectangle<int> infoCardRect;
    juce::Rectangle<int> macroCardRect;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerformancePanel)
};
