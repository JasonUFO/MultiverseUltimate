#pragma once
#include <JuceHeader.h>
#include "LayerManager.h"

class PluginProcessor; // forward

class LayersPanel : public juce::Component,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener
{
public:
    LayersPanel(PluginProcessor& p, LayerManager& mgr);
    ~LayersPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;

    void updateUI(); // call when layer count or state changes

private:
    PluginProcessor& processorRef;
    LayerManager& layerManager;

    struct LayerRow
    {
        juce::Label        nameLabel;
        juce::ComboBox     engineSelector;
        juce::Slider      levelSlider;
        juce::Slider      panSlider;
        juce::ToggleButton muteButton, soloButton;
        juce::TextButton   presetBtn;
        juce::Label        meterL, meterR;
    };
    std::array<std::unique_ptr<LayerRow>, 8> rows;

    void createRow(int index);
    void drawSection(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const;
};
