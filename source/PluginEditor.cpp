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
      granularPanel (p),
      layersPanel (p, p.getLayerManager()),
      performancePanel (p),
      routingPanel (p),
      tabs (juce::TabbedButtonBar::TabsAtTop),
      librarianPanel (p),
      modBar (p)
{
    setLookAndFeel (&mvTheme);

    setupTabs();

    // Add PRE tab for librarian
    addAndMakeVisible (librarianPanel);

    // Librarian panel (added to PRE tab in setupTabs)

    // Preset navigation (header)
    prevPresetButton.addListener (this);
    nextPresetButton.addListener (this);
    favoriteButton.addListener (this);
    backButton.addListener (this);
    forwardButton.addListener (this);

    presetNameLabel.setJustificationType (juce::Justification::centred);
    presetNameLabel.setFont (MultiverseFlatTheme::titleFont());
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

    // Menu button (☰)
    menuButton.setTooltip ("Menu — Save, Import, Export, Scale, Quality, FX Mode, MIDI Learn");
    menuButton.setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep);
    menuButton.setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textPrimary);
    menuButton.addListener (this);
    addAndMakeVisible (menuButton);

    // Randomize button
    randomizeButton.setTooltip ("Randomize synth parameters for sound design");
    randomizeButton.onClick = [this] { showRandomizeMenu(); };
    addAndMakeVisible (randomizeButton);

    // Scale combo (hidden — controlled via menu)
    scaleCombo.addItem ("75%",  1);
    scaleCombo.addItem ("100%", 2);
    scaleCombo.addItem ("125%", 3);
    scaleCombo.addItem ("150%", 4);
    scaleCombo.setSelectedId (2, juce::dontSendNotification);

    // Global quality combo (hidden — controlled via menu)
    qualCombo.addItem ("Off",      1);
    qualCombo.addItem ("2x High",  2);
    qualCombo.addItem ("4x Ultra", 3);
    qualAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        p.apvts, "globalQuality", qualCombo);

    // FX Mode button (hidden — controlled via menu)
    fxModeButton.setClickingTogglesState (true);
    fxModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.apvts, "fxModeEnabled", fxModeButton);

    // Mod bar (ENV/LFO/MACRO/QFX/KEY sub-tabs)
    addAndMakeVisible (modBar);

    addAndMakeVisible (tabs);

    // Mod-drag overlay (always on top for connection lines)
    addAndMakeVisible (modDragOverlay);
    modDragOverlay.setInterceptsMouseClicks (false, false);

    // Routing panel tab-switch callback
    routingPanel.onSwitchToTab = [this] (int index)
    {
        tabs.setCurrentTabIndex (index, true);
    };

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
    tabs.addTab ("PRE",  MultiverseFlatTheme::bgBase, &librarianPanel,        false);
    tabs.addTab ("SYN",  MultiverseFlatTheme::bgBase, &synthPanel,            false);
    tabs.addTab ("DRM",  MultiverseFlatTheme::bgBase, &drumSequencerPanel,    false);
    tabs.addTab ("MOD",  MultiverseFlatTheme::bgBase, &modulationMatrixPanel, false);
    tabs.addTab ("SMP",  MultiverseFlatTheme::bgBase, &samplerPanel,          false);
    tabs.addTab ("SEQ",  MultiverseFlatTheme::bgBase, &proSequencerPanel,     false);
    tabs.addTab ("ARP",  MultiverseFlatTheme::bgBase, &arpeggiatorPanel,      false);
    tabs.addTab ("FX",   MultiverseFlatTheme::bgBase, &effectsPanel,          false);
    tabs.addTab ("GRN",  MultiverseFlatTheme::bgBase, &granularPanel,         false);
    tabs.addTab ("LYR",  MultiverseFlatTheme::bgBase, &layersPanel,           false);
    tabs.addTab ("PRF",  MultiverseFlatTheme::bgBase, &performancePanel,      false);
    tabs.addTab ("ROU",  MultiverseFlatTheme::bgBase, &routingPanel,          false);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (MultiverseFlatTheme::bgVoid);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Header: 32px across full width
    auto header = area.removeFromTop (32);
    menuButton.setBounds (header.removeFromLeft (32).reduced (4, 4));
    randomizeButton.setBounds (header.removeFromRight (52).reduced (4, 4));

    // Center: preset navigation strip
    auto center = header.withSize (320, 32).withCentre (header.getCentre());
    backButton.setBounds      (center.removeFromLeft (28).reduced (2, 4));
    prevPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    presetNameLabel.setBounds  (center.removeFromLeft (180).reduced (4, 4));
    nextPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    favoriteButton.setBounds  (center.removeFromLeft (28).reduced (2, 4));
    forwardButton.setBounds   (center.removeFromLeft (28).reduced (2, 4));

    // Mod bar: 160px at bottom, full width
    auto bottomArea = area.removeFromBottom (ModBar::MOD_BAR_H);
    modBar.setBounds (bottomArea);

    // Tabs fill remaining center area (full width, no right strip)
    tabs.setBounds (area);

    // Mod-drag overlay covers full editor (always on top)
    modDragOverlay.setBounds (getLocalBounds());
    modDragOverlay.toFront (false);
}

