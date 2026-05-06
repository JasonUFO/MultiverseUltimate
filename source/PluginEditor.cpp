#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (p), mvTheme(), processorRef (p),
      drumSequencerPanel (p.getDrumSequencer()),
      modulationMatrixPanel (p, p.getModulationMatrix()),
      samplerPanel (p, p.getSamplerEngine()),
      proSequencerPanel (p.getProSequencer()),
      arpeggiatorPanel (p.getArpeggiator()),
      synthPanel (p),
      effectsPanel (p),
      macroPanel (p),
      granularPanel (p),
      layersPanel (p, p.getLayerManager()),
      performancePanel (p),
      tabs (juce::TabbedButtonBar::TabsAtTop),
      midiLearnButton ("MIDI Learn"),
      midiLearnLabel ("", ""),
      presetBrowserPanel (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&mvTheme);

    setupTabs();
    setupMidiLearnButton();

    presetsButton.addListener (this);
    addAndMakeVisible (presetsButton);
    addChildComponent (presetBrowserPanel);

    helpButton.setClickingTogglesState (true);
    helpButton.setToggleState (true, juce::dontSendNotification);
    helpButton.onClick = [this]
    {
        tooltipWindow.setMillisecondsBeforeTipAppears (
            helpButton.getToggleState() ? 700 : 99999999);
    };
    addAndMakeVisible (helpButton);

    // Randomize button
    randomizeButton.setTooltip ("Randomize synth parameters for sound design");
    randomizeButton.onClick = [this] { showRandomizeMenu(); };
    addAndMakeVisible (randomizeButton);

    // Scale combo
    scaleCombo.addItem ("75%",  1);
    scaleCombo.addItem ("100%", 2);
    scaleCombo.addItem ("125%", 3);
    scaleCombo.addItem ("150%", 4);
    scaleCombo.setSelectedId (2, juce::dontSendNotification);
    scaleCombo.setTooltip ("UI scale factor");
    scaleCombo.onChange = [this] {
        const float factors[] = { 0.75f, 1.0f, 1.25f, 1.5f };
        int idx = juce::jlimit (0, 3, scaleCombo.getSelectedId() - 1);
        setSize (juce::roundToInt (1200 * factors[idx]),
                 juce::roundToInt (800  * factors[idx]));
    };
    addAndMakeVisible (scaleCombo);

    // Global quality combo
    qualCombo.addItem ("Off",      1);
    qualCombo.addItem ("2x High",  2);
    qualCombo.addItem ("4x Ultra", 3);
    qualCombo.setTooltip ("Global oversampling quality — higher quality reduces aliasing, increases CPU (takes effect on next play)");
    qualAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        p.apvts, "globalQuality", qualCombo);
    addAndMakeVisible (qualCombo);

    // FX Mode button
    fxModeButton.setClickingTogglesState (true);
    fxModeButton.setTooltip ("FX Mode — route audio input through the effects chain (connect audio track to this plugin)");
    fxModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.apvts, "fxModeEnabled", fxModeButton);
    addAndMakeVisible (fxModeButton);

    // Built-in keyboard
    keyboard.setOctaveForMiddleC (5);
    keyboard.setAvailableRange (24, 108);
    keyboard.setLowestVisibleKey (36);
    addAndMakeVisible (keyboard);

    addAndMakeVisible (tabs);
    addAndMakeVisible (midiLearnButton);
    addAndMakeVisible (midiLearnLabel);
    addAndMakeVisible (paramSelector);

    setResizable (true, true);
    setResizeLimits (800, 533, 1920, 1280);
    setSize (1200, 800);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

void PluginEditor::setupTabs()
{
    tabs.addTab ("Synth",      CyberpunkTheme::bgBase, &synthPanel,            false);
    tabs.addTab ("Drums",      CyberpunkTheme::bgBase, &drumSequencerPanel,    false);
    tabs.addTab ("Modulation", CyberpunkTheme::bgBase, &modulationMatrixPanel, false);
    tabs.addTab ("Sampler",    CyberpunkTheme::bgBase, &samplerPanel,          false);
    tabs.addTab ("Sequencer",  CyberpunkTheme::bgBase, &proSequencerPanel,     false);
    tabs.addTab ("Arp",        CyberpunkTheme::bgBase, &arpeggiatorPanel,      false);
    tabs.addTab ("Effects",    CyberpunkTheme::bgBase, &effectsPanel,          false);
    tabs.addTab ("Macros",     CyberpunkTheme::bgBase, &macroPanel,            false);
    tabs.addTab ("Granular",   CyberpunkTheme::bgBase, &granularPanel,         false);
    tabs.addTab ("Layers",     CyberpunkTheme::bgBase, &layersPanel,           false);
    tabs.addTab ("Perf",       CyberpunkTheme::bgBase, &performancePanel,      false);
}

