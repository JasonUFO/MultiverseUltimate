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
        const bool isFM = (modeSelector.getSelectedId() == 2);
        synthEngine.setSynthMode(isFM ? SynthMode::FM : SynthMode::Classic);
        updateVisibility();
        resized();
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
        processorRef.baseWaveform = static_cast<WaveformType>(waveformSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(waveformSelector);

    // ADSR
    setupLabel(envSectionLabel, "ENVELOPE");
    addAndMakeVisible(envSectionLabel);

    setupSlider(attackSlider,  0.001, 5.0,  0.01, 0.4);
    setupSlider(decaySlider,   0.001, 5.0,  0.1,  0.4);
    setupSlider(sustainSlider, 0.0,   1.0,  0.7);
    setupSlider(releaseSlider, 0.001, 10.0, 0.5,  0.4);

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

    setupSlider(cutoffSlider,    20.0, 20000.0, 20000.0, 0.3);
    setupSlider(resonanceSlider, 0.1,  10.0,    0.707,   0.5);

    setupLabel(cutoffLabel,    "Cutoff");
    setupLabel(resonanceLabel, "Resonance");

    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(cutoffLabel);
    addAndMakeVisible(resonanceLabel);

    // Oversampling selector
    setupLabel(oversamplingLabel, "OS");
    addAndMakeVisible(oversamplingLabel);
    oversamplingSelector.addItem("Off",  1);
    oversamplingSelector.addItem("2x",   2);
    oversamplingSelector.addItem("4x",   3);
    oversamplingSelector.addItem("Auto", 4);
    oversamplingSelector.setSelectedId(1, juce::dontSendNotification);
    oversamplingSelector.onChange = [this]
    {
        Filter::OversamplingMode mode = Filter::OversamplingMode::Off;
        switch (oversamplingSelector.getSelectedId())
        {
            case 2: mode = Filter::OversamplingMode::X2;   break;
            case 3: mode = Filter::OversamplingMode::X4;   break;
            case 4: mode = Filter::OversamplingMode::Auto; break;
            default: break;
        }
        synthEngine.setOversamplingMode(mode);
    };
    addAndMakeVisible(oversamplingSelector);

    // APVTS attachments — connect sliders to automatable parameters
    auto& apvts = processorRef.apvts;
    attackAttach    = std::make_unique<SliderAttachment>(apvts, "attack",           attackSlider);
    decayAttach     = std::make_unique<SliderAttachment>(apvts, "decay",            decaySlider);
    sustainAttach   = std::make_unique<SliderAttachment>(apvts, "sustain",          sustainSlider);
    releaseAttach   = std::make_unique<SliderAttachment>(apvts, "release",          releaseSlider);
    cutoffAttach    = std::make_unique<SliderAttachment>(apvts, "filterCutoff",     cutoffSlider);
    resonanceAttach = std::make_unique<SliderAttachment>(apvts, "filterResonance",  resonanceSlider);

    // FM Algorithm selector
    setupLabel(algorithmLabel, "FM ALGORITHM");
    addAndMakeVisible(algorithmLabel);
    for (int i = 1; i <= 8; ++i)
        algorithmSelector.addItem(juce::String(i), i);
    algorithmSelector.setSelectedId(synthEngine.getFMAlgorithm(), juce::dontSendNotification);
    addAndMakeVisible(algorithmSelector);
    algorithmAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "fmAlgorithm", algorithmSelector);

    // FM Operators (4 operators)
    for (int op = 0; op < 4; ++op)
    {
        auto& opControls = fmOps[op];
        const int opNum = op + 1;

        setupLabel(opControls.sectionLabel, "OP " + juce::String(opNum));
        addAndMakeVisible(opControls.sectionLabel);

        setupSlider(opControls.ratioSlider,    0.5, 32.0,  1.0, 0.5);
        setupSlider(opControls.levelSlider,     0.0,  1.0,   1.0);
        setupSlider(opControls.feedbackSlider, 0.0, 1.0,   0.0);
        setupSlider(opControls.attackSlider,   0.001, 5.0,  0.01, 0.4);
        setupSlider(opControls.decaySlider,    0.001, 5.0,  0.1,  0.4);
        setupSlider(opControls.sustainSlider,  0.0,   1.0,  0.7);
        setupSlider(opControls.releaseSlider,   0.001, 10.0, 0.3,  0.4);

        setupLabel(opControls.ratioLabel,     "Ratio");
        setupLabel(opControls.levelLabel,   "Level");
        setupLabel(opControls.feedbackLabel, "Fdbk");
        setupLabel(opControls.attackLabel,  "A");
        setupLabel(opControls.decayLabel,   "D");
        setupLabel(opControls.sustainLabel, "S");
        setupLabel(opControls.releaseLabel, "R");

        addAndMakeVisible(opControls.ratioSlider);
        addAndMakeVisible(opControls.levelSlider);
        addAndMakeVisible(opControls.feedbackSlider);
        addAndMakeVisible(opControls.attackSlider);
        addAndMakeVisible(opControls.decaySlider);
        addAndMakeVisible(opControls.sustainSlider);
        addAndMakeVisible(opControls.releaseSlider);
        addAndMakeVisible(opControls.ratioLabel);
        addAndMakeVisible(opControls.levelLabel);
        addAndMakeVisible(opControls.feedbackLabel);
        addAndMakeVisible(opControls.attackLabel);
        addAndMakeVisible(opControls.decayLabel);
        addAndMakeVisible(opControls.sustainLabel);
        addAndMakeVisible(opControls.releaseLabel);

        opControls.ratioAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Ratio", opControls.ratioSlider);
        opControls.levelAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Level", opControls.levelSlider);
        opControls.feedbackAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Feedback", opControls.feedbackSlider);
        opControls.attackAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Attack", opControls.attackSlider);
        opControls.decayAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Decay", opControls.decaySlider);
        opControls.sustainAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Sustain", opControls.sustainSlider);
        opControls.releaseAttach = std::make_unique<SliderAttachment>(
            apvts, "fmOp" + juce::String(opNum) + "Release", opControls.releaseSlider);
    }

    updateVisibility();
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

