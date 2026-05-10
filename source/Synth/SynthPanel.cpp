#include "SynthPanel.h"
#include "../MultiverseFlatTheme.h"
#include "../PluginProcessor.h"

const juce::Colour SynthPanel::oscColours[8] = {
    MultiverseFlatTheme::accentCyan(),
    MultiverseFlatTheme::accentPink(),
    MultiverseFlatTheme::accentPurple(),
    MultiverseFlatTheme::accentGreen(),
    MultiverseFlatTheme::accentAmber(),
    MultiverseFlatTheme::accentCyan(),
    MultiverseFlatTheme::accentPink(),
    MultiverseFlatTheme::accentPurple()
};

const char* SynthPanel::sectionNames[kNumSections] = {
    "OSC", "SUB / NOISE", "UNISON", "FILTER", "ENV", "CHORD / STRUM"
};
const int SynthPanel::sectionDefaultHeights[kNumSections] = {
    280, 110, 110, 170, 150, 90
};
const int SynthPanel::sectionCollapsedH = 24;

int SynthPanel::getSectionHeight(SectionID id) const
{
    return sectionExpanded[id] ? sectionDefaultHeights[id] : sectionCollapsedH;
}

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
        synthEngine.setSynthMode(modeSelector.getSelectedId() == 2 ? SynthMode::FM : SynthMode::Classic);
        updateVisibility();
        resized();
        repaint();
    };
    addAndMakeVisible(modeSelector);

    // OSC count buttons
    addOscButton.setTooltip("Add an oscillator (max 4)");
    removeOscButton.setTooltip("Remove an oscillator (min 1)");
    addOscButton.onClick = [this]()
    {
        auto* param = processorRef.apvts.getParameter("oscCount");
        if (param)
        {
            const int current = juce::roundToInt(param->getValue() * 3.0f);
            if (current < 3)
                param->setValueNotifyingHost((current + 1) / 3.0f);
        }
        updateVisibility();
        resized();
    };
    removeOscButton.onClick = [this]()
    {
        auto* param = processorRef.apvts.getParameter("oscCount");
        if (param)
        {
            const int current = juce::roundToInt(param->getValue() * 3.0f);
            if (current > 0)
                param->setValueNotifyingHost((current - 1) / 3.0f);
        }
        updateVisibility();
        resized();
    };
    addAndMakeVisible(addOscButton);
    addAndMakeVisible(removeOscButton);

    // 8 Oscillator strips — all active ones are visible simultaneously
    for (int osc = 0; osc < 8; ++osc)
    {
        // Per-osc waveform display
        oscDisplays[osc].setWaveform(processorRef.baseWaveform);
        addChildComponent(oscDisplays[osc]);

        auto& c = oscControls[osc];
        const juce::String pfx = "Osc " + juce::String(osc + 1);
        setupLabel(c.sectionLabel, pfx);
        addChildComponent(c.sectionLabel);

        c.typeSelector.addItem("Classic",    1);
        c.typeSelector.addItem("Wavetable",  2);
        c.typeSelector.addItem("Additive",   3);
        c.typeSelector.addItem("Phase Dist", 4);
        c.typeSelector.addItem("Analog",     5);
        c.typeSelector.addItem("Digital",    6);
        c.typeSelector.setSelectedId(1, juce::dontSendNotification);
        addChildComponent(c.typeSelector);

        setupSlider(c.levelSlider, 0.0, 1.0, osc < 3 ? 1.0 : 0.0);
        setupLabel(c.levelLabel, "Level");
        addChildComponent(c.levelSlider); addChildComponent(c.levelLabel);

        setupSlider(c.detuneSlider, -12.0, 12.0, 0.0);
        setupLabel(c.detuneLabel, "Detune");
        addChildComponent(c.detuneSlider); addChildComponent(c.detuneLabel);

        c.waveformSelector.addItem("Sine",     1);
        c.waveformSelector.addItem("Saw",      2);
        c.waveformSelector.addItem("Square",   3);
        c.waveformSelector.addItem("Triangle", 4);
        c.waveformSelector.addItem("Noise",    5);
        c.waveformSelector.setSelectedId(2, juce::dontSendNotification);
        addChildComponent(c.waveformSelector);

        setupSlider(c.wavePosSlider, 0.0, 1.0, 0.0);
        setupLabel(c.wavePosLabel, "WavePos");
        addChildComponent(c.wavePosSlider); addChildComponent(c.wavePosLabel);

        // Shape controls
        c.shapeTypeSelector.addItem("Off",   1);
        c.shapeTypeSelector.addItem("Drive", 2);
        c.shapeTypeSelector.addItem("Fold",  3);
        c.shapeTypeSelector.addItem("Clip",  4);
        c.shapeTypeSelector.setSelectedId(1, juce::dontSendNotification);
        addChildComponent(c.shapeTypeSelector);

        setupSlider(c.shapeAmtSlider, 0.0, 1.0, 0.0);
        setupLabel(c.shapeAmtLabel, "Shape");
        addChildComponent(c.shapeAmtSlider); addChildComponent(c.shapeAmtLabel);

        setupSlider(c.selfOscSlider, 0.0, 1.0, 0.0);
        setupLabel(c.selfOscLabel, "Self");
        addChildComponent(c.selfOscSlider); addChildComponent(c.selfOscLabel);

        setupSlider(c.phaseDistSlider, 0.0, 1.0, 0.5);
        setupLabel(c.phaseDistLabel, "PD Amt");
        addChildComponent(c.phaseDistSlider); addChildComponent(c.phaseDistLabel);

        const juce::String paramPrefix = "osc" + juce::String(osc + 1);
        auto& apvts = processorRef.apvts;
        c.typeAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, paramPrefix + "Type", c.typeSelector);
        c.levelAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "Level", c.levelSlider);
        c.detuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "Detune", c.detuneSlider);
        c.waveformAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, paramPrefix + "Waveform", c.waveformSelector);
        c.wavePosAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "WavePos", c.wavePosSlider);
        c.shapeTypeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, paramPrefix + "ShapeType", c.shapeTypeSelector);
        c.shapeAmtAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "ShapeAmt", c.shapeAmtSlider);
        c.selfOscAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "SelfOsc", c.selfOscSlider);
        c.phaseDistAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramPrefix + "PhaseDist", c.phaseDistSlider);

        c.levelSlider.init(processorRef, paramPrefix + "Level");
        c.detuneSlider.init(processorRef, paramPrefix + "Detune");
        c.wavePosSlider.init(processorRef, paramPrefix + "WavePos");
        c.shapeAmtSlider.init(processorRef, paramPrefix + "ShapeAmt");
        c.selfOscSlider.init(processorRef, paramPrefix + "SelfOsc");
        c.phaseDistSlider.init(processorRef, paramPrefix + "PhaseDist");

        setupLabel(c.wtFileLabel, "no file");
        c.wtFileLabel.setFont(MultiverseFlatTheme::labelFont());
        addChildComponent(c.loadWTButton);
        addChildComponent(c.editWTButton);
        addChildComponent(c.wtFileLabel);
        c.loadWTButton.setTooltip("Load a .wav or .aif wavetable file from disk");
        c.editWTButton.setTooltip("Open the wavetable editor for this oscillator");

        // Create wavetable editor overlay for this osc
        wavetableEditors[osc] = std::make_unique<WavetableEditor>(synthEngine.getWavetableOscillator(osc));
        wavetableEditors[osc]->onWavetableChanged = [this, osc]() { synthEngine.distributeWavetable(osc); };
        wavetableEditors[osc]->setVisible(false);
        addChildComponent(wavetableEditors[osc].get());

        c.editWTButton.onClick = [this, osc]()
        {
            for (int i = 0; i < 8; ++i)
                if (i != osc && wavetableEditors[i]) wavetableEditors[i]->setVisible(false);

            auto* ed = wavetableEditors[osc].get();
            if (ed->isVisible()) { ed->setVisible(false); return; }
            ed->setBounds(getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin));
            ed->setVisible(true);
            ed->toFront(false);
        };

        const int oscIdx = osc;
        c.loadWTButton.onClick = [this, oscIdx]()
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Load Wavetable - Osc " + juce::String(oscIdx + 1),
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*.wav;*.aif;*.aiff");
            fileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                [this, oscIdx](const juce::FileChooser& fc)
                {
                    auto f = fc.getResult();
                    if (!f.existsAsFile()) return;
                    if (synthEngine.loadWavetableFile(oscIdx, f))
                        oscControls[oscIdx].wtFileLabel.setText(
                            f.getFileNameWithoutExtension(), juce::dontSendNotification);
                });
        };
    }

    // Type change listeners — update waveform display and visibility
    for (int osc = 0; osc < 8; ++osc)
    {
        oscControls[osc].typeSelector.onChange = [this]()
        {
            updateVisibility();
            resized();
        };
        // Waveform selector updates osc display
        oscControls[osc].waveformSelector.onChange = [this, osc]()
        {
            const auto wf = static_cast<WaveformType>(oscControls[osc].waveformSelector.getSelectedId() - 1);
            oscDisplays[osc].setWaveform(wf);
        };
    }

    // Visualisations (filter and envelope displays remain in their sections)
    filterDisplay.setProcessor(&processorRef);
    addAndMakeVisible(filterDisplay);
    addAndMakeVisible(envelopeDisplay);

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

    // Envelope display
    addAndMakeVisible(envelopeDisplay);
    envelopeDisplay.setSliders(&attackSlider, &decaySlider, &sustainSlider, &releaseSlider);

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

    // Filter type selector
    setupLabel(filterTypeLabel, "TYPE");
    addAndMakeVisible(filterTypeLabel);
    filterTypeSelector.addItem("LP",    1);
    filterTypeSelector.addItem("HP",    2);
    filterTypeSelector.addItem("BP",    3);
    filterTypeSelector.addItem("Notch", 4);
    filterTypeSelector.setSelectedId(1, juce::dontSendNotification);
    filterTypeSelector.setTooltip("Filter topology: Low-pass, High-pass, Band-pass, or Notch");
    addAndMakeVisible(filterTypeSelector);
    auto& apvts = processorRef.apvts;
    filterTypeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "filterType", filterTypeSelector);
    attackAttach    = std::make_unique<SliderAttachment>(apvts, "attack",          attackSlider);
    decayAttach     = std::make_unique<SliderAttachment>(apvts, "decay",           decaySlider);
    sustainAttach   = std::make_unique<SliderAttachment>(apvts, "sustain",         sustainSlider);
    releaseAttach   = std::make_unique<SliderAttachment>(apvts, "release",         releaseSlider);
    cutoffAttach    = std::make_unique<SliderAttachment>(apvts, "filterCutoff",    cutoffSlider);
    resonanceAttach = std::make_unique<SliderAttachment>(apvts, "filterResonance", resonanceSlider);

    // Unison controls
    setupLabel(unisonVoicesLabel, "VOICES");
    setupLabel(unisonDetuneLabel, "DETUNE");
    setupLabel(unisonWidthLabel,  "WIDTH");
    addAndMakeVisible(unisonVoicesLabel);
    addAndMakeVisible(unisonDetuneLabel);
    addAndMakeVisible(unisonWidthLabel);

    for (int i = 1; i <= 8; ++i)
        unisonVoicesBox.addItem(juce::String(i), i);
    unisonVoicesBox.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(unisonVoicesBox);

    setupSlider(unisonDetuneSlider, 0.0, 100.0, 20.0);
    setupSlider(unisonWidthSlider,  0.0, 1.0,   1.0);
    addAndMakeVisible(unisonDetuneSlider);
    addAndMakeVisible(unisonWidthSlider);

    unisonVoicesAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "unisonVoices", unisonVoicesBox);
    unisonDetuneAttach = std::make_unique<SliderAttachment>(apvts, "unisonDetune", unisonDetuneSlider);
    unisonWidthAttach  = std::make_unique<SliderAttachment>(apvts, "unisonWidth",  unisonWidthSlider);
    unisonDetuneSlider.init(processorRef, "unisonDetune");
    unisonWidthSlider.init(processorRef,  "unisonWidth");

    unisonVoicesBox.setTooltip("Number of stacked voices per note (1 = off)");
    unisonDetuneSlider.setTooltip("Detune spread across unison voices in cents");
    unisonWidthSlider.setTooltip("Stereo spread of unison voices (0 = mono, 1 = full wide)");

    // Unison spread mode
    setupLabel(unisonSpreadLabel, "SPREAD");
    addAndMakeVisible(unisonSpreadLabel);
    unisonSpreadSelector.addItem("Stacked", 1);
    unisonSpreadSelector.addItem("Chord",   2);
    unisonSpreadSelector.addItem("Random",  3);
    unisonSpreadSelector.setSelectedId(1, juce::dontSendNotification);
    unisonSpreadSelector.setTooltip("Unison spread: Stacked=linear detune, Chord=major chord intervals, Random=random detune per voice");
    addAndMakeVisible(unisonSpreadSelector);
    unisonSpreadAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "unisonSpreadMode", unisonSpreadSelector);

    // Sub oscillator controls
    setupLabel(subOscLabel,       "SUB");
    setupLabel(noiseOscLabel,     "NOISE");
    setupLabel(subOscLevelLabel,  "Level");
    setupLabel(noiseOscLevelLabel,"Level");
    setupLabel(noiseOscColorLabel,"Color");
    addAndMakeVisible(subOscLabel);
    addAndMakeVisible(noiseOscLabel);

    subOscEnableButton.setButtonText("ON");
    subOscEnableButton.setTooltip("Enable sub oscillator (1 octave below, Sine or Square)");
    addAndMakeVisible(subOscEnableButton);

    noiseOscEnableButton.setButtonText("ON");
    noiseOscEnableButton.setTooltip("Enable noise oscillator with LP color filter");
    addAndMakeVisible(noiseOscEnableButton);

    setupSlider(subOscLevelSlider,   0.0, 1.0, 0.5);
    setupSlider(noiseOscLevelSlider, 0.0, 1.0, 0.3);
    setupSlider(noiseOscColorSlider, 200.0, 20000.0, 5000.0, 0.3);
    addAndMakeVisible(subOscLevelSlider);   addAndMakeVisible(subOscLevelLabel);
    addAndMakeVisible(noiseOscLevelSlider); addAndMakeVisible(noiseOscLevelLabel);
    addAndMakeVisible(noiseOscColorSlider); addAndMakeVisible(noiseOscColorLabel);

    subOscWaveSelector.addItem("Sine",   1);
    subOscWaveSelector.addItem("Square", 2);
    subOscWaveSelector.setSelectedId(1, juce::dontSendNotification);
    subOscWaveSelector.setTooltip("Sub oscillator waveform");
    addAndMakeVisible(subOscWaveSelector);

    subOscEnableAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "subOscEnable",   subOscEnableButton);
    noiseOscEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "noiseOscEnable", noiseOscEnableButton);
    subOscLevelAttach    = std::make_unique<SliderAttachment>(apvts, "subOscLevel",    subOscLevelSlider);
    noiseOscLevelAttach  = std::make_unique<SliderAttachment>(apvts, "noiseOscLevel",  noiseOscLevelSlider);
    noiseOscColorAttach  = std::make_unique<SliderAttachment>(apvts, "noiseOscColor",  noiseOscColorSlider);
    subOscWaveAttach     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "subOscWave", subOscWaveSelector);
    subOscLevelSlider.init(processorRef,   "subOscLevel");
    noiseOscLevelSlider.init(processorRef, "noiseOscLevel");
    noiseOscColorSlider.init(processorRef, "noiseOscColor");

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
        setupSlider(c.releaseSlider, 0.001, 10.0, 0.3,  0.4);

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

        c.ratioSlider.init(processorRef, pfx + "Ratio");
        c.levelSlider.init(processorRef, pfx + "Level");
        c.feedbackSlider.init(processorRef, pfx + "Feedback");
        c.attackSlider.init(processorRef, pfx + "Attack");
        c.decaySlider.init(processorRef, pfx + "Decay");
        c.sustainSlider.init(processorRef, pfx + "Sustain");
        c.releaseSlider.init(processorRef, pfx + "Release");
    }

    // Voice mode / portamento controls
    setupLabel(voiceModeLabel, "VOICE");
    addAndMakeVisible(voiceModeLabel);

    voiceModeSelector.addItem("Poly",   1);
    voiceModeSelector.addItem("Mono",   2);
    voiceModeSelector.addItem("Legato", 3);
    voiceModeSelector.setSelectedId(1, juce::dontSendNotification);
    voiceModeSelector.setTooltip("Poly: full polyphony; Mono: one note, retrigger; Legato: one note, smooth (no retrigger)");
    addAndMakeVisible(voiceModeSelector);
    voiceModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "voiceMode", voiceModeSelector);

    setupLabel(portamentoLabel, "PORTA");
    addAndMakeVisible(portamentoLabel);

    portamentoSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    portamentoSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 20);
    portamentoSlider.setRange(0.0, 2.0);
    portamentoSlider.setSkewFactor(0.4);
    portamentoSlider.setValue(0.0, juce::dontSendNotification);
    portamentoSlider.setTooltip("Portamento glide time (0 = instant, 2s = slow glide)");
    addAndMakeVisible(portamentoSlider);
    portamentoSlider.init(processorRef, "portamento");
    portamentoAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "portamento", portamentoSlider);

    portaAlwaysButton.setButtonText("Always");
    portaAlwaysButton.setTooltip("Always porta: glide even when notes don't overlap");
    addAndMakeVisible(portaAlwaysButton);
    portaAlwaysAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "portaAlways", portaAlwaysButton);

    mpeButton.setButtonText("MPE");
    mpeButton.setTooltip("Enable MPE: per-note pitch bend, pressure, and slide from expressive controllers");
    addAndMakeVisible(mpeButton);
    mpeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "mpeEnabled", mpeButton);

    // Tooltips
    modeSelector.setTooltip("Synthesis mode: Classic (subtractive) or FM");
    savePresetButton.setTooltip("Save current state to a preset file");
    loadPresetButton.setTooltip("Load a previously saved preset file");

    for (int osc = 0; osc < 8; ++osc)
    {
        auto& c = oscControls[osc];
        const juce::String pfx = "Osc " + juce::String(osc + 1) + " — ";
        c.typeSelector.setTooltip   (pfx + "Type: Classic/Wavetable/Additive/Phase Dist/Analog/Digital");
        c.levelSlider.setTooltip    (pfx + "Output level (0–100%)");
        c.detuneSlider.setTooltip   (pfx + "Detune in semitones (±12)");
        c.waveformSelector.setTooltip(pfx + "Classic waveform: Sine/Saw/Square/Triangle/Noise");
        c.wavePosSlider.setTooltip  (pfx + "Wavetable scan position (0–100%)");
        c.shapeTypeSelector.setTooltip(pfx + "Wave shaping mode: Off/Drive/Fold/Clip");
        c.shapeAmtSlider.setTooltip (pfx + "Wave shaping amount (0–100%)");
        c.selfOscSlider.setTooltip  (pfx + "Self-oscillation feedback (0–100%)");
        c.phaseDistSlider.setTooltip(pfx + "Phase distortion amount (0–100%)");
    }

    attackSlider.setTooltip      ("Envelope Attack: time to reach full volume (1ms–5s)");
    decaySlider.setTooltip       ("Envelope Decay: time to fall to sustain level (1ms–5s)");
    sustainSlider.setTooltip     ("Envelope Sustain: held volume level (0–100%)");
    releaseSlider.setTooltip     ("Envelope Release: time to silence after note-off (1ms–10s)");
    cutoffSlider.setTooltip      ("Filter Cutoff: frequency where filtering begins (20Hz–20kHz)");
    resonanceSlider.setTooltip   ("Filter Resonance: emphasis at cutoff. High values add a ringing tone.");
    oversamplingSelector.setTooltip("Filter Oversampling: Off / 2x / 4x / Auto");
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
        c.releaseSlider.setTooltip(pfx + "Operator envelope Release (1ms–10s)");
    }

    // Chord/Strum mode
    chordEnableButton.setButtonText("CHORD");
    chordEnableButton.setTooltip("Chord mode: each note triggers a full chord voicing");
    addAndMakeVisible(chordEnableButton);
    chordEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "chordModeEnabled", chordEnableButton);

    chordShapeSelector.addItem("Root Only", 1);
    chordShapeSelector.addItem("Major",     2);
    chordShapeSelector.addItem("Minor",     3);
    chordShapeSelector.addItem("Maj7",      4);
    chordShapeSelector.addItem("Min7",      5);
    chordShapeSelector.addItem("Dom7",      6);
    chordShapeSelector.addItem("Dim",       7);
    chordShapeSelector.addItem("Aug",       8);
    chordShapeSelector.addItem("Sus2",      9);
    chordShapeSelector.addItem("Sus4",      10);
    chordShapeSelector.addItem("Power",     11);
    chordShapeSelector.addItem("Octave",    12);
    chordShapeSelector.setSelectedId(2, juce::dontSendNotification);
    chordShapeSelector.setTooltip("Chord voicing: interval set triggered above root note");
    addAndMakeVisible(chordShapeSelector);
    chordShapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "chordShape", chordShapeSelector);

    setupSlider(chordStrumSlider, 0.0, 200.0, 0.0, 1.0);
    chordStrumSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 16);
    chordStrumSlider.setTooltip("Strum delay: time between successive chord notes (0–200 ms)");
    addAndMakeVisible(chordStrumSlider);
    chordStrumSlider.init(processorRef, "chordStrumDelay");
    chordStrumAttach = std::make_unique<SliderAttachment>(apvts, "chordStrumDelay", chordStrumSlider);

    setupLabel(chordShapeLabel, "SHAPE");
    addAndMakeVisible(chordShapeLabel);
    setupLabel(chordStrumLabel, "STRUM");
    addAndMakeVisible(chordStrumLabel);

    // Readout bar
    addAndMakeVisible(readoutBar);

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
    l.setFont(MultiverseFlatTheme::labelFont());
    l.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
}