void PluginEditor::buttonClicked (juce::Button* button)
{
    if (button == &menuButton)
    {
        showMainMenu();
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
}

void PluginEditor::comboBoxChanged (juce::ComboBox* combo)
{
    if (combo == midiLearnCallout.getComponent())
    {
        const int selectedId = combo->getSelectedId();
        if (selectedId <= 1)
        {
            processorRef.learnParameterIndex = -1;
            return;
        }
        const int paramIndex = selectedId - 2;
        processorRef.startMidiLearnForParameter (paramIndex);
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
    // Cmd+F: focus preset search (sidebar is always visible)
    if (key == juce::KeyPress ('f', juce::ModifierKeys::commandModifier, 0))
    {
        librarianPanel.focusSearchEditor();
        return true;
    }
    return false;
}

void PluginEditor::showMainMenu()
{
    juce::PopupMenu menu;

    // Preset actions
    menu.addItem ("Save Preset", [this] { librarianPanel.saveCurrentPreset(); });

    // Import/Export
    menu.addItem ("Import Preset", [this] { librarianPanel.importPreset(); });
    menu.addItem ("Export Preset", [this] { librarianPanel.exportPreset(); });

    menu.addSeparator();

    // UI Scale submenu
    juce::PopupMenu scaleMenu;
    const float factors[] = { 0.75f, 1.0f, 1.25f, 1.5f };
    const char* scaleLabels[] = { "75%", "100%", "125%", "150%" };
    for (int i = 0; i < 4; ++i)
    {
        const int id = i + 1;
        scaleMenu.addItem (scaleLabels[i], true, scaleCombo.getSelectedId() == id,
            [this, id] {
                scaleCombo.setSelectedId (id);
                const float f[] = { 0.75f, 1.0f, 1.25f, 1.5f };
                setSize (juce::roundToInt (1200 * f[id - 1]),
                         juce::roundToInt (800  * f[id - 1]));
            });
    }
    menu.addSubMenu ("UI Scale", scaleMenu);

    // Quality submenu
    juce::PopupMenu qualMenu;
    qualMenu.addItem ("Off (no oversampling)",     true, qualCombo.getSelectedId() == 1, [this] { qualCombo.setSelectedId (1); });
    qualMenu.addItem ("2x High Quality",           true, qualCombo.getSelectedId() == 2, [this] { qualCombo.setSelectedId (2); });
    qualMenu.addItem ("4x Ultra Quality",           true, qualCombo.getSelectedId() == 3, [this] { qualCombo.setSelectedId (3); });
    menu.addSubMenu ("Quality", qualMenu);

    // FX Mode toggle
    menu.addItem ("FX Mode (Audio Input)", true, fxModeButton.getToggleState(),
        [this] {
            fxModeButton.setToggleState (! fxModeButton.getToggleState(), juce::sendNotificationSync);
        });

    menu.addSeparator();

    // MIDI Learn toggle
    menu.addItem ("MIDI Learn", true, midiLearnActive,
        [this] {
            midiLearnActive = ! midiLearnActive;
            processorRef.midiLearnActive = midiLearnActive;
            if (! midiLearnActive)
            {
                processorRef.stopMidiLearn();
                midiLearnCallout = nullptr;
            }
            else
            {
                showMidiLearnCallout();
            }
        });

    // Tooltips toggle
    menu.addItem ("Show Tooltips", true, tooltipsEnabled,
        [this] {
            tooltipsEnabled = ! tooltipsEnabled;
            tooltipWindow.setMillisecondsBeforeTipAppears (tooltipsEnabled ? 700 : 99999999);
        });

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (menuButton));
}

void PluginEditor::showMidiLearnCallout()
{
    auto* selector = new juce::ComboBox();
    selector->addItem ("(select parameter)", 1);
    auto& params = processorRef.getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(params[i]))
            selector->addItem (rp->getName (64), i + 2);
    }
    selector->setSize (250, 24);
    selector->addListener (this);

    midiLearnCallout = selector;

    juce::CallOutBox::launchAsynchronously (
        std::unique_ptr<juce::Component>(selector),
        menuButton.getScreenBounds(), nullptr);
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
        librarianPanel.refresh();
    }
}