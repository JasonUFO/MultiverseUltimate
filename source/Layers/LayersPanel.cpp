#include "LayersPanel.h"
#include "../CyberpunkTheme.h"
#include "LayerEngine.h"
#include "LayerEffectChain.h"
#include "../PluginProcessor.h"
#include "../CyberpunkTheme.h"

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
        juce::ignoreUnused(index);
    };
    addAndMakeVisible(row->presetBtn);

    addAndMakeVisible(row->meterL);
    addAndMakeVisible(row->meterR);

    // Note range sliders
    auto setupRangeSlider = [](juce::Slider& s, double lo, double hi, double val)
    {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 32, 16);
        s.setRange(lo, hi, 1.0);
        s.setValue(val, juce::dontSendNotification);
    };
    setupRangeSlider(row->loNoteSlider, 0, 127, 0);
    setupRangeSlider(row->hiNoteSlider, 0, 127, 127);
    setupRangeSlider(row->loVelSlider,  0, 127, 0);
    setupRangeSlider(row->hiVelSlider,  0, 127, 127);

    row->loNoteSlider.onValueChange = [this, index, row]
    {
        layerManager.getLayer(index).setNoteRange(
            (int)row->loNoteSlider.getValue(), (int)row->hiNoteSlider.getValue());
    };
    row->hiNoteSlider.onValueChange = row->loNoteSlider.onValueChange;

    row->loVelSlider.onValueChange = [this, index, row]
    {
        layerManager.getLayer(index).setVelocityRange(
            (int)row->loVelSlider.getValue(), (int)row->hiVelSlider.getValue());
    };
    row->hiVelSlider.onValueChange = row->loVelSlider.onValueChange;

    addAndMakeVisible(row->loNoteSlider);
    addAndMakeVisible(row->hiNoteSlider);
    addAndMakeVisible(row->loVelSlider);
    addAndMakeVisible(row->hiVelSlider);

    // MIDI channel filter (0=All, 1-16)
    row->midiChSelector.addItem("All Ch", 1);
    for (int ch = 1; ch <= 16; ++ch)
        row->midiChSelector.addItem("Ch " + juce::String(ch), ch + 1);
    row->midiChSelector.setSelectedId(1, juce::dontSendNotification);
    row->midiChSelector.onChange = [this, index, row]
    {
        int selectedId = row->midiChSelector.getSelectedId();
        layerManager.getLayer(index).setMidiChannelFilter(selectedId <= 1 ? 0 : selectedId - 1);
    };
    addAndMakeVisible(row->midiChSelector);

    // FX button — opens CallOutBox with layer effect chain controls
    row->fxButton.onClick = [this, index]() { showFXPopup(index); };
    addAndMakeVisible(row->fxButton);
}

void LayersPanel::paint(juce::Graphics& g)
{
    g.fillAll(CyberpunkTheme::bgBase);

    drawSection(g, getLocalBounds().reduced(8), "LAYERS");
}