void SynthPanel::drawSection(juce::Graphics& g, juce::Rectangle<int> r,
                               const juce::String& title) const
{
    drawSection(g, r, title, true);
}

void SynthPanel::drawSection(juce::Graphics& g, juce::Rectangle<int> r,
                               const juce::String& title, bool expanded) const
{
    const float cr = 8.0f;
    MultiverseFlatTheme::drawCard(g, r.toFloat(), cr);

    // Disclosure triangle
    const float triX = 10.0f;
    const float triY = r.getY() + 12.0f;
    const float triSize = 6.0f;
    juce::Path triangle;
    if (expanded)
    {
        triangle.addTriangle(triX - triSize * 0.5f, triY - triSize * 0.4f,
                             triX + triSize * 0.5f, triY - triSize * 0.4f,
                             triX, triY + triSize * 0.6f);
    }
    else
    {
        triangle.addTriangle(triX - triSize * 0.4f, triY - triSize * 0.5f,
                             triX - triSize * 0.4f, triY + triSize * 0.5f,
                             triX + triSize * 0.6f, triY);
    }
    g.setColour(expanded ? MultiverseFlatTheme::accentCyan() : MultiverseFlatTheme::textMuted());
    g.fillPath(triangle);

    g.setColour(expanded ? MultiverseFlatTheme::textPrimary() : MultiverseFlatTheme::textMuted());
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText(title, r.getX() + 22, r.getY() + 5, 200, 14, juce::Justification::centredLeft);

    if (expanded)
    {
        MultiverseFlatTheme::drawDivider(g, static_cast<float>(r.getY() + 18),
                                          static_cast<float>(r.getX() + 8),
                                          static_cast<float>(r.getRight() - 8));
    }
}

