#include "PluginEditor.h"
#include "Assets/AssetManager.h"

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
      globalPanel (p),
      tabBar (*this),
      librarianPanel (p),
      presetOverlay (librarianPanel),
      modBar (p),
      keyboardStrip (p)
{
    setLookAndFeel (&mvTheme);

    // Load UI assets
    AssetManager::instance().initialise();

    // Register for skin changes
    SkinManager::instance().addListener(this);
    SkinManager::instance().addSkinChangeCallback([this]()
    {
        mvTheme.applySkinColours();
        repaint();
    });

    // Panel array
    panels[0]  = &synthPanel;
    panels[1]  = &effectsPanel;
    panels[2]  = &modulationMatrixPanel;
    panels[3]  = &globalPanel;
    panels[4]  = &drumSequencerPanel;
    panels[5]  = &granularPanel;
    panels[6]  = &samplerPanel;
    panels[7]  = &layersPanel;
    panels[8]  = &proSequencerPanel;
    panels[9]  = &arpeggiatorPanel;
    panels[10] = &routingPanel;

    for (auto* panel : panels)
        addChildComponent (panel);

    addAndMakeVisible (contentViewport);
    contentViewport.setScrollBarsShown (true, false);
    contentViewport.setScrollBarThickness (10);
    contentViewport.setLookAndFeel (&mvTheme);

    // Two-tier tab bar
    juce::Array<TwoTierTabBar::TabDef> tabDefs;
    tabDefs.add ({ "OSC",    0, true  });
    tabDefs.add ({ "FX",     1, true  });
    tabDefs.add ({ "MOD",    2, true  });
    tabDefs.add ({ "GLOBAL", 3, true  });
    tabDefs.add ({ "DRM",   4, false });
    tabDefs.add ({ "GRN",   5, false });
    tabDefs.add ({ "SMP",   6, false });
    tabDefs.add ({ "LYR",   7, false });
    tabDefs.add ({ "SEQ",   8, false });
    tabDefs.add ({ "ARP",   9, false });
    tabDefs.add ({ "ROU",  10, false });
    tabBar.setTabs (tabDefs);
    tabBar.setActivePanel (0);
    addAndMakeVisible (tabBar);

    activePanelIndex = 0;
    panels[0]->setVisible (true);
    contentViewport.setViewedComponent (panels[0], false);

    // Preset navigation
    prevPresetButton.addListener (this);
    nextPresetButton.addListener (this);
    favoriteButton.addListener (this);
    backButton.addListener (this);
    forwardButton.addListener (this);

    presetNameButton.setButtonText ("Init");
    presetNameButton.setTooltip ("Click to browse presets (Cmd+F)");
    presetNameButton.setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep());
    presetNameButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    presetNameButton.setColour (juce::TextButton::textColourOnId, MultiverseFlatTheme::accent1());
    presetNameButton.setColour (juce::TextButton::buttonOnColourId, MultiverseFlatTheme::bgDeep());
    presetNameButton.onClick = [this] { showPresetOverlay(); };
    addAndMakeVisible (presetNameButton);

    for (auto* btn : { &prevPresetButton, &nextPresetButton, &favoriteButton, &backButton, &forwardButton })
    {
        btn->setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep());
        btn->setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textSecondary());
        addAndMakeVisible (btn);
    }

    updatePresetNameLabel();
    updateFavoriteButtonColor();

    // Menu button
    menuButton.setTooltip ("Menu — Save, Import, Export, Scale, Quality, FX Mode, MIDI Learn, Skins");
    menuButton.setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep());
    menuButton.setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textPrimary());
    menuButton.addListener (this);
    addAndMakeVisible (menuButton);

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
    scaleCombo.setColour (juce::ComboBox::backgroundColourId, MultiverseFlatTheme::bgDeep());
    scaleCombo.setColour (juce::ComboBox::textColourId, MultiverseFlatTheme::textSecondary());
    scaleCombo.setColour (juce::ComboBox::outlineColourId, MultiverseFlatTheme::borderLight());
    scaleCombo.setColour (juce::ComboBox::arrowColourId, MultiverseFlatTheme::textMuted());
    scaleCombo.addListener (this);
    addAndMakeVisible (scaleCombo);

    // Quality combo
    qualCombo.addItem ("Off",      1);
    qualCombo.addItem ("2x",  2);
    qualCombo.addItem ("4x", 3);
    qualAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        p.apvts, "globalQuality", qualCombo);
    qualCombo.setColour (juce::ComboBox::backgroundColourId, MultiverseFlatTheme::bgDeep());
    qualCombo.setColour (juce::ComboBox::textColourId, MultiverseFlatTheme::textSecondary());
    qualCombo.setColour (juce::ComboBox::outlineColourId, MultiverseFlatTheme::borderLight());
    qualCombo.setColour (juce::ComboBox::arrowColourId, MultiverseFlatTheme::textMuted());
    addAndMakeVisible (qualCombo);

    // FX Mode toggle
    fxModeButton.setClickingTogglesState (true);
    fxModeButton.setColour (juce::TextButton::buttonColourId, MultiverseFlatTheme::bgDeep());
    fxModeButton.setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textMuted());
    fxModeButton.setColour (juce::TextButton::buttonOnColourId, MultiverseFlatTheme::accent1().withAlpha (0.3f));
    fxModeButton.setColour (juce::TextButton::textColourOnId, MultiverseFlatTheme::accent1());
    fxModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.apvts, "fxModeEnabled", fxModeButton);
    addAndMakeVisible (fxModeButton);

    // Mod bar (no KEY tab)
    addAndMakeVisible (modBar);

    // Keyboard strip — always visible at bottom
    addAndMakeVisible (keyboardStrip);

    // Preset overlay
    addChildComponent (presetOverlay);

    // Mod-drag overlay
    addAndMakeVisible (modDragOverlay);
    modDragOverlay.setInterceptsMouseClicks (false, false);

    // Routing panel tab-switch callback
    routingPanel.onSwitchToTab = [this] (int index)
    {
        switchToPanel (index);
    };

    setResizable (true, true);
    setResizeLimits (800, 533, 1920, 1280);
    setSize (1200, 800);
}