void LayersPanel::drawSection(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const
{
    const float cr = 8.0f;
    CyberpunkTheme::drawNeumorphicRect(g, r.toFloat(), cr, 3.0f);
    g.setColour(CyberpunkTheme::bgRaised);
    g.fillRoundedRectangle(r.toFloat(), cr);
    g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
    g.drawRoundedRectangle(r.toFloat().reduced(0.5f), cr, 1.0f);
    g.setColour(CyberpunkTheme::textLabel);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(title, r.getX() + 8, r.getY() + 5, 100, 14, juce::Justification::centredLeft);
}

void LayersPanel::resized()
{
    auto area = getLocalBounds().reduced(8);
    area.removeFromTop(20); // title space

    const int rowH = 40;
    for (int i = 0; i < 8; ++i)
    {
        auto rowArea = area.removeFromTop(rowH).reduced(4);
        auto* r = rows[i].get();

        r->nameLabel.setBounds    (rowArea.removeFromLeft(60));
        r->engineSelector.setBounds(rowArea.removeFromLeft(100));
        rowArea.removeFromLeft(6);
        r->levelSlider.setBounds  (rowArea.removeFromLeft(90));
        rowArea.removeFromLeft(6);
        r->panSlider.setBounds    (rowArea.removeFromLeft(80));
        rowArea.removeFromLeft(6);
        r->muteButton.setBounds   (rowArea.removeFromLeft(28).reduced(0, 6));
        r->soloButton.setBounds   (rowArea.removeFromLeft(28).reduced(0, 6));
        rowArea.removeFromLeft(6);

        // Note range
        r->loNoteSlider.setBounds(rowArea.removeFromLeft(80));
        rowArea.removeFromLeft(4);
        r->hiNoteSlider.setBounds(rowArea.removeFromLeft(80));
        rowArea.removeFromLeft(6);

        // Velocity range
        r->loVelSlider.setBounds(rowArea.removeFromLeft(72));
        rowArea.removeFromLeft(4);
        r->hiVelSlider.setBounds(rowArea.removeFromLeft(72));
        rowArea.removeFromLeft(6);

        // MIDI channel
        r->midiChSelector.setBounds(rowArea.removeFromLeft(76));
        rowArea.removeFromLeft(6);

        // FX button
        r->fxButton.setBounds(rowArea.removeFromLeft(36).reduced(0, 6));
        rowArea.removeFromLeft(6);

        r->presetBtn.setBounds(rowArea.removeFromLeft(54).reduced(0, 6));
        rowArea.removeFromLeft(6);
        r->meterL.setBounds(rowArea.removeFromLeft(16));
        r->meterR.setBounds(rowArea.removeFromLeft(16));

        area.removeFromTop(4);
    }
}

void LayersPanel::buttonClicked(juce::Button* button) { juce::ignoreUnused(button); }
void LayersPanel::comboBoxChanged(juce::ComboBox* comboBox) { juce::ignoreUnused(comboBox); }

void LayersPanel::updateUI()
{
    for (int i = 0; i < 8; ++i)
    {
        auto& layer = layerManager.getLayer(i);
        auto* r = rows[i].get();

        r->engineSelector.setSelectedId(static_cast<int>(layer.getEngineType()) + 2);
        r->levelSlider.setValue(layer.getLevel(), juce::dontSendNotification);
        r->panSlider.setValue(layer.getPan(), juce::dontSendNotification);
        r->loNoteSlider.setValue(layer.getLoNote(), juce::dontSendNotification);
        r->hiNoteSlider.setValue(layer.getHiNote(), juce::dontSendNotification);
        r->loVelSlider.setValue(layer.getLoVel(), juce::dontSendNotification);
        r->hiVelSlider.setValue(layer.getHiVel(), juce::dontSendNotification);
        const int chFilter = layer.getMidiChannelFilter();
        r->midiChSelector.setSelectedId(chFilter == 0 ? 1 : chFilter + 1, juce::dontSendNotification);
    }
}

//==============================================================================
void LayersPanel::showFXPopup(int layerIndex)
{
    auto& chain = layerManager.getLayer(layerIndex).getEffectChain();

    static const char* names[LayerEffectChain::NumEffects] = {
        "Chorus", "Distortion", "EQ", "Compressor", "Delay", "Reverb"
    };

    // Build popup content component
    auto* content = new juce::Component();
    content->setSize(320, 200);

    for (int fx = 0; fx < LayerEffectChain::NumEffects; ++fx)
    {
        const int x = 8 + (fx % 3) * 104;
        const int y = 8 + (fx / 3) * 90;

        auto* label = new juce::Label();
        label->setText(names[fx], juce::dontSendNotification);
        label->setBounds(x, y, 90, 18);
        content->addAndMakeVisible(label);

        auto* toggle = new juce::ToggleButton("On");
        toggle->setToggleState(chain.isEnabled(fx), juce::dontSendNotification);
        toggle->setBounds(x, y + 20, 70, 20);
        const int fxIdx = fx;
        toggle->onStateChange = [toggle, &chain, fxIdx]()
        {
            chain.setEnabled(fxIdx, toggle->getToggleState());
        };
        content->addAndMakeVisible(toggle);

        auto* mixSlider = new juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        mixSlider->setRange(0.0, 1.0);
        mixSlider->setValue(chain.getMix(fx), juce::dontSendNotification);
        mixSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 36, 18);
        mixSlider->setBounds(x, y + 46, 96, 22);
        mixSlider->onValueChange = [mixSlider, &chain, fxIdx]()
        {
            chain.setMix(fxIdx, (float)mixSlider->getValue());
        };
        content->addAndMakeVisible(mixSlider);
    }

    auto* fxBtn = rows[layerIndex]->fxButton.isVisible() ? &rows[layerIndex]->fxButton : nullptr;
    if (fxBtn)
    {
        juce::CallOutBox::launchAsynchronously(
            std::unique_ptr<juce::Component>(content), fxBtn->getScreenBounds(), nullptr);
    }
    else
    {
        delete content;
    }
}