//==============================================================================
void SynthPanel::updateVisibility()
{
    const bool isFM = (modeSelector.getSelectedId() == 2);

    // OSC section — all active oscillators visible (if section expanded)
    const bool oscVisible = !isFM && sectionExpanded[kOSC];
    const int activeOscCount = static_cast<int>(*processorRef.apvts.getRawParameterValue("oscCount")) + 1;

    for (int i = 0; i < 8; ++i)
    {
        auto& c = oscControls[i];
        const bool isActive = oscVisible && (i < activeOscCount);
        const bool isWavetable  = isActive && c.typeSelector.getSelectedId() == 2;
        const bool isPhaseDist  = isActive && c.typeSelector.getSelectedId() == 4;
        const bool hasShaping   = isActive && c.typeSelector.getSelectedId() > 1;

        c.sectionLabel.setVisible(isActive);
        c.typeSelector.setVisible(isActive);
        c.levelSlider.setVisible(isActive);    c.levelLabel.setVisible(isActive);
        c.detuneSlider.setVisible(isActive);   c.detuneLabel.setVisible(isActive);
        c.waveformSelector.setVisible(isActive && !isWavetable);
        c.wavePosSlider.setVisible(isActive && isWavetable);
        c.wavePosLabel.setVisible(isActive && isWavetable);
        c.loadWTButton.setVisible(isActive && isWavetable);
        c.editWTButton.setVisible(isActive && isWavetable);
        c.wtFileLabel.setVisible(isActive && isWavetable);
        c.shapeTypeSelector.setVisible(isActive);
        c.shapeAmtSlider.setVisible(isActive && hasShaping);
        c.shapeAmtLabel.setVisible(isActive && hasShaping);
        c.selfOscSlider.setVisible(isActive);
        c.selfOscLabel.setVisible(isActive);
        c.phaseDistSlider.setVisible(isActive && isPhaseDist);
        c.phaseDistLabel.setVisible(isActive && isPhaseDist);

        // Per-osc waveform display
        oscDisplays[i].setVisible(isActive);
    }

    // +/- OSC buttons
    addOscButton.setVisible(!isFM);
    removeOscButton.setVisible(!isFM);

    // Mode badge shows osc count
    modeSelector.setVisible(true);

    // Collapsed sections hide their controls regardless of mode
    const bool subNoiseVisible = !isFM && sectionExpanded[kSubNoise];
    const bool unisonVisible = !isFM && sectionExpanded[kUnison];
    const bool filterVisible = !isFM && sectionExpanded[kFilter];
    const bool envVisible = !isFM && sectionExpanded[kEnv];
    const bool chordVisible = !isFM && sectionExpanded[kChord];

    unisonVoicesLabel.setVisible(unisonVisible); unisonVoicesBox.setVisible(unisonVisible);
    unisonDetuneLabel.setVisible(unisonVisible); unisonDetuneSlider.setVisible(unisonVisible);
    unisonWidthLabel.setVisible(unisonVisible);  unisonWidthSlider.setVisible(unisonVisible);
    unisonSpreadLabel.setVisible(unisonVisible); unisonSpreadSelector.setVisible(unisonVisible);

    attackLabel.setVisible(envVisible);  attackSlider.setVisible(envVisible);
    decayLabel.setVisible(envVisible);   decaySlider.setVisible(envVisible);
    sustainLabel.setVisible(envVisible); sustainSlider.setVisible(envVisible);
    releaseLabel.setVisible(envVisible); releaseSlider.setVisible(envVisible);
    envelopeDisplay.setVisible(envVisible);

    cutoffLabel.setVisible(filterVisible);        cutoffSlider.setVisible(filterVisible);
    filterDisplay.setVisible(filterVisible);
    resonanceLabel.setVisible(filterVisible);     resonanceSlider.setVisible(filterVisible);
    oversamplingLabel.setVisible(filterVisible);  oversamplingSelector.setVisible(filterVisible);
    filterTypeLabel.setVisible(filterVisible);    filterTypeSelector.setVisible(filterVisible);

    subOscLabel.setVisible(subNoiseVisible);       subOscEnableButton.setVisible(subNoiseVisible);
    subOscLevelSlider.setVisible(subNoiseVisible); subOscLevelLabel.setVisible(subNoiseVisible);
    subOscWaveSelector.setVisible(subNoiseVisible);
    noiseOscLabel.setVisible(subNoiseVisible);          noiseOscEnableButton.setVisible(subNoiseVisible);
    noiseOscLevelSlider.setVisible(subNoiseVisible);    noiseOscLevelLabel.setVisible(subNoiseVisible);
    noiseOscColorSlider.setVisible(subNoiseVisible);    noiseOscColorLabel.setVisible(subNoiseVisible);

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

    // Chord/Strum — only in Classic mode and section expanded
    chordEnableButton.setVisible(chordVisible);
    chordShapeLabel.setVisible(chordVisible);
    chordShapeSelector.setVisible(chordVisible);
    chordStrumLabel.setVisible(chordVisible);
    chordStrumSlider.setVisible(chordVisible);

    savePresetButton.setVisible(true);
    loadPresetButton.setVisible(true);
}

