#include "SynthPanel.h"
#include "../PluginProcessor.h"

SynthPanel::SynthPanel(PluginProcessor& p)
    : processorRef(p), synthEngine(p.getSynthEngine())
{
    setLookAndFeel(&synthLookAndFeel);

    // Mode selector
    setupLabel(modeLabel, "MODE");
    addAndMakeVisible(modeLabel);
    modeSelector.addItem("Classic", 1);
    modeSelector.addItem("FM", 2);
    modeSelector.setSelectedId(synthEngine.getSynthMode() == SynthMode::Classic ? 1 : 2,
                               juce::dontSendNotification);
    modeSelector.onChange = [this]
    {
        synthEngine.setSynthMode(modeSelector.getSelectedId() == 2 ? SynthMode::FM : SynthMode::Classic);
        updateVisibility();
        resized();
        repaint();
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
        const auto wf = static_cast<WaveformType>(waveformSelector.getSelectedId() - 1);
        processorRef.baseWaveform = wf;
        oscDisplay.setWaveform(wf);
    };
    addAndMakeVisible(waveformSelector);

    // Visualisations
    oscDisplay.setWaveform(processorRef.baseWaveform);
    addAndMakeVisible(oscDisplay);
    addAndMakeVisible(lfoDisplay);

    // Preset buttons
    savePresetButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Save Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("MultiversePresets"),
            "*.mvpreset");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.getFullPathName().isEmpty()) return;
                auto file = f.withFileExtension(".mvpreset");
                juce::MemoryBlock state;
                processorRef.getStateInformation(state);
                file.replaceWithData(state.getData(), state.getSize());
            });
    };
    addAndMakeVisible(savePresetButton);

    loadPresetButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Load Preset",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("MultiversePresets"),
            "*.mvpreset");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (!f.existsAsFile()) return;
                juce::MemoryBlock state;
                f.loadFileAsData(state);
                processorRef.setStateInformation(state.getData(), (int)state.getSize());
            });
    };
    addAndMakeVisible(loadPresetButton);

    // ADSR
    setupSlider(attackSlider,  0.001, 5.0,  0.01, 0.4);
    setupSlider(decaySlider,   0.001, 5.0,  0.1,  0.4);
    setupSlider(sustainSlider, 0.0,   1.0,  0.7);
    setupSlider(releaseSlider, 0.001, 10.0, 0.5,  0.4);
    setupLabel(attackLabel,  "Attack");
    setupLabel(decayLabel,   "Decay");
    setupLabel(sustainLabel, "Sustain");
    setupLabel(releaseLabel, "Release");
    addAndMakeVisible(attackSlider);  addAndMakeVisible(attackLabel);
    addAndMakeVisible(decaySlider);   addAndMakeVisible(decayLabel);
    addAndMakeVisible(sustainSlider); addAndMakeVisible(sustainLabel);
    addAndMakeVisible(releaseSlider); addAndMakeVisible(releaseLabel);

    // Filter
    setupSlider(cutoffSlider,    20.0, 20000.0, 20000.0, 0.3);
    setupSlider(resonanceSlider, 0.1,  10.0,    0.707,   0.5);
    setupLabel(cutoffLabel,    "Cutoff");
    setupLabel(resonanceLabel, "Resonance");
    addAndMakeVisible(cutoffSlider);    addAndMakeVisible(cutoffLabel);
    addAndMakeVisible(resonanceSlider); addAndMakeVisible(resonanceLabel);

    // Oversampling
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

    // APVTS attachments
    auto& apvts = processorRef.apvts;
    attackAttach    = std::make_unique<SliderAttachment>(apvts, "attack",          attackSlider);
    decayAttach     = std::make_unique<SliderAttachment>(apvts, "decay",           decaySlider);
    sustainAttach   = std::make_unique<SliderAttachment>(apvts, "sustain",         sustainSlider);
    releaseAttach   = std::make_unique<SliderAttachment>(apvts, "release",         releaseSlider);
    cutoffAttach    = std::make_unique<SliderAttachment>(apvts, "filterCutoff",    cutoffSlider);
    resonanceAttach = std::make_unique<SliderAttachment>(apvts, "filterResonance", resonanceSlider);
    
    // Initialize MidiLearnSliders
    attackSlider.init(processorRef, "attack");
    decaySlider.init(processorRef, "decay");
    sustainSlider.init(processorRef, "sustain");
    releaseSlider.init(processorRef, "release");
    cutoffSlider.init(processorRef, "filterCutoff");
    resonanceSlider.init(processorRef, "filterResonance");

    // FM Algorithm
    setupLabel(algorithmLabel, "FM ALGORITHM");
    addAndMakeVisible(algorithmLabel);
    for (int i = 1; i <= 8; ++i)
        algorithmSelector.addItem(juce::String(i), i);
    algorithmSelector.setSelectedId(synthEngine.getFMAlgorithm(), juce::dontSendNotification);
    addAndMakeVisible(algorithmSelector);
    algorithmAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "fmAlgorithm", algorithmSelector);

    // FM Operators
    for (int op = 0; op < 4; ++op)
    {
        auto& c = fmOps[op];
        const int n = op + 1;

        setupLabel(c.sectionLabel, "OP " + juce::String(n));
        addAndMakeVisible(c.sectionLabel);

        setupSlider(c.ratioSlider,    0.5,  32.0, 1.0,  0.5);
        setupSlider(c.levelSlider,    0.0,   1.0,  1.0);
        setupSlider(c.feedbackSlider, 0.0,   1.0,  0.0);
        setupSlider(c.attackSlider,   0.001, 5.0,  0.01, 0.4);
        setupSlider(c.decaySlider,    0.001, 5.0,  0.1,  0.4);
        setupSlider(c.sustainSlider,  0.0,   1.0,  0.7);
        setupSlider(c.releaseSlider,  0.001, 10.0, 0.3,  0.4);

        setupLabel(c.ratioLabel,    "Ratio");
        setupLabel(c.levelLabel,    "Level");
        setupLabel(c.feedbackLabel, "Fdbk");
        setupLabel(c.attackLabel,   "A");
        setupLabel(c.decayLabel,    "D");
        setupLabel(c.sustainLabel,  "S");
        setupLabel(c.releaseLabel,  "R");

        addAndMakeVisible(c.ratioSlider);    addAndMakeVisible(c.ratioLabel);
        addAndMakeVisible(c.levelSlider);    addAndMakeVisible(c.levelLabel);
        addAndMakeVisible(c.feedbackSlider); addAndMakeVisible(c.feedbackLabel);
        addAndMakeVisible(c.attackSlider);   addAndMakeVisible(c.attackLabel);
        addAndMakeVisible(c.decaySlider);    addAndMakeVisible(c.decayLabel);
        addAndMakeVisible(c.sustainSlider);  addAndMakeVisible(c.sustainLabel);
        addAndMakeVisible(c.releaseSlider);  addAndMakeVisible(c.releaseLabel);

        const juce::String pfx = "fmOp" + juce::String(n);
        c.ratioAttach    = std::make_unique<SliderAttachment>(apvts, pfx + "Ratio",    c.ratioSlider);
        c.levelAttach    = std::make_unique<SliderAttachment>(apvts, pfx + "Level",    c.levelSlider);
        c.feedbackAttach = std::make_unique<SliderAttachment>(apvts, pfx + "Feedback", c.feedbackSlider);
        c.attackAttach   = std::make_unique<SliderAttachment>(apvts, pfx + "Attack",   c.attackSlider);
        c.decayAttach    = std::make_unique<SliderAttachment>(apvts, pfx + "Decay",    c.decaySlider);
        c.sustainAttach  = std::make_unique<SliderAttachment>(apvts, pfx + "Sustain",  c.sustainSlider);
        c.releaseAttach  = std::make_unique<SliderAttachment>(apvts, pfx + "Release",  c.releaseSlider);
        
        // Initialize MidiLearnSliders for FM operator
        c.ratioSlider.init(processorRef, pfx + "Ratio");
        c.levelSlider.init(processorRef, pfx + "Level");
        c.feedbackSlider.init(processorRef, pfx + "Feedback");
        c.attackSlider.init(processorRef, pfx + "Attack");
        c.decaySlider.init(processorRef, pfx + "Decay");
        c.sustainSlider.init(processorRef, pfx + "Sustain");
        c.releaseSlider.init(processorRef, pfx + "Release");
    }

    // Tooltips
    modeSelector.setTooltip      ("Synthesis mode: Classic (subtractive) or FM");
    waveformSelector.setTooltip  ("Oscillator waveform: Sine / Saw / Square / Triangle / Noise");
    savePresetButton.setTooltip  ("Save current state to a preset file");
    loadPresetButton.setTooltip  ("Load a previously saved preset file");
    attackSlider.setTooltip      ("Envelope Attack: time to reach full volume (1ms–5s)");
    decaySlider.setTooltip       ("Envelope Decay: time to fall to sustain level (1ms–5s)");
    sustainSlider.setTooltip     ("Envelope Sustain: held volume level (0–100%)");
    releaseSlider.setTooltip     ("Envelope Release: time to silence after note-off (1ms–10s)");
    cutoffSlider.setTooltip      ("Filter Cutoff: frequency where filtering begins (20Hz–20kHz). Lower = darker sound.");
    resonanceSlider.setTooltip   ("Filter Resonance: emphasis at cutoff. High values add a ringing tone.");
    oversamplingSelector.setTooltip("Filter Oversampling: Off / 2x / 4x / Auto (Auto enables 2× above 5kHz)");
    algorithmSelector.setTooltip ("FM Algorithm: operator routing topology (1–8)");
    for (int op = 0; op < 4; ++op)
    {
        auto& c = fmOps[op];
        const auto pfx = "OP " + juce::String(op + 1) + " — ";
        c.ratioSlider.setTooltip   (pfx + "Frequency ratio relative to base pitch (0.5–32)");
        c.levelSlider.setTooltip   (pfx + "Operator output amplitude (0–100%)");
        c.feedbackSlider.setTooltip(pfx + "Self-modulation / feedback depth (0–100%)");
        c.attackSlider.setTooltip  (pfx + "Operator envelope Attack (1ms–5s)");
        c.decaySlider.setTooltip   (pfx + "Operator envelope Decay (1ms–5s)");
        c.sustainSlider.setTooltip (pfx + "Operator envelope Sustain level (0–100%)");
        c.releaseSlider.setTooltip (pfx + "Operator envelope Release (1ms–10s)");
    }

    updateVisibility();
}