PluginEditor::~PluginEditor()
{
    SkinManager::instance().removeListener(this);
    setLookAndFeel (nullptr);
}

void PluginEditor::switchToPanel (int panelIndex)
{
    auto* currentViewed = contentViewport.getViewedComponent();
    if (currentViewed != nullptr)
    {
        contentViewport.setViewedComponent (nullptr, false);
        addChildComponent (currentViewed);
        currentViewed->setVisible (false);
    }

    activePanelIndex = panelIndex;

    panels[panelIndex]->setVisible (true);
    contentViewport.setViewedComponent (panels[panelIndex], false);

    tabBar.setActivePanel (panelIndex);
    resized();
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (MultiverseFlatTheme::bgVoid());

    // Draw image-based header background
    constexpr int headerH = MultiverseFlatTheme::Metrics::headerH;
    MultiverseFlatTheme::drawHeaderBackground (g, getLocalBounds().toFloat().removeFromTop (headerH));
}

void PluginEditor::resized()
{
    const Skin& s = MultiverseFlatTheme::skin();
    auto area = getLocalBounds();

    // Header: 36px across full width
    constexpr int headerH = MultiverseFlatTheme::Metrics::headerH;
    auto header = area.removeFromTop (headerH);
    menuButton.setBounds (header.removeFromLeft (36).reduced (4, 4));

    // Right side: Scale, Quality, FX Mode, RAND
    randomizeButton.setBounds (header.removeFromRight (52).reduced (4, 4));
    fxModeButton.setBounds (header.removeFromRight (36).reduced (4, 4));
    qualCombo.setBounds (header.removeFromRight (52).reduced (2, 4));
    scaleCombo.setBounds (header.removeFromRight (52).reduced (2, 4));

    // Center: preset navigation strip
    auto center = header.withSize (320, headerH).withCentre (header.getCentre());
    backButton.setBounds      (center.removeFromLeft (28).reduced (2, 4));
    prevPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    presetNameButton.setBounds(center.removeFromLeft (180).reduced (4, 4));
    nextPresetButton.setBounds(center.removeFromLeft (28).reduced (2, 4));
    favoriteButton.setBounds  (center.removeFromLeft (28).reduced (2, 4));
    forwardButton.setBounds   (center.removeFromLeft (28).reduced (2, 4));

    // Tab bar: now taller (64px)
    constexpr int tabBarH = MultiverseFlatTheme::Metrics::tabBarH;
    auto tabBarArea = area.removeFromTop (tabBarH);
    tabBar.setBounds (tabBarArea);

    // Keyboard strip: always visible at the bottom
    const int kbStripH = KeyboardStrip::STRIP_H;
    auto kbArea = area.removeFromBottom (kbStripH);
    keyboardStrip.setBounds (kbArea);

    // Mod bar: above keyboard strip
    auto bottomArea = area.removeFromBottom (ModBar::MOD_BAR_H);
    modBar.setBounds (bottomArea);

    // Content area: viewport fills remaining space
    contentViewport.setBounds (area);

    auto* activePanel = panels[activePanelIndex];
    const int viewportH = area.getHeight();
    const int panelW = area.getWidth();
    const int panelH = (activePanelIndex == 0) ? juce::jmax (viewportH, 960) : viewportH;
    activePanel->setSize (panelW, panelH);

    if (contentViewport.getViewedComponent() != activePanel)
        contentViewport.setViewedComponent (activePanel, false);

    presetOverlay.setBounds (getLocalBounds());
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
    if (combo == &scaleCombo)
    {
        const float f[] = { 0.75f, 1.0f, 1.25f, 1.5f };
        const int id = scaleCombo.getSelectedId();
        if (id >= 1 && id <= 4)
            setSize (juce::roundToInt (1200 * f[id - 1]),
                     juce::roundToInt (800  * f[id - 1]));
        return;
    }

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
    if (key == juce::KeyPress ('f', juce::ModifierKeys::commandModifier, 0))
    {
        showPresetOverlay();
        return true;
    }
    if (key == juce::KeyPress::escapeKey && presetOverlay.isVisible())
    {
        hidePresetOverlay();
        return true;
    }
    return false;
}