//==============================================================================
void SynthPanel::paint(juce::Graphics& g)
{
    MultiverseFlatTheme::drawContentBackground(g, getLocalBounds().toFloat());

    const bool isFM = (modeSelector.getSelectedId() == 2);

    if (!isFM)
    {
        drawSection(g, oscSectionRect,      "OSC",          sectionExpanded[kOSC]);
        drawSection(g, subNoiseSectionRect, "SUB / NOISE",  sectionExpanded[kSubNoise]);
        drawSection(g, unisonSectionRect,   "UNISON",       sectionExpanded[kUnison]);
        drawSection(g, filterSectionRect,   "FILTER",       sectionExpanded[kFilter]);
        drawSection(g, envSectionRect,      "ENV",          sectionExpanded[kEnv]);
        drawSection(g, chordSectionRect,    "CHORD / STRUM", sectionExpanded[kChord]);

        // Accent lines at top of each osc strip
        const int activeOscCount = static_cast<int>(*processorRef.apvts.getRawParameterValue("oscCount")) + 1;
        const int gap = 6;
        const int stripW = (oscSectionRect.getWidth() - 16 - (activeOscCount - 1) * gap) / activeOscCount;
        for (int i = 0; i < activeOscCount; ++i)
        {
            const auto col = oscColours[i];
            const int x = oscSectionRect.getX() + 8 + i * (stripW + gap);
            auto accentRect = juce::Rectangle<float>(x, oscSectionRect.getY() + 2, stripW, 2.0f);
            g.setColour(col);
            g.fillRect(accentRect);
        }
    }
    else
    {
        drawSection(g, fmSectionRect, "FM OPERATORS");
    }

    // Voice mode section
    if (voiceSectionRect.getHeight() > 0)
    {
        drawSection(g, voiceSectionRect, "VOICE MODE");
    }

    // Mode badge
    const juce::Colour badgeCol = isFM ? MultiverseFlatTheme::accentBlue().withAlpha(0.7f) : MultiverseFlatTheme::accentBlue();
    g.setColour(badgeCol.withAlpha(0.18f));
    g.fillRoundedRectangle(modeBadgeRect.toFloat(), 4.0f);
    g.setColour(badgeCol);
    g.drawRoundedRectangle(modeBadgeRect.toFloat(), 4.0f, 1.0f);
    g.setColour(badgeCol);
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText(isFM ? "FM" : "CLASSIC", modeBadgeRect, juce::Justification::centred);
}