//==============================================================================
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

void SynthPanel::drawSection(juce::Graphics& g, juce::Rectangle<int> r,
                              const juce::String& title) const
{
    g.setColour(juce::Colour(0xff0e1c24));
    g.fillRoundedRectangle(r.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xff2c3e47));
    g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.0f);
    g.setColour(juce::Colour(0xff4fc3f7));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(title, r.getX() + 8, r.getY() + 5, 100, 14, juce::Justification::centredLeft);
}

//==============================================================================
void SynthPanel::updateVisibility()
{
    const bool isFM = (modeSelector.getSelectedId() == 2);

    waveformLabel.setVisible(!isFM);
    waveformSelector.setVisible(!isFM);
    oscDisplay.setVisible(!isFM);
    lfoDisplay.setVisible(!isFM);

    attackLabel.setVisible(!isFM);  attackSlider.setVisible(!isFM);
    decayLabel.setVisible(!isFM);   decaySlider.setVisible(!isFM);
    sustainLabel.setVisible(!isFM); sustainSlider.setVisible(!isFM);
    releaseLabel.setVisible(!isFM); releaseSlider.setVisible(!isFM);

    cutoffLabel.setVisible(!isFM);       cutoffSlider.setVisible(!isFM);
    resonanceLabel.setVisible(!isFM);    resonanceSlider.setVisible(!isFM);
    oversamplingLabel.setVisible(!isFM); oversamplingSelector.setVisible(!isFM);

    algorithmLabel.setVisible(isFM);
    algorithmSelector.setVisible(isFM);

    for (auto& c : fmOps)
    {
        c.sectionLabel.setVisible(isFM);
        c.ratioLabel.setVisible(isFM);    c.ratioSlider.setVisible(isFM);
        c.levelLabel.setVisible(isFM);    c.levelSlider.setVisible(isFM);
        c.feedbackLabel.setVisible(isFM); c.feedbackSlider.setVisible(isFM);
        c.attackLabel.setVisible(isFM);   c.attackSlider.setVisible(isFM);
        c.decayLabel.setVisible(isFM);    c.decaySlider.setVisible(isFM);
        c.sustainLabel.setVisible(isFM);  c.sustainSlider.setVisible(isFM);
        c.releaseLabel.setVisible(isFM);  c.releaseSlider.setVisible(isFM);
    }
}

