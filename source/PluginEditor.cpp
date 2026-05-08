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

    // Preset navigation (header)
    prevPresetButton.addListener (this);
    nextPresetButton.addListener (this);
    favoriteButton.addListener (this);
    backButton.addListener (this);
    forwardButton.addListener (this);

    presetNameLabel.setJustificationType (juce::Justification::centred);
    presetNameLabel.setFont (juce::Font (12.0f, juce::Font::bold));
    presetNameLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    presetNameLabel.setColour (juce::Label::backgroundColourId, MultiverseFlatTheme::bgDeep);
    presetNameLabel.setColour (juce::Label::outlineColourId, MultiverseFlatTheme::accentBlue.withAlpha (0.3f));
    presetNameLabel.setMinimumHorizontalScale (0.8f);

    for (auto* btn : { &prevPresetButton, &nextPresetButton, &favoriteButton, &backButton, &forwardButton })
    {
        btn->setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep);
        btn->setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textSecondary);
        addAndMakeVisible (btn);
    }
    addAndMakeVisible (presetNameLabel);

    updatePresetNameLabel();
    updateFavoriteButtonColor();

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
    tabs.addTab ("Synth",      MultiverseFlatTheme::bgBase, &synthPanel,            false);
    tabs.addTab ("Drums",      MultiverseFlatTheme::bgBase, &drumSequencerPanel,    false);
    tabs.addTab ("Modulation", MultiverseFlatTheme::bgBase, &modulationMatrixPanel, false);
    tabs.addTab ("Sampler",    MultiverseFlatTheme::bgBase, &samplerPanel,          false);
    tabs.addTab ("Sequencer",  MultiverseFlatTheme::bgBase, &proSequencerPanel,     false);
    tabs.addTab ("Arp",        MultiverseFlatTheme::bgBase, &arpeggiatorPanel,      false);
    tabs.addTab ("Effects",    MultiverseFlatTheme::bgBase, &effectsPanel,          false);
    tabs.addTab ("Macros",     MultiverseFlatTheme::bgBase, &macroPanel,            false);
    tabs.addTab ("Granular",   MultiverseFlatTheme::bgBase, &granularPanel,         false);
    tabs.addTab ("Layers",     MultiverseFlatTheme::bgBase, &layersPanel,           false);
    tabs.addTab ("Perf",       MultiverseFlatTheme::bgBase, &performancePanel,      false);
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

    // Right side controls
    presetsButton.setBounds   (header.removeFromRight (72).reduced (4, 4));
    helpButton.setBounds      (header.removeFromRight (28).reduced (2, 4));
    randomizeButton.setBounds (header.removeFromRight (52).reduced (4, 4));
    scaleCombo.setBounds      (header.removeFromRight (70).reduced (4, 4));
    qualCombo.setBounds       (header.removeFromRight (80).reduced (4, 4));
    fxModeButton.setBounds    (header.removeFromRight (36).reduced (4, 4));

    // Center: preset navigation strip (prev | name | next | ★ | ◀ | ▶)
    auto center = header.withSize (320, 32).withCentre (header.getCentre());
    backButton.setBounds      (center.removeFromLeft (28).reduced (2, 4));
    prevPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    presetNameLabel.setBounds  (center.removeFromLeft (180).reduced (4, 4));
    nextPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    favoriteButton.setBounds  (center.removeFromLeft (28).reduced (2, 4));
    forwardButton.setBounds   (center.removeFromLeft (28).reduced (2, 4));

    // Preset browser (collapsible, 280px)
    if (presetsVisible)
    {
        presetBrowserPanel.setBounds (area.removeFromTop (280));
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

    if (button == &prevPresetButton)   { navigatePresetPrev();  return; }
    if (button == &nextPresetButton)   { navigatePresetNext();  return; }
    if (button == &favoriteButton)     { cycleFavorite();       return; }
    if (button == &backButton)
    {
        if (processorRef.getPresetManager().canGoBack())
        {
            int idx = processorRef.getPresetManager().goBack();
            processorRef.cancelPreviewNote();
            processorRef.loadPresetAtIndex (idx);
            updatePresetNameLabel();
            updateFavoriteButtonColor();
        }
        return;
    }
    if (button == &forwardButton)
    {
        if (processorRef.getPresetManager().canGoForward())
        {
            int idx = processorRef.getPresetManager().goForward();
            processorRef.cancelPreviewNote();
            processorRef.loadPresetAtIndex (idx);
            updatePresetNameLabel();
            updateFavoriteButtonColor();
        }
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
    // Cmd+F: focus preset search
    if (key == juce::KeyPress ('f', juce::ModifierKeys::commandModifier, 0))
    {
        if (!presetsVisible)
        {
            presetsVisible = true;
            presetBrowserPanel.refresh();
            resized();
        }
        presetBrowserPanel.focusSearchEditor();
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

//==============================================================================
// Preset navigation methods

void PluginEditor::updatePresetNameLabel()
{
    auto& pm = processorRef.getPresetManager();
    juce::String name = pm.getCurrentPreset().getName();
    if (name.isEmpty())
        name = "Init";
    presetNameLabel.setText (name, juce::dontSendNotification);
}

void PluginEditor::updateFavoriteButtonColor()
{
    auto& pm = processorRef.getPresetManager();
    juce::String currentName = pm.getCurrentPreset().getName();

    // Find the preset index by name
    auto names = pm.getPresetNames();
    int idx = names.indexOf (currentName);

    if (idx >= 0 && pm.isFavorite (idx))
    {
        int colorIdx = pm.getFavoriteColor (idx);
        if (colorIdx >= 0 && colorIdx < PresetManager::NUM_FAV_COLORS)
        {
            static const juce::Colour favColors[8] = {
                juce::Colour(0xFFFF3B30), juce::Colour(0xFFFF9500), juce::Colour(0xFFFFCC00),
                juce::Colour(0xFF34C759), juce::Colour(0xFF00C7BE), juce::Colour(0xFF007AFF),
                juce::Colour(0xFFAF52DE), juce::Colour(0xFFFF2D55)
            };
            favoriteButton.setColour (juce::TextButton::textColourOffId, favColors[colorIdx]);
            favoriteButton.setColour (juce::TextButton::textColourOnId, favColors[colorIdx]);
            return;
        }
    }
    favoriteButton.setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textSecondary);
    favoriteButton.setColour (juce::TextButton::textColourOnId, MultiverseFlatTheme::textSecondary);
}

void PluginEditor::navigatePresetPrev()
{
    auto& pm = processorRef.getPresetManager();
    pm.previousPreset();
    auto& current = pm.getCurrentPreset();
    int idx = pm.getPresetNames().indexOf (current.getName());
    processorRef.cancelPreviewNote();
    if (idx >= 0)
        processorRef.loadPresetAtIndex (idx);
    updatePresetNameLabel();
    updateFavoriteButtonColor();
}

void PluginEditor::navigatePresetNext()
{
    auto& pm = processorRef.getPresetManager();
    pm.nextPreset();
    auto& current = pm.getCurrentPreset();
    int idx = pm.getPresetNames().indexOf (current.getName());
    processorRef.cancelPreviewNote();
    if (idx >= 0)
        processorRef.loadPresetAtIndex (idx);
    updatePresetNameLabel();
    updateFavoriteButtonColor();
}

void PluginEditor::cycleFavorite()
{
    auto& pm = processorRef.getPresetManager();
    juce::String currentName = pm.getCurrentPreset().getName();
    auto names = pm.getPresetNames();
    int idx = names.indexOf (currentName);

    if (idx >= 0)
    {
        int currentColor = pm.getFavoriteColor (idx);
        // Cycle: -1 -> 0, 0 -> 1, ..., 6 -> 7, 7 -> -1
        int newColor = (currentColor + 1) % PresetManager::NUM_FAV_COLORS;
        // If was not favorite (colorIdx == -1), set to 0; if was 7, wrap to -1
        if (currentColor < 0)
            newColor = 0;
        else if (currentColor >= PresetManager::NUM_FAV_COLORS - 1)
            newColor = -1;
        pm.setFavorite (idx, newColor);
        updateFavoriteButtonColor();
        if (presetsVisible)
            presetBrowserPanel.refresh();
    }
}