//==============================================================================
void SynthPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin);
    const bool isFM = (modeSelector.getSelectedId() == 2);

    // Header
    {
        auto hdr = area.removeFromTop(46);
        auto voiceHdr = hdr;

        // Buttons on the right first
        auto btnArea = hdr.removeFromRight(176);
        savePresetButton.setBounds(btnArea.removeFromRight(82).reduced(0, 8));
        btnArea.removeFromRight(8);
        loadPresetButton.setBounds(btnArea.removeFromRight(82).reduced(0, 8));

        // Synth mode on the left
        auto modeCol = hdr.removeFromLeft(180);
        modeLabel.setBounds(modeCol.removeFromTop(18));
        modeSelector.setBounds(modeCol.removeFromTop(26));
        hdr.removeFromLeft(8);

        modeBadgeRect = hdr.removeFromLeft(80).reduced(0, 10);
        hdr.removeFromLeft(16);

        // Voice section rect (remaining header area)
        voiceSectionRect = hdr.reduced(2, 4);

        // Voice mode strip (centre)
        voiceModeLabel.setBounds(hdr.removeFromLeft(52).reduced(0, 14));
        hdr.removeFromLeft(4);
        voiceModeSelector.setBounds(hdr.removeFromLeft(108).reduced(0, 8));
        hdr.removeFromLeft(12);
        portaAlwaysButton.setBounds(hdr.removeFromLeft(68).reduced(4, 12));
        hdr.removeFromLeft(8);
        mpeButton.setBounds(hdr.removeFromLeft(48).reduced(4, 12));
        hdr.removeFromLeft(8);
        auto portaCol = hdr.removeFromLeft(180);
        portamentoLabel.setBounds(portaCol.removeFromTop(18));
        portamentoSlider.setBounds(portaCol.removeFromTop(26));
    }
    area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

    if (isFM)
    {
        fmSectionRect = area.withHeight(46 + 8 + 4 * (16 + 60 + 16 + 8));
        auto fmArea = fmSectionRect.reduced(8);

        auto algRow = fmArea.removeFromTop(46);
        algorithmLabel.setBounds(algRow.removeFromTop(18));
        algorithmSelector.setBounds(algRow.removeFromTop(26));
        fmArea.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        const int opKnobW  = 60;
        const int opLabelH = 16;

        for (int op = 0; op < 4; ++op)
        {
            auto& c    = fmOps[op];
            auto opRow = fmArea.removeFromTop(opLabelH + opKnobW + opLabelH);
            c.sectionLabel.setBounds(opRow.removeFromTop(opLabelH));

            auto knobs = opRow;
            auto place = [&](juce::Slider& s, juce::Label& l)
            {
                auto col = knobs.removeFromLeft(opKnobW);
                s.setBounds(col.removeFromTop(opKnobW));
                l.setBounds(col.removeFromTop(opLabelH));
                knobs.removeFromLeft(MultiverseFlatTheme::Metrics::smallGap);
            };
            place(c.ratioSlider,    c.ratioLabel);
            place(c.levelSlider,    c.levelLabel);
            place(c.feedbackSlider, c.feedbackLabel);
            place(c.attackSlider,   c.attackLabel);
            place(c.decaySlider,    c.decayLabel);
            place(c.sustainSlider,  c.sustainLabel);
            place(c.releaseSlider,  c.releaseLabel);
            fmArea.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);
        }
        area.removeFromTop(fmSectionRect.getHeight() + MultiverseFlatTheme::Metrics::sectionGap);
    }
    else
    {
        const int knobSz = 80;
        const int labelH  = 18;
        const int gap      = 6;

        // OSC section: all oscillators in a horizontal row
        const int activeOscCount = static_cast<int>(*processorRef.apvts.getRawParameterValue("oscCount")) + 1;
        // Dynamic height based on collapse state
        const int oscSectionH = getSectionHeight(kOSC);
        oscSectionRect = area.removeFromTop(oscSectionH);
        sectionHeaderRects[kOSC] = oscSectionRect.withHeight(sectionCollapsedH);

        if (sectionExpanded[kOSC])
        {
            auto oscInner = oscSectionRect.reduced(8).withTrimmedTop(20);

            // + / - buttons row (compact, right-aligned)
            auto btnRow = oscInner.removeFromTop(24);
            addOscButton.setBounds(btnRow.removeFromRight(24).reduced(1, 2));
            btnRow.removeFromRight(4);
            removeOscButton.setBounds(btnRow.removeFromRight(24).reduced(1, 2));

            // Strip width calculation
            const int totalWidth = oscInner.getWidth();
            const int stripW = (totalWidth - (activeOscCount - 1) * gap) / activeOscCount;

            // Layout each oscillator strip
            int xPos = oscInner.getX();
            for (int i = 0; i < activeOscCount; ++i)
            {
                auto& c = oscControls[i];
                const bool isWavetable = c.typeSelector.getSelectedId() == 2;
                const bool isPhaseDist = c.typeSelector.getSelectedId() == 4;
                const bool hasShaping   = c.typeSelector.getSelectedId() > 1;

                auto strip = oscInner.withLeft(xPos).withWidth(stripW);

                // Waveform display (compact)
                oscDisplays[i].setBounds(strip.removeFromTop(32));
                strip.removeFromTop(4);

                // Type selector
                c.typeSelector.setBounds(strip.removeFromTop(26));
                strip.removeFromTop(4);

                // Level + Detune knobs (+ WavePos if Wavetable)
                {
                    auto knobRow = strip.removeFromTop(knobSz + labelH);
                    auto placeKnob = [&](juce::Component& comp, juce::Label& lbl)
                    {
                        auto col = knobRow.removeFromLeft(knobSz);
                        comp.setBounds(col.removeFromTop(knobSz));
                        lbl.setBounds(col.removeFromTop(labelH));
                        knobRow.removeFromLeft(8);
                    };
                    placeKnob(c.levelSlider, c.levelLabel);
                    placeKnob(c.detuneSlider, c.detuneLabel);
                    if (isWavetable)
                        placeKnob(c.wavePosSlider, c.wavePosLabel);
                }

                strip.removeFromTop(4);

                // Waveform selector (Classic mode only) or WavePos label
                if (!isWavetable)
                    c.waveformSelector.setBounds(strip.removeFromTop(26).withSizeKeepingCentre(80, 26));

                // Shape type combo
                c.shapeTypeSelector.setBounds(strip.removeFromTop(26).withSizeKeepingCentre(80, 26));
                strip.removeFromTop(4);

                // Shape amount, Self-osc, PhaseDist knobs
                {
                    auto knobRow = strip.removeFromTop(knobSz + labelH);
                    auto placeKnob = [&](juce::Component& comp, juce::Label& lbl)
                    {
                        auto col = knobRow.removeFromLeft(knobSz);
                        comp.setBounds(col.removeFromTop(knobSz));
                        lbl.setBounds(col.removeFromTop(labelH));
                        knobRow.removeFromLeft(8);
                    };
                    if (hasShaping)
                        placeKnob(c.shapeAmtSlider, c.shapeAmtLabel);
                    placeKnob(c.selfOscSlider, c.selfOscLabel);
                    if (isPhaseDist)
                        placeKnob(c.phaseDistSlider, c.phaseDistLabel);
                }

                // Wavetable buttons (if applicable)
                if (isWavetable)
                {
                    auto wtRow = strip.removeFromTop(20);
                    c.loadWTButton.setBounds(wtRow.removeFromLeft(56));
                    wtRow.removeFromLeft(4);
                    c.editWTButton.setBounds(wtRow.removeFromLeft(52));
                }

                xPos += stripW + gap;
            }
        }
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        // SUB / NOISE section
        subNoiseSectionRect = area.removeFromTop(getSectionHeight(kSubNoise));
        sectionHeaderRects[kSubNoise] = subNoiseSectionRect.withHeight(sectionCollapsedH);
        if (sectionExpanded[kSubNoise])
        {
            auto inner = subNoiseSectionRect.reduced(8).withTrimmedTop(18);
            const int halfW = inner.getWidth() / 2;

            // Left half: SUB
            auto subArea = inner.removeFromLeft(halfW).reduced(4, 0);
            subOscLabel.setBounds(subArea.removeFromTop(labelH));
            auto subCtrlRow = subArea.removeFromTop(26);
            subOscEnableButton.setBounds(subCtrlRow.removeFromLeft(40));
            subCtrlRow.removeFromLeft(6);
            subOscWaveSelector.setBounds(subCtrlRow.removeFromLeft(90));
            subArea.removeFromTop(MultiverseFlatTheme::Metrics::smallGap);
            auto subKnobArea = subArea.removeFromTop(knobSz + labelH);
            subOscLevelSlider.setBounds(subKnobArea.removeFromTop(knobSz));
            subOscLevelLabel.setBounds(subKnobArea.removeFromTop(labelH));

            // Right half: NOISE
            auto noiseArea = inner.reduced(4, 0);
            noiseOscLabel.setBounds(noiseArea.removeFromTop(labelH));
            auto noiseCtrlRow = noiseArea.removeFromTop(26);
            noiseOscEnableButton.setBounds(noiseCtrlRow.removeFromLeft(40));
            noiseArea.removeFromTop(MultiverseFlatTheme::Metrics::smallGap);
            auto noiseKnobRow = noiseArea.removeFromTop(knobSz + labelH);
            auto placeNK = [&](NeuKnob& s, juce::Label& l)
            {
                auto col = noiseKnobRow.removeFromLeft(knobSz);
                s.setBounds(col.removeFromTop(knobSz));
                l.setBounds(col.removeFromTop(labelH));
                noiseKnobRow.removeFromLeft(8);
            };
            placeNK(noiseOscLevelSlider, noiseOscLevelLabel);
            placeNK(noiseOscColorSlider, noiseOscColorLabel);
        }
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        // UNISON section
        unisonSectionRect = area.removeFromTop(getSectionHeight(kUnison));
        sectionHeaderRects[kUnison] = unisonSectionRect.withHeight(sectionCollapsedH);
        if (sectionExpanded[kUnison])
        {
            auto inner = unisonSectionRect.reduced(8).withTrimmedTop(18);
            // Voices ComboBox (labeled)
            auto voicesCol = inner.removeFromLeft(100);
            unisonVoicesLabel.setBounds(voicesCol.removeFromBottom(labelH));
            unisonVoicesBox.setBounds(voicesCol.reduced(0, 4));
            inner.removeFromLeft(12);
            // Detune knob
            auto detuneCol = inner.removeFromLeft(knobSz);
            unisonDetuneSlider.setBounds(detuneCol.removeFromTop(knobSz));
            unisonDetuneLabel.setBounds(detuneCol.removeFromTop(labelH));
            inner.removeFromLeft(12);
            // Width knob
            auto widthCol = inner.removeFromLeft(knobSz);
            unisonWidthSlider.setBounds(widthCol.removeFromTop(knobSz));
            unisonWidthLabel.setBounds(widthCol.removeFromTop(labelH));
            inner.removeFromLeft(12);
            // Spread mode
            auto spreadCol = inner.removeFromLeft(130);
            unisonSpreadLabel.setBounds(spreadCol.removeFromBottom(labelH));
            unisonSpreadSelector.setBounds(spreadCol.reduced(0, 4));
        }
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        // FILTER section (with curve display)
        filterSectionRect = area.removeFromTop(getSectionHeight(kFilter));
        sectionHeaderRects[kFilter] = filterSectionRect.withHeight(sectionCollapsedH);
        if (sectionExpanded[kFilter])
        {
            auto inner = filterSectionRect.reduced(8).withTrimmedTop(18);

            // Left: filter curve display
            auto filterDisplayArea = inner.removeFromLeft(juce::jmin(180, inner.getWidth() / 3));
            filterDisplay.setBounds(filterDisplayArea.reduced(4));

            inner.removeFromLeft(8);

            // Right: filter knobs + controls
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

            inner.removeFromTop(MultiverseFlatTheme::Metrics::smallGap);
            auto typeRow = inner.removeFromTop(28);
            filterTypeLabel.setBounds(typeRow.removeFromLeft(36));
            typeRow.removeFromLeft(6);
            filterTypeSelector.setBounds(typeRow.removeFromLeft(90));

            inner.removeFromTop(MultiverseFlatTheme::Metrics::smallGap);
            auto osRow = inner.removeFromTop(28);
            oversamplingLabel.setBounds(osRow.removeFromLeft(30));
            osRow.removeFromLeft(6);
            oversamplingSelector.setBounds(osRow.removeFromLeft(90));
        }
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        // ENV section (with visualizer)
        envSectionRect = area.removeFromTop(getSectionHeight(kEnv));
        sectionHeaderRects[kEnv] = envSectionRect.withHeight(sectionCollapsedH);
        if (sectionExpanded[kEnv])
        {
            auto inner = envSectionRect.reduced(8).withTrimmedTop(18);

            // Left: envelope display
            auto envDisplayArea = inner.removeFromLeft(juce::jmin(200, inner.getWidth() / 2));
            envelopeDisplay.setBounds(envDisplayArea.reduced(4));

            // Right: ADSR knobs
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
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);

        // CHORD / STRUM section
        chordSectionRect = area.removeFromTop(getSectionHeight(kChord));
        sectionHeaderRects[kChord] = chordSectionRect.withHeight(sectionCollapsedH);
        if (sectionExpanded[kChord])
        {
            auto inner = chordSectionRect.reduced(8).withTrimmedTop(18);

            // Row 1: enable toggle + shape label + shape combo
            auto row1 = inner.removeFromTop(26);
            chordEnableButton.setBounds(row1.removeFromLeft(72));
            row1.removeFromLeft(8);
            chordShapeLabel.setBounds(row1.removeFromLeft(44));
            row1.removeFromLeft(4);
            chordShapeSelector.setBounds(row1.removeFromLeft(120));

            inner.removeFromTop(6);

            // Row 2: strum knob + label
            auto row2 = inner;
            auto strumCol = row2.removeFromLeft(knobSz);
            chordStrumSlider.setBounds(strumCol.removeFromTop(knobSz));
            chordStrumLabel.setBounds(strumCol.removeFromTop(labelH));
        }
        area.removeFromTop(MultiverseFlatTheme::Metrics::sectionGap);
    }

    // Readout bar at the very bottom
    readoutBar.setBounds(area.removeFromBottom(28));
}

void SynthPanel::mouseDown(const juce::MouseEvent& e)
{
    const bool isFM = (modeSelector.getSelectedId() == 2);
    if (isFM) return;

    for (int i = 0; i < kNumSections; ++i)
    {
        if (sectionHeaderRects[i].contains(e.getPosition()))
        {
            sectionExpanded[i] = !sectionExpanded[i];
            updateVisibility();
            resized();
            repaint();
            return;
        }
    }
}