//==============================================================================
void SynthPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a2530));

    const bool isFM = (modeSelector.getSelectedId() == 2);

    if (!isFM)
    {
        drawSection(g, oscSectionRect,    "OSC");
        drawSection(g, filterSectionRect, "FILTER");
        drawSection(g, envSectionRect,    "ENV");
    }

    // Mode badge
    const juce::Colour badgeCol = isFM ? juce::Colour(0xff7986cb) : juce::Colour(0xff4fc3f7);
    g.setColour(badgeCol.withAlpha(0.18f));
    g.fillRoundedRectangle(modeBadgeRect.toFloat(), 4.0f);
    g.setColour(badgeCol);
    g.drawRoundedRectangle(modeBadgeRect.toFloat(), 4.0f, 1.0f);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(isFM ? "FM" : "CLASSIC", modeBadgeRect, juce::Justification::centred);
}

//==============================================================================
void SynthPanel::resized()
{
    auto area = getLocalBounds().reduced(16);
    const bool isFM = (modeSelector.getSelectedId() == 2);

    // Header
    {
        auto hdr = area.removeFromTop(46);

        auto modeCol = hdr.removeFromLeft(180);
        modeLabel.setBounds(modeCol.removeFromTop(18));
        modeSelector.setBounds(modeCol.removeFromTop(26));
        hdr.removeFromLeft(8);

        modeBadgeRect = hdr.removeFromLeft(80).reduced(0, 10);
        hdr.removeFromLeft(8);

        auto btnArea = hdr.removeFromRight(176);
        savePresetButton.setBounds(btnArea.removeFromRight(82).reduced(0, 8));
        btnArea.removeFromRight(8);
        loadPresetButton.setBounds(btnArea.removeFromRight(82).reduced(0, 8));
    }
    area.removeFromTop(10);

    if (isFM)
    {
        auto algRow = area.removeFromTop(46);
        algorithmLabel.setBounds(algRow.removeFromTop(18));
        algorithmSelector.setBounds(algRow.removeFromTop(26));
        area.removeFromTop(8);

        const int opKnobW  = 60;
        const int opLabelH = 16;

        for (int op = 0; op < 4; ++op)
        {
            auto& c    = fmOps[op];
            auto opRow = area.removeFromTop(opLabelH + opKnobW + opLabelH);
            c.sectionLabel.setBounds(opRow.removeFromTop(opLabelH));

            auto knobs = opRow;
            auto place = [&](juce::Slider& s, juce::Label& l)
            {
                auto col = knobs.removeFromLeft(opKnobW);
                s.setBounds(col.removeFromTop(opKnobW));
                l.setBounds(col.removeFromTop(opLabelH));
                knobs.removeFromLeft(4);
            };
            place(c.ratioSlider,    c.ratioLabel);
            place(c.levelSlider,    c.levelLabel);
            place(c.feedbackSlider, c.feedbackLabel);
            place(c.attackSlider,   c.attackLabel);
            place(c.decaySlider,    c.decayLabel);
            place(c.sustainSlider,  c.sustainLabel);
            place(c.releaseSlider,  c.releaseLabel);
            area.removeFromTop(8);
        }
    }
    else
    {
        const int knobSz = 70;
        const int labelH = 18;

        // OSC section
        oscSectionRect = area.removeFromTop(80);
        {
            auto inner = oscSectionRect.reduced(8).withTrimmedTop(16);
            auto left  = inner.removeFromLeft(220);
            waveformLabel.setBounds(left.removeFromTop(16));
            waveformSelector.setBounds(left.removeFromTop(26));
            oscDisplay.setBounds(inner.reduced(2));
        }
        area.removeFromTop(8);

        // FILTER section
        filterSectionRect = area.removeFromTop(155);
        {
            auto inner   = filterSectionRect.reduced(8).withTrimmedTop(18);
            auto knobRow = inner.removeFromTop(knobSz + labelH);
            auto placeKnob = [&](juce::Slider& s, juce::Label& l)
            {
                auto col = knobRow.removeFromLeft(knobSz);
                s.setBounds(col.removeFromTop(knobSz));
                l.setBounds(col.removeFromTop(labelH));
                knobRow.removeFromLeft(12);
            };
            placeKnob(cutoffSlider,    cutoffLabel);
            placeKnob(resonanceSlider, resonanceLabel);

            inner.removeFromTop(4);
            auto osRow = inner.removeFromTop(28);
            oversamplingLabel.setBounds(osRow.removeFromLeft(30));
            osRow.removeFromLeft(6);
            oversamplingSelector.setBounds(osRow.removeFromLeft(90));
        }
        area.removeFromTop(8);

        // ENV section
        envSectionRect = area.removeFromTop(130);
        {
            auto inner   = envSectionRect.reduced(8).withTrimmedTop(18);
            auto knobRow = inner;
            auto placeKnob = [&](juce::Slider& s, juce::Label& l)
            {
                auto col = knobRow.removeFromLeft(knobSz);
                s.setBounds(col.removeFromTop(knobSz));
                l.setBounds(col.removeFromTop(labelH));
                knobRow.removeFromLeft(12);
            };
            placeKnob(attackSlider,  attackLabel);
            placeKnob(decaySlider,   decayLabel);
            placeKnob(sustainSlider, sustainLabel);
            placeKnob(releaseSlider, releaseLabel);
        }
        area.removeFromTop(8);

        // LFO display
        lfoDisplay.setBounds(area.removeFromTop(55));
    }
}