void PluginEditor::setupMidiLearnButton()
{
    midiLearnButton.setToggleState (false, juce::dontSendNotification);
    midiLearnButton.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    midiLearnButton.setTooltip ("Enable MIDI Learn — select a parameter, then move a controller");
    midiLearnButton.addListener (this);

    midiLearnLabel.setJustificationType (juce::Justification::centredLeft);
    midiLearnLabel.setFont (juce::Font (11.0f, juce::Font::plain));
    midiLearnLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    midiLearnLabel.setVisible (false);

    // Populate parameter selector with all APVTS parameters
    paramSelector.addItem ("(select parameter)", 1);
    auto& params = processorRef.getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(params[i]))
            paramSelector.addItem (rp->getName (64), i + 2); // offset by 2: id 1 = placeholder
    }
    paramSelector.setSelectedId (1, juce::dontSendNotification);
    paramSelector.setVisible (false);
    paramSelector.addListener (this);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Header strip: 32 px tall
    auto header = area.removeFromTop (32);
    midiLearnButton.setBounds (header.removeFromLeft (110).reduced (4, 4));
    paramSelector.setBounds   (header.removeFromLeft (200).reduced (2, 4));
    presetsButton.setBounds   (header.removeFromRight (72).reduced (4, 4));
    helpButton.setBounds      (header.removeFromRight (28).reduced (2, 4));
    randomizeButton.setBounds (header.removeFromRight (52).reduced (4, 4));
    scaleCombo.setBounds      (header.removeFromRight (70).reduced (4, 4));
    qualCombo.setBounds       (header.removeFromRight (80).reduced (4, 4));
    fxModeButton.setBounds    (header.removeFromRight (36).reduced (4, 4));
    midiLearnLabel.setBounds  (header.reduced (4, 4));

    // Preset browser (collapsible, 220px)
    if (presetsVisible)
    {
        presetBrowserPanel.setBounds (area.removeFromTop (220));
        presetBrowserPanel.setVisible (true);
    }
    else
    {
        presetBrowserPanel.setVisible (false);
    }

    // Built-in keyboard at bottom
    keyboard.setBounds (area.removeFromBottom (KEYBOARD_H));

    tabs.setBounds (area);
}

void PluginEditor::buttonClicked (juce::Button* button)
{
    if (button == &presetsButton)
    {
        presetsVisible = !presetsVisible;
        if (presetsVisible)
            presetBrowserPanel.refresh();
        resized();
        return;
    }

    if (button != &midiLearnButton)
        return;

    const bool isActive = midiLearnButton.getToggleState();
    processorRef.midiLearnActive = isActive;

    if (!isActive)
    {
        processorRef.stopMidiLearn();
        paramSelector.setSelectedId (1, juce::dontSendNotification);
    }

    updateMidiLearnUI();
}

void PluginEditor::comboBoxChanged (juce::ComboBox* combo)
{
    if (combo == &paramSelector)
    {
        const int selectedId = paramSelector.getSelectedId();
        if (selectedId <= 1)
        {
            processorRef.learnParameterIndex = -1;
            return;
        }
        const int paramIndex = selectedId - 2;
        processorRef.startMidiLearnForParameter (paramIndex);
        midiLearnLabel.setText ("Waiting for CC...", juce::dontSendNotification);
        midiLearnLabel.setVisible (true);
    }
}

void PluginEditor::updateMidiLearnUI()
{
    const bool active = midiLearnButton.getToggleState();
    paramSelector.setVisible (active);

    if (!active)
    {
        midiLearnLabel.setVisible (false);
        midiLearnLabel.setText ("", juce::dontSendNotification);
    }
}

bool PluginEditor::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier, 0))
    {
        processorRef.undoManager.undo();
        return true;
    }
    if (key == juce::KeyPress ('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        processorRef.undoManager.redo();
        return true;
    }
    return false;
}

void PluginEditor::showRandomizeMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Randomize OSC + Filter + Env");
    menu.addItem (2, "Randomize Filter + Env");
    menu.addItem (3, "Randomize LFOs");
    menu.addSeparator();
    menu.addItem (4, "Randomize Everything");

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (randomizeButton),
        [this] (int result)
        {
            switch (result)
            {
                case 1:
                    randomizeParams ({"osc", "filterCutoff", "filterResonance", "filterType",
                                      "attack", "decay", "sustain", "release"});
                    break;
                case 2:
                    randomizeParams ({"filterCutoff", "filterResonance",
                                      "attack", "decay", "sustain", "release"});
                    break;
                case 3:
                    randomizeParams ({"lfo1Rate", "lfo2Rate", "lfo3Rate", "lfo4Rate",
                                      "lfo5Rate", "lfo6Rate", "lfo7Rate", "lfo8Rate",
                                      "lfo1Shape", "lfo2Shape", "lfo3Shape", "lfo4Shape",
                                      "lfo5Shape", "lfo6Shape", "lfo7Shape", "lfo8Shape"});
                    break;
                case 4:
                    randomizeParams ({});  // empty = all params
                    break;
                default:
                    break;
            }
        });
}

void PluginEditor::randomizeParams(const juce::StringArray& prefixes, bool filterBoring)
{
    // Params that should never be randomized
    static const juce::StringArray skipList {
        "masterVolume", "mpeEnabled", "portaAlways", "oscCount",
        "samplerVolume", "samplerPan", "fmAlgorithm"
    };

    auto& params = processorRef.getParameters();
    juce::Random rng;

    for (auto* p : params)
    {
        auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p);
        if (!rp) continue;

        const juce::String id = rp->getParameterID();

        // Always skip boring/structural params
        if (skipList.contains (id)) continue;
        if (id.startsWith ("lfo") && id.endsWith ("Sync")) continue;
        if (id.startsWith ("lfo") && id.endsWith ("SyncDiv")) continue;

        // If a prefix filter is given, only randomize matching params
        if (!prefixes.isEmpty())
        {
            bool matches = false;
            for (const auto& prefix : prefixes)
                if (id.startsWith (prefix)) { matches = true; break; }
            if (!matches) continue;
        }

        rp->setValueNotifyingHost (rng.nextFloat());
    }
}
