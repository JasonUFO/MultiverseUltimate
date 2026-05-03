#include "LayersPanel.h"
#include "LayerEngine.h"
#include "../PluginProcessor.h"
#include "../MultiverseTheme.h"

LayersPanel::LayersPanel(PluginProcessor& p, LayerManager& mgr)
    : processorRef(p), layerManager(mgr)
{
    for (int i = 0; i < 8; ++i)
        createRow(i);

    updateUI();
}

void LayersPanel::createRow(int index)
{
    rows[index] = std::make_unique<LayerRow>();
    auto* row = rows[index].get();

    row->nameLabel.setText("Layer " + juce::String(index + 1), juce::dontSendNotification);
    row->nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(row->nameLabel);

    row->engineSelector.addItem("Off",      1);
    row->engineSelector.addItem("Synth",    2);
    row->engineSelector.addItem("Granular", 3);
    row->engineSelector.addItem("Sampler",  4);
    row->engineSelector.setSelectedId(2, juce::dontSendNotification);
    row->engineSelector.onChange = [this, index, row]
    {
        auto type = static_cast<LayerEngineType>(row->engineSelector.getSelectedId() - 2);
        layerManager.getLayer(index).setEngineType(type);
    };
    addAndMakeVisible(row->engineSelector);

    row->levelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    row->levelSlider.setRange(0.0, 1.0);
    row->levelSlider.setValue(1.0, juce::dontSendNotification);
    row->levelSlider.onValueChange = [this, index, row]
    {
        layerManager.getLayer(index).setLevel(static_cast<float>(row->levelSlider.getValue()));
    };
    addAndMakeVisible(row->levelSlider);

    row->panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    row->panSlider.setRange(-1.0, 1.0);
    row->panSlider.setValue(0.0, juce::dontSendNotification);
    row->panSlider.onValueChange = [this, index, row]
    {
        layerManager.getLayer(index).setPan(static_cast<float>(row->panSlider.getValue()));
    };
    addAndMakeVisible(row->panSlider);

    row->muteButton.setButtonText("M");
    row->muteButton.onClick = [this, index, row]
    {
        layerManager.getLayer(index).setMute(row->muteButton.getToggleState());
    };
    addAndMakeVisible(row->muteButton);

    row->soloButton.setButtonText("S");
    row->soloButton.onClick = [this, index, row]
    {
        layerManager.getLayer(index).setSolo(row->soloButton.getToggleState());
    };
    addAndMakeVisible(row->soloButton);

    row->presetBtn.setButtonText("Preset");
    row->presetBtn.onClick = [this, index]
    {
        // TODO: open layer preset browser
        juce::ignoreUnused(index);
    };
    addAndMakeVisible(row->presetBtn);

    addAndMakeVisible(row->meterL);
    addAndMakeVisible(row->meterR);
}

void LayersPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseTheme::bgBase);

    drawSection(g, getLocalBounds().reduced(8), "LAYERS");
}

void LayersPanel::drawSection(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const
{
    const float cr = 8.0f;
    MultiverseTheme::drawNeumorphicRect(g, r.toFloat(), cr, 3.0f);
    g.setColour(MultiverseTheme::bgRaised);
    g.fillRoundedRectangle(r.toFloat(), cr);
    g.setColour(MultiverseTheme::shadowLight.withAlpha(0.3f));
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), cr, 1.0f);
    g.setColour(MultiverseTheme::textLabel);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(title, r.getX() + 8, r.getY() + 5, 100, 14, juce::Justification::centredLeft);
}

void LayersPanel::resized()
{
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(20); // title space

    int rowH = 40;
    for (int i = 0; i < 8; ++i)
    {
        auto rowArea = area.removeFromTop(rowH).reduced(4);
        auto* r = rows[i].get();

        r->nameLabel.setBounds(rowArea.removeFromLeft(80));
        r->engineSelector.setBounds(rowArea.removeFromLeft(120));
        rowArea.removeFromLeft(8);
        r->levelSlider.setBounds(rowArea.removeFromLeft(100));
        rowArea.removeFromLeft(8);
        r->panSlider.setBounds(rowArea.removeFromLeft(100));
        rowArea.removeFromLeft(8);
        r->muteButton.setBounds(rowArea.removeFromLeft(40));
        r->soloButton.setBounds(rowArea.removeFromLeft(40));
        rowArea.removeFromLeft(8);
        r->presetBtn.setBounds(rowArea.removeFromLeft(60));
        rowArea.removeFromLeft(8);
        r->meterL.setBounds(rowArea.removeFromLeft(20));
        r->meterR.setBounds(rowArea.removeFromLeft(20));

        area.removeFromTop(4);
    }
}

void LayersPanel::buttonClicked(juce::Button* button) {}
void LayersPanel::comboBoxChanged(juce::ComboBox* comboBox) {}

void LayersPanel::updateUI()
{
    for (int i = 0; i < 8; ++i)
    {
        auto& layer = layerManager.getLayer(i);
        auto* r = rows[i].get();

        r->engineSelector.setSelectedId(static_cast<int>(layer.getEngineType()) + 2);
        r->levelSlider.setValue(layer.getLevel(), juce::dontSendNotification);
        r->panSlider.setValue(layer.getPan(), juce::dontSendNotification);
    }
}