void SynthPanel::updateVisibility()
{
    const bool isFM = (modeSelector.getSelectedId() == 2);

    waveformLabel.setVisible(!isFM);
    waveformSelector.setVisible(!isFM);
    envSectionLabel.setVisible(!isFM);
    attackLabel.setVisible(!isFM);
    decayLabel.setVisible(!isFM);
    sustainLabel.setVisible(!isFM);
    releaseLabel.setVisible(!isFM);
    attackSlider.setVisible(!isFM);
    decaySlider.setVisible(!isFM);
    sustainSlider.setVisible(!isFM);
    releaseSlider.setVisible(!isFM);

    algorithmLabel.setVisible(isFM);
    algorithmSelector.setVisible(isFM);

    oversamplingLabel.setVisible(!isFM);
    oversamplingSelector.setVisible(!isFM);

    for (auto& op : fmOps)
    {
        op.sectionLabel.setVisible(isFM);
        op.ratioLabel.setVisible(isFM);
        op.levelLabel.setVisible(isFM);
        op.feedbackLabel.setVisible(isFM);
        op.attackLabel.setVisible(isFM);
        op.decayLabel.setVisible(isFM);
        op.sustainLabel.setVisible(isFM);
        op.releaseLabel.setVisible(isFM);
        op.ratioSlider.setVisible(isFM);
        op.levelSlider.setVisible(isFM);
        op.feedbackSlider.setVisible(isFM);
        op.attackSlider.setVisible(isFM);
        op.decaySlider.setVisible(isFM);
        op.sustainSlider.setVisible(isFM);
        op.releaseSlider.setVisible(isFM);
    }
}

void SynthPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SynthPanel::resized()
{
    auto area = getLocalBounds().reduced(16);
    const int rowH   = 30;
    const int knobSz = 70;
    const int labelH = 20;
    const int gap    = 12;

    const bool isFM = (modeSelector.getSelectedId() == 2);

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

    if (isFM)
    {
        // FM Algorithm
        auto algRow = area.removeFromTop(labelH + rowH);
        algorithmLabel.setBounds(algRow.removeFromTop(labelH));
        algorithmSelector.setBounds(algRow.removeFromTop(rowH));
        area.removeFromTop(gap);

        // FM Operators: 4 rows, each with 7 knobs
        const int opKnobW = 60;
        const int opLabelH = 16;
        const int opRowH = opKnobW + opLabelH + 4;

        for (int op = 0; op < 4; ++op)
        {
            auto opRow = area.removeFromTop(opLabelH + opKnobW + opLabelH);
            fmOps[op].sectionLabel.setBounds(opRow.removeFromTop(opLabelH));

            auto knobs = opRow;
            auto placeFMSlider = [&](juce::Slider& s, juce::Label& l)
            {
                auto col = knobs.removeFromLeft(opKnobW);
                s.setBounds(col.removeFromTop(opKnobW));
                l.setBounds(col.removeFromTop(opLabelH));
                knobs.removeFromLeft(4);
            };
            placeFMSlider(fmOps[op].ratioSlider, fmOps[op].ratioLabel);
            placeFMSlider(fmOps[op].levelSlider, fmOps[op].levelLabel);
            placeFMSlider(fmOps[op].feedbackSlider, fmOps[op].feedbackLabel);
            placeFMSlider(fmOps[op].attackSlider, fmOps[op].attackLabel);
            placeFMSlider(fmOps[op].decaySlider, fmOps[op].decayLabel);
            placeFMSlider(fmOps[op].sustainSlider, fmOps[op].sustainLabel);
            placeFMSlider(fmOps[op].releaseSlider, fmOps[op].releaseLabel);
            area.removeFromTop(8);
        }
    }
    else
    {
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
        area.removeFromTop(4);
        // Oversampling row
        {
            auto osRow = area.removeFromTop(rowH);
            oversamplingLabel.setBounds(osRow.removeFromLeft(30));
            osRow.removeFromLeft(6);
            oversamplingSelector.setBounds(osRow.removeFromLeft(90));
        }
    }
}
