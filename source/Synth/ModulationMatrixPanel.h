#pragma once
#include <JuceHeader.h>
#include "ModulationMatrix.h"

class ModulationMatrixPanel : public juce::Component,
                             public juce::Timer
{
public:
    explicit ModulationMatrixPanel (ModulationMatrix& matrix);
    ~ModulationMatrixPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    class SourceComponent : public juce::Component
    {
    public:
        int sourceIndex = 0;
        ModSourceType sourceType = ModSourceType::LFO1;
        ModulationMatrix& matrix;

        std::function<void()> onDragStarted;
        std::function<void(ModSourceType, float)> onDragEnded;

        explicit SourceComponent (ModulationMatrix& m);
        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseDrag (const juce::MouseEvent& e) override;
        void mouseUp (const juce::MouseEvent& e) override;
    };

    class TargetComponent : public juce::Component
    {
    public:
        int targetIndex = 0;
        ModTargetType targetType = ModTargetType::FilterCutoff;
        ModulationMatrix& matrix;

        std::function<void()> onDropped;
        std::function<void()> onClicked;

        explicit TargetComponent (ModulationMatrix& m);
        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void mouseEnter (const juce::MouseEvent& e) override;
        void mouseExit (const juce::MouseEvent& e) override;

    private:
        bool isDragOver = false;
    };

    class ConnectionItem : public juce::Component,
                         private juce::Slider::Listener
    {
    public:
        int connectionId = 0;
        ModulationMatrix& matrix;

        juce::Slider amountSlider;
        juce::Label sourceLabel;
        juce::Label targetLabel;
        juce::TextButton deleteButton { "X" };
        float currentAmount = 0.0f;

        std::function<void()> onDelete;
        std::function<void(float)> onAmountChanged;

        explicit ConnectionItem (ModulationMatrix& m);
        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;
        void resized() override;
        void sliderValueChanged (juce::Slider* slider) override;
    };

    ModulationMatrix& matrix;

    std::vector<std::unique_ptr<SourceComponent>> sourceComponents;
    std::vector<std::unique_ptr<TargetComponent>> targetComponents;
    std::vector<std::unique_ptr<ConnectionItem>> connectionItems;

    juce::Label titleLabel;
    juce::TextButton addConnectionButton { "+" };

    juce::Rectangle<int> sourceArea;
    juce::Rectangle<int> targetArea;
    juce::Rectangle<int> connectionArea;

    juce::Point<int> dragStartPoint;
    bool isDragging = false;
    int activeConnectionId = -1;

    int hoveredSourceIndex = -1;
    int hoveredTargetIndex = -1;

    void updateLayout();
    void createConnection(int sourceIndex, int targetIndex);
    void refreshConnections();
    int countConnectionsForSource (ModSourceType source) const;
    int countConnectionsForTarget (ModTargetType target) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulationMatrixPanel)
};