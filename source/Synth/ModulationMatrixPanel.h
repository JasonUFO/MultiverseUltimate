#pragma once
#include <JuceHeader.h>
#include "ModulationMatrix.h"

class ModulationMatrixPanel : public juce::Component,
                               public juce::Timer
{
public:
    explicit ModulationMatrixPanel(ModulationMatrix& matrix);
    ~ModulationMatrixPanel() override;
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    struct Row : public juce::Component
    {
        juce::ComboBox sourceBox, targetBox;
        juce::Slider amountSlider;
        juce::TextButton deleteButton{ "X" };

        std::function<void()> onDelete;
        std::function<void(ModSourceType, ModTargetType, float)> onChange;

        Row();
        void resized() override;
    };

    ModulationMatrix& matrix;
    juce::Label titleLabel;
    juce::TextButton addButton{ "+" };
    std::vector<std::unique_ptr<Row>> rows;
    int lastConnectionCount = -1;

    void rebuild();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMatrixPanel)
};
