#include "SynthPanel.h"
#include "../CyberpunkTheme.h"
#include "../PluginProcessor.h"
#include "../CyberpunkTheme.h"

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

    // 8 Oscillator strips
    for (int osc = 0; osc < 8; ++osc)
    {
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
        c.wtFileLabel.setFont(juce::Font(9.0f));
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
            ed->setBounds(getLocalBounds().reduced(16));
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

    // Add/Remove OSC buttons
    addOscButton.onClick = [this]()
    {
        auto* param = processorRef.apvts.getParameter("oscCount");
        if (param)
        {
            const int current = juce::roundToInt(param->getValue() * 7.0f); // 0-7 → count-1
            if (current < 7)
                param->setValueNotifyingHost((current + 1) / 7.0f);
        }
        updateVisibility();
        resized();
    };
    removeOscButton.onClick = [this]()
    {
        auto* param = processorRef.apvts.getParameter("oscCount");
        if (param)
        {
            const int current = juce::roundToInt(param->getValue() * 7.0f);
            if (current > 0)
                param->setValueNotifyingHost((current - 1) / 7.0f);
        }
        updateVisibility();
        resized();
    };
    addAndMakeVisible(addOscButton);
    addAndMakeVisible(removeOscButton);

    // Waveform selector (legacy, hidden in 3-osc mode)
    setupLabel(waveformLabel, "WAVEFORM");
    addAndMakeVisible(waveformLabel);
    waveformSelector.addItem("Sine",     1);
    waveformSelector.addItem("Saw",      2);
    waveformSelector.addItem("Square",   3);
    waveformSelector.addItem("Triangle", 4);
    waveformSelector.addItem("Noise",    5);
    waveformSelector.setSelectedId(2, juce::dontSendNotification);
    waveformSelector.onChange = [this]
    {
        const auto wf = static_cast<WaveformType>(waveformSelector.getSelectedId() - 1);
        processorRef.baseWaveform = wf;
        oscDisplay.setWaveform(wf);
        // Legacy: also update Osc 1 classic waveform
        auto& c = oscControls[0];
        c.waveformSelector.setSelectedId(waveformSelector.getSelectedId(), juce::dontSendNotification);
    };
    addAndMakeVisible(waveformSelector);

    // Visualisations
    oscDisplay.setWaveform(processorRef.baseWaveform);
    addAndMakeVisible(oscDisplay);
    addAndMakeVisible(lfoDisplay);
    synthDisplay.setProcessor(&processorRef);
    addAndMakeVisible(synthDisplay);

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
    // APVTS attachments
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
    mpeButton.setTooltip("Enable MPE: per-note pitch bend (±48 st), pressure, and slide from expressive controllers");
    addAndMakeVisible(mpeButton);
    mpeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "mpeEnabled", mpeButton);

    // Tooltips
    modeSelector.setTooltip      ("Synthesis mode: Classic (subtractive) for 3 oscillators, or FM");
    waveformSelector.setTooltip  ("Legacy oscillator waveform selector");
    savePresetButton.setTooltip  ("Save current state to a preset file");
    loadPresetButton.setTooltip  ("Load a previously saved preset file");

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
    addOscButton.setTooltip   ("Add an oscillator (max 8)");
    removeOscButton.setTooltip("Remove the last oscillator (min 1)");
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

