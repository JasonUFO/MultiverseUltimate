#pragma once
#include <JuceHeader.h>
#include "ModulationMatrix.h"
#include "../NeuKnob.h"

class ModulationMatrixPanel : public juce::Component,
                                public juce::Timer
{
public:
    explicit ModulationMatrixPanel(PluginProcessor& p, ModulationMatrix& matrix);
    ~ModulationMatrixPanel() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    // ── LFO bank row ─────────────────────────────────────────────────────────
    struct LFORow : public juce::Component
    {
        juce::Label      label;
        juce::Slider     rateSlider;
        juce::ComboBox   shapeCombo;
        juce::ToggleButton syncButton { "SYNC" };
        juce::ComboBox   syncDivCombo;

        std::unique_ptr<juce::SliderParameterAttachment>   rateAttachment;
        std::unique_ptr<juce::ComboBoxParameterAttachment> shapeAttachment;
        std::unique_ptr<juce::ButtonParameterAttachment>   syncAttachment;
        std::unique_ptr<juce::ComboBoxParameterAttachment> syncDivAttachment;

        LFORow(int index, juce::AudioProcessorValueTreeState& apvts);
        void resized() override;
    };

    // ── Mod connection row ────────────────────────────────────────────────────
    struct Row : public juce::Component
    {
        juce::ComboBox sourceBox, targetBox;
        NeuKnob amountSlider;
        juce::TextButton deleteButton{ "X" };

        std::function<void()> onDelete;
        std::function<void(ModSourceType, ModTargetType, float)> onChange;

        Row();
        void resized() override;
    };

    PluginProcessor&  processorRef;
    ModulationMatrix& matrix;

    juce::Label titleLabel;
    juce::TextButton addButton{ "+" };

    std::array<std::unique_ptr<LFORow>, 8> lfoRows;

    std::vector<std::unique_ptr<Row>> rows;
    std::vector<juce::Rectangle<int>> rowBounds;
    int lastConnectionCount = -1;

    void rebuild();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMatrixPanel)
};
