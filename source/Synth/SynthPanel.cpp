#include "SynthPanel.h"
#include "../PluginProcessor.h"

SynthPanel::SynthPanel(PluginProcessor& p)
    : processorRef(p), synthEngine(p.getSynthEngine())
{
    // Mode selector
    setupLabel(modeLabel, "MODE");
    addAndMakeVisible(modeLabel);
    modeSelector.addItem("Classic", 1);
    modeSelector.addItem("FM", 2);
    modeSelector.setSelectedId(synthEngine.getSynthMode() == SynthMode::Classic ? 1 : 2,
                               juce::dontSendNotification);
    modeSelector.onChange = [this]
    {
        synthEngine.setSynthMode(modeSelector.getSelectedId() == 1 ? SynthMode::Classic : SynthMode::FM);
    };
    addAndMakeVisible(modeSelector);

    // Waveform selector
    setupLabel(waveformLabel, "WAVEFORM");
    addAndMakeVisible(waveformLabel);
    waveformSelector.addItem("Sine",     1);
    waveformSelector.addItem("Saw",      2);
    waveformSelector.addItem("Square",   3);
    waveformSelector.addItem("Triangle", 4);
    waveformSelector.addItem("Noise",    5);
    waveformSelector.setSelectedId(static_cast<int>(synthEngine.getWaveform()) + 1,
                                   juce::dontSendNotification);
    waveformSelector.onChange = [this]
    {
        synthEngine.setWaveform(static_cast<WaveformType>(waveformSelector.getSelectedId() - 1));
    };
    addAndMakeVisible(waveformSelector);

    // ADSR
    setupLabel(envSectionLabel, "ENVELOPE");
    addAndMakeVisible(envSectionLabel);

    float envA, envD, envS, envR;
    synthEngine.getEnvelopeParams(envA, envD, envS, envR);

    setupSlider(attackSlider,  0.001, 5.0, envA, 0.4);
    setupSlider(decaySlider,   0.001, 5.0, envD, 0.4);
    setupSlider(sustainSlider, 0.0,   1.0, envS);
    setupSlider(releaseSlider, 0.001, 10.0, envR, 0.4);

    attackSlider.onValueChange  = [this] { applyEnvelope(); };
    decaySlider.onValueChange   = [this] { applyEnvelope(); };
    sustainSlider.onValueChange = [this] { applyEnvelope(); };
    releaseSlider.onValueChange = [this] { applyEnvelope(); };

    setupLabel(attackLabel,  "Attack");
    setupLabel(decayLabel,   "Decay");
    setupLabel(sustainLabel, "Sustain");
    setupLabel(releaseLabel, "Release");

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(attackLabel);
    addAndMakeVisible(decayLabel);
    addAndMakeVisible(sustainLabel);
    addAndMakeVisible(releaseLabel);

    // Filter
    setupLabel(filterSectionLabel, "FILTER");
    addAndMakeVisible(filterSectionLabel);

    setupSlider(cutoffSlider,    20.0, 20000.0, processorRef.getBaseFilterCutoff(),    0.3);
    setupSlider(resonanceSlider, 0.1,  10.0,    processorRef.getBaseFilterResonance(), 0.5);

    cutoffSlider.onValueChange    = [this] { processorRef.setBaseFilterCutoff((float)cutoffSlider.getValue()); };
    resonanceSlider.onValueChange = [this] { processorRef.setBaseFilterResonance((float)resonanceSlider.getValue()); };

    setupLabel(cutoffLabel,    "Cutoff");
    setupLabel(resonanceLabel, "Resonance");

    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(cutoffLabel);
    addAndMakeVisible(resonanceLabel);
}

void SynthPanel::setupSlider(juce::Slider& s, double min, double max, double value, double skew)
{
    s.setSliderStyle(juce::Slider::Rotary);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setRange(min, max);
    s.setSkewFactor(skew);
    s.setValue(value, juce::dontSendNotification);
}

void SynthPanel::setupLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
}

void SynthPanel::applyEnvelope()
{
    synthEngine.setEnvelopeParams(
        (float)attackSlider.getValue(),
        (float)decaySlider.getValue(),
        (float)sustainSlider.getValue(),
        (float)releaseSlider.getValue()
    );
}

void SynthPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SynthPanel::resized()
{
    auto area = getLocalBounds().reduced(16);
    const int rowH   = 30;
    const int knobSz = 90;
    const int labelH = 20;
    const int gap    = 12;

    // Row 1: Mode and Waveform selectors
    auto topRow = area.removeFromTop(rowH + labelH + gap);
    {
        auto modeCol = topRow.removeFromLeft(200);
        modeLabel.setBounds(modeCol.removeFromTop(labelH));
        modeSelector.setBounds(modeCol.removeFromTop(rowH));
        topRow.removeFromLeft(24);
        auto waveCol = topRow.removeFromLeft(200);
        waveformLabel.setBounds(waveCol.removeFromTop(labelH));
        waveformSelector.setBounds(waveCol.removeFromTop(rowH));
    }
    area.removeFromTop(gap);

    // Row 2: ADSR
    auto envRow = area.removeFromTop(labelH + knobSz + labelH);
    envSectionLabel.setBounds(envRow.removeFromTop(labelH));
    {
        auto knobRow = envRow;
        auto placeKnob = [&](juce::Slider& knob, juce::Label& lbl)
        {
            auto col = knobRow.removeFromLeft(knobSz + gap);
            knob.setBounds(col.removeFromTop(knobSz));
            lbl.setBounds(col.removeFromTop(labelH));
            knobRow.removeFromLeft(gap);
        };
        placeKnob(attackSlider,  attackLabel);
        placeKnob(decaySlider,   decayLabel);
        placeKnob(sustainSlider, sustainLabel);
        placeKnob(releaseSlider, releaseLabel);
    }
    area.removeFromTop(gap);

    // Row 3: Filter
    auto filterRow = area.removeFromTop(labelH + knobSz + labelH);
    filterSectionLabel.setBounds(filterRow.removeFromTop(labelH));
    {
        auto knobRow = filterRow;
        auto placeKnob = [&](juce::Slider& knob, juce::Label& lbl)
        {
            auto col = knobRow.removeFromLeft(knobSz + gap);
            knob.setBounds(col.removeFromTop(knobSz));
            lbl.setBounds(col.removeFromTop(labelH));
            knobRow.removeFromLeft(gap);
        };
        placeKnob(cutoffSlider,    cutoffLabel);
        placeKnob(resonanceSlider, resonanceLabel);
    }
}