//==============================================================================
void SynthPanel::updateVisibility()
{
    const bool isFM = (modeSelector.getSelectedId() == 2);

    waveformLabel.setVisible(!isFM);
    waveformSelector.setVisible(!isFM);
    oscDisplay.setVisible(!isFM);
    lfoDisplay.setVisible(false);
    synthDisplay.setVisible(!isFM);

    // 8 Osc controls visibility (only show up to oscCount active strips)
    const int activeOscCount = static_cast<int>(*processorRef.apvts.getRawParameterValue("oscCount")) + 1;
    for (int i = 0; i < 8; ++i)
    {
        auto& c = oscControls[i];
        const bool active = (!isFM) && (i < activeOscCount);
        const bool isWavetable  = c.typeSelector.getSelectedId() == 2;
        const bool isPhaseDist  = c.typeSelector.getSelectedId() == 4;
        const bool hasShaping   = active && (c.shapeTypeSelector.getSelectedId() > 1);
        c.sectionLabel.setVisible(active);
        c.typeSelector.setVisible(active);
        c.levelLabel.setVisible(active);    c.levelSlider.setVisible(active);
        c.detuneLabel.setVisible(active);   c.detuneSlider.setVisible(active);
        c.waveformSelector.setVisible(active && !isWavetable);
        c.wavePosLabel.setVisible(active && isWavetable);
        c.wavePosSlider.setVisible(active && isWavetable);
        c.loadWTButton.setVisible(active && isWavetable);
        c.editWTButton.setVisible(active && isWavetable);
        c.wtFileLabel.setVisible(active && isWavetable);
        c.shapeTypeSelector.setVisible(active);
        c.shapeAmtLabel.setVisible(active && hasShaping);
        c.shapeAmtSlider.setVisible(active && hasShaping);
        c.selfOscLabel.setVisible(active);
        c.selfOscSlider.setVisible(active);
        c.phaseDistLabel.setVisible(active && isPhaseDist);
        c.phaseDistSlider.setVisible(active && isPhaseDist);
    }
    addOscButton.setVisible(!isFM);
    removeOscButton.setVisible(!isFM);

    unisonVoicesLabel.setVisible(!isFM); unisonVoicesBox.setVisible(!isFM);
    unisonDetuneLabel.setVisible(!isFM); unisonDetuneSlider.setVisible(!isFM);
    unisonWidthLabel.setVisible(!isFM);  unisonWidthSlider.setVisible(!isFM);
    unisonSpreadLabel.setVisible(!isFM); unisonSpreadSelector.setVisible(!isFM);

    attackLabel.setVisible(!isFM);  attackSlider.setVisible(!isFM);
    decayLabel.setVisible(!isFM);   decaySlider.setVisible(!isFM);
    sustainLabel.setVisible(!isFM); sustainSlider.setVisible(!isFM);
    releaseLabel.setVisible(!isFM); releaseSlider.setVisible(!isFM);

    cutoffLabel.setVisible(!isFM);        cutoffSlider.setVisible(!isFM);
    resonanceLabel.setVisible(!isFM);     resonanceSlider.setVisible(!isFM);
    oversamplingLabel.setVisible(!isFM);  oversamplingSelector.setVisible(!isFM);
    filterTypeLabel.setVisible(!isFM);    filterTypeSelector.setVisible(!isFM);

    subOscLabel.setVisible(!isFM);       subOscEnableButton.setVisible(!isFM);
    subOscLevelSlider.setVisible(!isFM); subOscLevelLabel.setVisible(!isFM);
    subOscWaveSelector.setVisible(!isFM);
    noiseOscLabel.setVisible(!isFM);          noiseOscEnableButton.setVisible(!isFM);
    noiseOscLevelSlider.setVisible(!isFM);    noiseOscLevelLabel.setVisible(!isFM);
    noiseOscColorSlider.setVisible(!isFM);    noiseOscColorLabel.setVisible(!isFM);

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
    g.fillAll(CyberpunkTheme::bgBase);

    const bool isFM = (modeSelector.getSelectedId() == 2);

    if (!isFM)
    {
        drawSection(g, oscSectionRect,      "OSC");
        drawSection(g, subNoiseSectionRect, "SUB / NOISE");
        drawSection(g, unisonSectionRect,   "UNISON");
        drawSection(g, filterSectionRect,   "FILTER");
        drawSection(g, envSectionRect,      "ENV");
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
    const juce::Colour badgeCol = isFM ? CyberpunkTheme::accentBlue.withAlpha(0.7f) : CyberpunkTheme::accentBlue;
    g.setColour(badgeCol.withAlpha(0.18f));
    g.fillRoundedRectangle(modeBadgeRect.toFloat(), 4.0f);
    g.setColour(badgeCol);
    g.drawRoundedRectangle(modeBadgeRect.toFloat(), 4.0f, 1.0f);
    g.setColour(badgeCol);
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
    area.removeFromTop(10);

    if (isFM)
    {
        fmSectionRect = area.withHeight(46 + 8 + 4 * (16 + 60 + 16 + 8));
        auto fmArea = fmSectionRect.reduced(8);

        auto algRow = fmArea.removeFromTop(46);
        algorithmLabel.setBounds(algRow.removeFromTop(18));
        algorithmSelector.setBounds(algRow.removeFromTop(26));
        fmArea.removeFromTop(8);

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
                knobs.removeFromLeft(4);
            };
            place(c.ratioSlider,    c.ratioLabel);
            place(c.levelSlider,    c.levelLabel);
            place(c.feedbackSlider, c.feedbackLabel);
            place(c.attackSlider,   c.attackLabel);
            place(c.decaySlider,    c.decayLabel);
            place(c.sustainSlider,  c.sustainLabel);
            place(c.releaseSlider,  c.releaseLabel);
            fmArea.removeFromTop(8);
        }
        area.removeFromTop(fmSectionRect.getHeight() + 8);
    }
    else
    {
        const int knobSz = 70;
        const int labelH = 18;

        // OSC strips (1–8, up to 2 rows of 4)
        const int activeOscCount = static_cast<int>(*processorRef.apvts.getRawParameterValue("oscCount")) + 1;
        const int numRows = (activeOscCount > 4) ? 2 : 1;
        const int perRowHeight = 220;
        oscSectionRect = area.removeFromTop(numRows * perRowHeight + 20);

        // Add/Remove buttons row at top of OSC section
        {
            auto btnRow = oscSectionRect.removeFromTop(20).reduced(8, 0);
            addOscButton.setBounds(btnRow.removeFromLeft(60));
            btnRow.removeFromLeft(4);
            removeOscButton.setBounds(btnRow.removeFromLeft(60));
        }

        // Layout strips in rows of 4
        auto oscArea = oscSectionRect;
        for (int row = 0; row < numRows; ++row)
        {
            const int rowStart = row * 4;
            const int rowEnd   = juce::jmin(rowStart + 4, activeOscCount);
            const int count    = rowEnd - rowStart;
            if (count <= 0) break;

            auto rowRect = oscArea.removeFromTop(perRowHeight);
            auto inner   = rowRect.reduced(8).withTrimmedTop(8);
            const int stripW = inner.getWidth() / count;

            for (int i = rowStart; i < rowEnd; ++i)
            {
                auto strip = inner.removeFromLeft(stripW).reduced(4, 0);
                auto& c = oscControls[i];
                c.sectionLabel.setBounds(strip.removeFromTop(16));
                c.typeSelector.setBounds(strip.removeFromTop(26));
                strip.removeFromTop(4);

                auto knobArea = strip;
                const bool isWavetable = c.typeSelector.getSelectedId() == 2;
                const bool isPhaseDist = c.typeSelector.getSelectedId() == 4;
                const bool hasShape    = c.shapeTypeSelector.getSelectedId() > 1;

                auto placeKnob = [&](juce::Component& comp, juce::Label& lbl)
                {
                    auto col = knobArea.removeFromTop(knobSz + labelH);
                    comp.setBounds(col.removeFromTop(knobSz));
                    lbl.setBounds(col.removeFromTop(labelH));
                    knobArea.removeFromTop(2);
                };
                placeKnob(c.levelSlider, c.levelLabel);
                placeKnob(c.detuneSlider, c.detuneLabel);
                if (isWavetable)
                    placeKnob(c.wavePosSlider, c.wavePosLabel);
                else
                    c.waveformSelector.setBounds(knobArea.removeFromTop(26));
                if (isPhaseDist)
                    placeKnob(c.phaseDistSlider, c.phaseDistLabel);

                // Shape row
                auto shapeRow = knobArea.removeFromTop(26);
                c.shapeTypeSelector.setBounds(shapeRow.removeFromLeft(shapeRow.getWidth() * 3 / 5));
                if (hasShape)
                    placeKnob(c.shapeAmtSlider, c.shapeAmtLabel);
                placeKnob(c.selfOscSlider, c.selfOscLabel);

                if (isWavetable)
                {
                    auto btnRow = knobArea.removeFromTop(20);
                    c.loadWTButton.setBounds(btnRow.removeFromLeft(60));
                    btnRow.removeFromLeft(4);
                    c.editWTButton.setBounds(btnRow.removeFromLeft(56));
                    btnRow.removeFromLeft(4);
                    c.wtFileLabel.setBounds(btnRow);
                }
            }
        }
        area.removeFromTop(8);

        // SUB / NOISE section
        subNoiseSectionRect = area.removeFromTop(126);
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
            subArea.removeFromTop(4);
            auto subKnobArea = subArea.removeFromTop(knobSz + labelH);
            subOscLevelSlider.setBounds(subKnobArea.removeFromTop(knobSz));
            subOscLevelLabel.setBounds(subKnobArea.removeFromTop(labelH));

            // Right half: NOISE
            auto noiseArea = inner.reduced(4, 0);
            noiseOscLabel.setBounds(noiseArea.removeFromTop(labelH));
            auto noiseCtrlRow = noiseArea.removeFromTop(26);
            noiseOscEnableButton.setBounds(noiseCtrlRow.removeFromLeft(40));
            noiseArea.removeFromTop(4);
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
        area.removeFromTop(8);

        // UNISON section
        unisonSectionRect = area.removeFromTop(120);
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
        area.removeFromTop(8);

        // FILTER section
        filterSectionRect = area.removeFromTop(185);
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
            auto typeRow = inner.removeFromTop(28);
            filterTypeLabel.setBounds(typeRow.removeFromLeft(36));
            typeRow.removeFromLeft(6);
            filterTypeSelector.setBounds(typeRow.removeFromLeft(90));

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

        // Waveform / spectrum display
        synthDisplay.setBounds(area.removeFromTop(90));
    }
}
