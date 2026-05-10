#pragma once
#include <JuceHeader.h>
#include "ModulationMatrix.h"
#include "../NeuKnob.h"
#include "../MultiverseFlatTheme.h"

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

    std::vector<std::unique_ptr<Row>> rows;
    std::vector<juce::Rectangle<int>> rowBounds;
    int lastConnectionCount = -1;

    void rebuild();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMatrixPanel)
};