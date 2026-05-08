#pragma once

#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../NeuKnob.h"

class PluginProcessor;

class RoutingPanel : public juce::Component,
                     private juce::Timer
{
public:
    explicit RoutingPanel(PluginProcessor& p);
    ~RoutingPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void(int tabIndex)> onSwitchToTab;

    void updateUI();

private:
    PluginProcessor& processorRef;

    // ─── Layer Block ───
    struct LayerBlock : public juce::Component
    {
        int layerIndex;
        PluginProcessor& proc;

        juce::Label nameLabel;
        juce::ComboBox engineSelector;
        juce::ToggleButton muteButton  { "M" };
        juce::ToggleButton soloButton { "S" };
        juce::ComboBox busSelector;
        float levelValue = 1.0f;

        LayerBlock(int idx, PluginProcessor& p);
        void paint(juce::Graphics& g) override;
        void resized() override;
        void updateFromModel();
    };

    std::array<std::unique_ptr<LayerBlock>, 8> layerBlocks;

    // ─── Drums Summary Block ───
    struct DrumsSummaryBlock : public juce::Component
    {
        PluginProcessor& proc;
        juce::Label infoLabel;
        juce::TextButton editButton { "Edit" };

        std::function<void()> onEdit;

        DrumsSummaryBlock(PluginProcessor& p);
        void paint(juce::Graphics& g) override;
        void resized() override;
    };

    DrumsSummaryBlock drumsBlock;

    // ─── Effect Chain Strip (drag-reorder) ───
    struct ChainStrip : public juce::Component
    {
        PluginProcessor& proc;
        int dragSource = -1;
        int dragOver   = -1;
        bool dragging  = false;

        std::function<void()> onClickEffect;

        ChainStrip(PluginProcessor& p);
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;
        juce::Rectangle<int> tileRect(int i) const;
        int slotAt(int x) const;
        static const char* effectName(int id);
    };

    ChainStrip chainStrip;

    // ─── Aux Send Knobs ───
    juce::Label auxSectionLabel;
    NeuKnob auxDelayKnob;
    NeuKnob auxReverbKnob;
    juce::Label auxDelayLabel;
    juce::Label auxReverbLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> auxDelayAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> auxReverbAttach;

    // ─── Section layout rects ───
    juce::Rectangle<int> generatorsRect;
    juce::Rectangle<int> drumsRect;
    juce::Rectangle<int> chainRect;
    juce::Rectangle<int> auxRect;
    juce::Rectangle<int> outputRect;

    void drawConnectionLines(juce::Graphics& g);
    void drawArrowDown(juce::Graphics& g, float x, float y1, float y2, juce::Colour colour);

    void timerCallback() override { updateUI(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutingPanel)
};