void PluginEditor::showMainMenu()
{
    juce::PopupMenu menu;

    menu.addItem ("Save Preset", [this] { librarianPanel.saveCurrentPreset(); });
    menu.addItem ("Import Preset", [this] { librarianPanel.importPreset(); });
    menu.addItem ("Export Preset", [this] { librarianPanel.exportPreset(); });

    menu.addSeparator();

    // Skin selector submenu
    juce::PopupMenu skinMenu;
    auto& sm = SkinManager::instance();
    for (int i = 0; i < sm.numSkins(); ++i)
    {
        const bool isSelected = (sm.getSkinIndex() == i);
        skinMenu.addItem (sm.skinName(i), true, isSelected, [i] {
            SkinManager::instance().setSkin(i);
        });
    }
    menu.addSubMenu ("Skins", skinMenu);

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

        if (skipList.contains (id)) continue;
        if (id.startsWith ("lfo") && id.endsWith ("Sync")) continue;
        if (id.startsWith ("lfo") && id.endsWith ("SyncDiv")) continue;

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
    presetNameButton.setButtonText (name);
}

void PluginEditor::updateFavoriteButtonColor()
{
    auto& pm = processorRef.getPresetManager();
    juce::String currentName = pm.getCurrentPreset().getName();

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
    favoriteButton.setColour (juce::TextButton::textColourOffId, MultiverseFlatTheme::textSecondary());
    favoriteButton.setColour (juce::TextButton::textColourOnId, MultiverseFlatTheme::textSecondary());
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
        int newColor = (currentColor + 1) % PresetManager::NUM_FAV_COLORS;
        if (currentColor < 0)
            newColor = 0;
        else if (currentColor >= PresetManager::NUM_FAV_COLORS - 1)
            newColor = -1;
        pm.setFavorite (idx, newColor);
        updateFavoriteButtonColor();
        librarianPanel.refresh();
    }
}

void PluginEditor::showPresetOverlay()
{
    presetOverlay.showOverlay();
}

void PluginEditor::hidePresetOverlay()
{
    presetOverlay.hideOverlay();
}