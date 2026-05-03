#include "ProSequencerPanel.h"
#include "../MultiverseTheme.h"

// =============================================================================
// ProStepButton
// =============================================================================

juce::String ProStepButton::noteName (int midi)
{
    static const char* names[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    return juce::String (names[midi % 12]) + juce::String (midi / 12 - 1);
}

void ProStepButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.5f);

    juce::Colour bg, border;
    if (highlighted)
    {
        bg     = MultiverseTheme::accentAmber;
        border = MultiverseTheme::accentAmber.brighter (0.3f);
    }
    else if (active)
    {
        bg     = MultiverseTheme::accentBlue.darker (0.2f);
        border = MultiverseTheme::accentBlue;
    }
    else
    {
        bg     = MultiverseTheme::bgRaised;
        border = MultiverseTheme::shadowLight;
    }

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    if (active || highlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.setFont (juce::Font (9.5f, juce::Font::bold));
        g.drawText (noteName (note),
                    getLocalBounds().reduced (2).removeFromTop (getHeight() - 7),
                    juce::Justification::centred);

        // Velocity bar at the bottom when not highlighted
        if (!highlighted && velocity >0.0f)
        {
            auto bar = bounds.removeFromBottom (5.0f).reduced (2.0f, 0.0f);
            g.setColour (MultiverseTheme::accentBlue.darker (0.3f));
            g.fillRoundedRectangle (bar, 2.0f);
            g.setColour (MultiverseTheme::accentBlue);
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * velocity), 2.0f);
        }
        }
    }

void ProStepButton::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (onRightClick) onRightClick();
    }
    else
    {
        active = !active;
        repaint();
        if (onToggle) onToggle();
    }
}

// =============================================================================
// ProSequencerPanel
// =============================================================================

ProSequencerPanel::ProSequencerPanel (ProSequencer& seq) : sequencer (seq)
{
    // Transport row
    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    bpmLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (bpmLabel);

    bpmSlider.setRange (40.0, 240.0, 0.5);
    bpmSlider.setValue (sequencer.getBPM(), juce::dontSendNotification);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 44, 20);
    bpmSlider.onValueChange = [this]() { sequencer.setBPM (static_cast<float> (bpmSlider.getValue())); };
    addAndMakeVisible (bpmSlider);

    playButton.onClick = [this]() { sequencer.start(); };
    addAndMakeVisible (playButton);

    stopButton.onClick = [this]()
    {
        sequencer.stop();
        if (lastHighlightedStep >= 0)
        {
            stepButtons[lastHighlightedStep].setHighlighted (false);
            lastHighlightedStep = -1;
        }
    };
    addAndMakeVisible (stopButton);

    exportButton.onClick = [this]() { exportMidi(); };
    addAndMakeVisible (exportButton);

    bpmSlider.setTooltip    ("Tempo: beats per minute (40–240 BPM)");
    playButton.setTooltip   ("Start the sequencer");
    stopButton.setTooltip   ("Stop the sequencer and reset playhead");
    exportButton.setTooltip ("Export all active lanes as a MIDI file");

    // Lane selector buttons
    const char* laneNames[] = { "Lane 1", "Lane 2", "Lane 3", "Lane 4" };
    for (int i = 0; i < PRO_SEQ_LANES; ++i)
    {
        laneButtons[i].setButtonText (laneNames[i]);
        laneButtons[i].setClickingTogglesState (true);
        laneButtons[i].onClick = [this, i]() { selectLane (i); };
        addAndMakeVisible (laneButtons[i]);
    }
    laneButtons[0].setToggleState (true, juce::dontSendNotification);

    // Play mode controls
    playModeLabel.setText ("Mode:", juce::dontSendNotification);
    playModeLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (playModeLabel);

    fwdBtn.setClickingTogglesState (true);
    fwdBtn.onClick = [this]() { applyPlayMode (ProPlayMode::Forward); updatePlayModeButtons(); };
    addAndMakeVisible (fwdBtn);

    revBtn.setClickingTogglesState (true);
    revBtn.onClick = [this]() { applyPlayMode (ProPlayMode::Reverse); updatePlayModeButtons(); };
    addAndMakeVisible (revBtn);

    rndBtn.setClickingTogglesState (true);
    rndBtn.onClick = [this]() { applyPlayMode (ProPlayMode::Random);  updatePlayModeButtons(); };
    addAndMakeVisible (rndBtn);

    updatePlayModeButtons();

    // Num steps
    numStepsLabel.setText ("Steps:", juce::dontSendNotification);
    numStepsLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (numStepsLabel);

    numStepsBox.addItem ("8",  1);
    numStepsBox.addItem ("16", 2);
    numStepsBox.addItem ("32", 3);
    numStepsBox.setSelectedId (2, juce::dontSendNotification);
    numStepsBox.onChange = [this]()
    {
        int n = 0;
        switch (numStepsBox.getSelectedId())
        {
            case 1: n = 8;  break;
            case 2: n = 16; break;
            case 3: n = 32; break;
        }
        if (n > 0)
            sequencer.getLane (selectedLane).numSteps = n;
    };
    addAndMakeVisible (numStepsBox);

    // Row labels
    row1Label.setText ("Steps 1 – 16", juce::dontSendNotification);
    row1Label.setColour (juce::Label::textColourId, MultiverseTheme::textMuted);
    row1Label.setFont (juce::Font (9.5f));
    addAndMakeVisible (row1Label);

    row2Label.setText ("Steps 17 – 32", juce::dontSendNotification);
    row2Label.setColour (juce::Label::textColourId, MultiverseTheme::textMuted);
    row2Label.setFont (juce::Font (9.5f));
    addAndMakeVisible (row2Label);

    // Step buttons
    for (int s = 0; s < PRO_SEQ_STEPS; ++s)
    {
        const auto& step = sequencer.getLane (0).steps[s];
        stepButtons[s].setActive   (step.active);
        stepButtons[s].setNote     (step.note);
        stepButtons[s].setVelocity (step.velocity);

        stepButtons[s].onToggle = [this, s]()
        {
            sequencer.getLane (selectedLane).steps[s].active = stepButtons[s].isActive();
            selectStep (s);
        };
        stepButtons[s].onRightClick = [this, s]()
        {
            selectStep (s);
            showNoteMenu (s);
        };

        addAndMakeVisible (stepButtons[s]);
    }

    // Step editor header
    stepEditorTitle.setText ("Click a step to edit", juce::dontSendNotification);
    stepEditorTitle.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (stepEditorTitle);

    // Note picker
    noteLabel.setText ("Note:", juce::dontSendNotification);
    noteLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (noteLabel);
    noteBtn.onClick = [this]() { if (selectedStep >= 0) showNoteMenu (selectedStep); };
    addAndMakeVisible (noteBtn);

    auto setupSlider = [this](juce::Slider& sl, juce::Label& lbl, const juce::String& name)
    {
        lbl.setText (name, juce::dontSendNotification);
        lbl.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
        sl.setSliderStyle (juce::Slider::LinearHorizontal);
        sl.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 40, 18);
        addAndMakeVisible (lbl);
        addAndMakeVisible (sl);
    };

    setupSlider (velocitySlider, velocityLabel, "Velocity:");
    velocitySlider.setRange (0.0, 1.0, 0.01);
    velocitySlider.setValue (0.8, juce::dontSendNotification);
    velocitySlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        auto& step = sequencer.getLane (selectedLane).steps[selectedStep];
        step.velocity = static_cast<float> (velocitySlider.getValue());
        stepButtons[selectedStep].setVelocity (step.velocity);
    };

    setupSlider (gateSlider, gateLabel, "Gate:");
    gateSlider.setRange (0.01, 0.99, 0.01);
    gateSlider.setValue (0.5, juce::dontSendNotification);
    gateSlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        sequencer.getLane (selectedLane).steps[selectedStep].gate =
            static_cast<float> (gateSlider.getValue());
    };

    setupSlider (probSlider, probLabel, "Prob:");
    probSlider.setRange (0.0, 1.0, 0.01);
    probSlider.setValue (1.0, juce::dontSendNotification);
    probSlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        sequencer.getLane (selectedLane).steps[selectedStep].probability =
            static_cast<float> (probSlider.getValue());
    };

    ratchetLabel.setText ("Ratchet:", juce::dontSendNotification);
    ratchetLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (ratchetLabel);

    for (int r = 1; r <= 8; ++r)
        ratchetBox.addItem (juce::String (r) + "×", r);
    ratchetBox.setSelectedId (1, juce::dontSendNotification);
    ratchetBox.onChange = [this]()
    {
        if (selectedStep < 0) return;
        sequencer.getLane (selectedLane).steps[selectedStep].ratchet = ratchetBox.getSelectedId();
    };
    addAndMakeVisible (ratchetBox);

    // Tooltips
    fwdBtn.setTooltip          ("Play steps in forward order (left to right)");
    revBtn.setTooltip          ("Play steps in reverse order (right to left)");
    rndBtn.setTooltip          ("Play steps in random order");
    numStepsBox.setTooltip     ("Number of active steps per lane (8, 16, or 32)");
    noteBtn.setTooltip         ("Set the MIDI note for the selected step");
    velocitySlider.setTooltip  ("Step velocity: note hit strength (0–100%)");
    gateSlider.setTooltip      ("Step gate: note duration as fraction of step length (1–99%)");
    probSlider.setTooltip      ("Step probability: chance the step will trigger (0–100%)");
    ratchetBox.setTooltip      ("Ratchet: number of triggers within one step (1–8×)");
    for (int i = 0; i < PRO_SEQ_LANES; ++i)
        laneButtons[i].setTooltip ("Select lane " + juce::String(i + 1) + " — each lane has independent step data");

    startTimer (50);
}

ProSequencerPanel::~ProSequencerPanel()
{
    stopTimer();
}

// =============================================================================
// Layout
// =============================================================================

void ProSequencerPanel::paint (juce::Graphics& g)
{
    g.fillAll (MultiverseTheme::bgBase);

    g.setColour (MultiverseTheme::textSecondary);
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("PRO SEQUENCER   4 LANES \xc3\x97 32 STEPS",
                getLocalBounds().removeFromTop (28).reduced (10, 0),
                juce::Justification::centredLeft);

    // Draw neumorphic section cards
    const float cr = 8.0f;
    if (transportBounds.getHeight() > 0)
    {
        MultiverseTheme::drawNeumorphicRect (g, transportBounds.toFloat(), cr, 3.0f);
        g.setColour (MultiverseTheme::bgRaised);
        g.fillRoundedRectangle (transportBounds.toFloat(), cr);
        g.setColour (MultiverseTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (transportBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (laneModeBounds.getHeight() > 0)
    {
        MultiverseTheme::drawNeumorphicRect (g, laneModeBounds.toFloat(), cr, 3.0f);
        g.setColour (MultiverseTheme::bgRaised);
        g.fillRoundedRectangle (laneModeBounds.toFloat(), cr);
        g.setColour (MultiverseTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (laneModeBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (stepGridBounds.getHeight() > 0)
    {
        MultiverseTheme::drawNeumorphicRect (g, stepGridBounds.toFloat(), cr, 3.0f);
        g.setColour (MultiverseTheme::bgRaised);
        g.fillRoundedRectangle (stepGridBounds.toFloat(), cr);
        g.setColour (MultiverseTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (stepGridBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (editorBounds.getHeight() > 0)
    {
        MultiverseTheme::drawNeumorphicRect (g, editorBounds.toFloat(), cr, 3.0f);
        g.setColour (MultiverseTheme::bgRaised);
        g.fillRoundedRectangle (editorBounds.toFloat(), cr);
        g.setColour (MultiverseTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (editorBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
}

void ProSequencerPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (28);  // title

    // ── Transport row ──────────────────────────────────────────────
    transportBounds = area.withHeight (26);
    auto transportRow = area.removeFromTop (26);
    bpmLabel.setBounds  (transportRow.removeFromLeft (30));
    bpmSlider.setBounds (transportRow.removeFromLeft (130));
    transportRow.removeFromLeft (6);
    playButton.setBounds   (transportRow.removeFromLeft (46).reduced (0, 1));
    transportRow.removeFromLeft (3);
    stopButton.setBounds   (transportRow.removeFromLeft (46).reduced (0, 1));
    transportRow.removeFromLeft (3);
    exportButton.setBounds (transportRow.removeFromLeft (100).reduced (0, 1));

    area.removeFromTop (4);

    // ── Lane selector + mode controls ─────────────────────────────
    laneModeBounds = area.withHeight (28);
    auto topRow = area.removeFromTop (28);
    for (int i = 0; i < PRO_SEQ_LANES; ++i)
        laneButtons[i].setBounds (topRow.removeFromLeft (82).reduced (2, 1));

    topRow.removeFromLeft (16);
    playModeLabel.setBounds (topRow.removeFromLeft (44).reduced (0, 5));
    fwdBtn.setBounds (topRow.removeFromLeft (46).reduced (2, 1));
    revBtn.setBounds (topRow.removeFromLeft (46).reduced (2, 1));
    rndBtn.setBounds (topRow.removeFromLeft (46).reduced (2, 1));
    topRow.removeFromLeft (20);
    numStepsLabel.setBounds (topRow.removeFromLeft (44).reduced (0, 5));
    numStepsBox.setBounds   (topRow.removeFromLeft (70).reduced (2, 2));

    area.removeFromTop (6);

    // ── Step editor (bottom) ───────────────────────────────────────
    editorBounds   = area.removeFromBottom (116).reduced (0, 4);
    auto editorArea   = editorBounds;
    auto editorInner  = editorArea.reduced (10, 6);

    stepEditorTitle.setBounds (editorInner.removeFromTop (20));
    editorInner.removeFromTop (4);

    auto ctrlRow = editorInner.removeFromTop (26);
    noteLabel.setBounds (ctrlRow.removeFromLeft (38).reduced (0, 4));
    noteBtn.setBounds   (ctrlRow.removeFromLeft (52).reduced (0, 2));
    ctrlRow.removeFromLeft (14);

    auto placeCtrl = [&](juce::Label& lbl, juce::Component& ctrl, int lw, int cw)
    {
        lbl.setBounds  (ctrlRow.removeFromLeft (lw).reduced (0, 5));
        ctrl.setBounds (ctrlRow.removeFromLeft (cw).reduced (2, 1));
        ctrlRow.removeFromLeft (8);
    };

    placeCtrl (velocityLabel, velocitySlider, 56, 170);
    placeCtrl (gateLabel,     gateSlider,     36, 170);
    placeCtrl (probLabel,     probSlider,     36, 170);
    ratchetLabel.setBounds (ctrlRow.removeFromLeft (54).reduced (0, 5));
    ratchetBox.setBounds   (ctrlRow.removeFromLeft (60).reduced (2, 2));

    area.removeFromBottom (4);

    // ── Step grid row 1 (steps 1-16) ──────────────────────────────
    row1Label.setBounds (area.removeFromTop (14).reduced (2, 0));
    auto row1 = area.removeFromTop (64);
    // Step grid card bounds (rows 1 + 2 together)
    stepGridBounds = stepGridBounds.withHeight (14 + 4 + 64 + 4 + 14 + 4 + 64);
    stepGridBounds = stepGridBounds.withPosition (row1Label.getBounds().getPosition());
    const int btnW = row1.getWidth() / 16;
    for (int s = 0; s < 16; ++s)
        stepButtons[s].setBounds (row1.removeFromLeft (btnW).reduced (1));

    area.removeFromTop (4);

    // ── Step grid row 2 (steps 17-32) ─────────────────────────────
    row2Label.setBounds (area.removeFromTop (14).reduced (2, 0));
    auto row2 = area.removeFromTop (64);
    for (int s = 16; s < 32; ++s)
        stepButtons[s].setBounds (row2.removeFromLeft (btnW).reduced (1));
}

// =============================================================================
// Timer — playhead indicator
// =============================================================================

void ProSequencerPanel::timerCallback()
{
    bpmSlider.setValue (sequencer.getBPM(), juce::dontSendNotification);

    if (!sequencer.isPlaying())
    {
        if (lastHighlightedStep >= 0)
        {
            stepButtons[lastHighlightedStep].setHighlighted (false);
            lastHighlightedStep = -1;
        }
        return;
    }

    // getCurrentStep returns the next step; display the one just played
    const int numSteps   = sequencer.getLane (selectedLane).numSteps;
    const int next       = sequencer.getCurrentStep (selectedLane);
    const int displayStep = (next - 1 + numSteps) % numSteps;

    if (displayStep != lastHighlightedStep)
    {
        if (lastHighlightedStep >= 0)
            stepButtons[lastHighlightedStep].setHighlighted (false);
        stepButtons[displayStep].setHighlighted (true);
        lastHighlightedStep = displayStep;
    }
}

// =============================================================================
// Private helpers
// =============================================================================

void ProSequencerPanel::selectLane (int lane)
{
    selectedLane        = lane;
    selectedStep        = -1;
    lastHighlightedStep = -1;

    for (int i = 0; i < PRO_SEQ_LANES; ++i)
        laneButtons[i].setToggleState (i == lane, juce::dontSendNotification);

    const int ns = sequencer.getLane (lane).numSteps;
    numStepsBox.setSelectedId (ns <= 8 ? 1 : ns <= 16 ? 2 : 3, juce::dontSendNotification);

    updatePlayModeButtons();
    refreshStepGrid();
    refreshStepEditor();
}

void ProSequencerPanel::selectStep (int step)
{
    selectedStep = step;
    refreshStepEditor();
    repaint();  // redraw step-editor background
}

void ProSequencerPanel::refreshStepGrid()
{
    const auto& lane = sequencer.getLane (selectedLane);
    for (int s = 0; s < PRO_SEQ_STEPS; ++s)
    {
        const auto& st = lane.steps[s];
        stepButtons[s].setActive   (st.active);
        stepButtons[s].setNote     (st.note);
        stepButtons[s].setVelocity (st.velocity);
    }
}

void ProSequencerPanel::refreshStepEditor()
{
    if (selectedStep < 0)
    {
        stepEditorTitle.setText ("Click a step to edit", juce::dontSendNotification);
        return;
    }

    const auto& st = sequencer.getLane (selectedLane).steps[selectedStep];
    stepEditorTitle.setText ("Lane " + juce::String (selectedLane + 1)
                                + "  \xe2\x80\x94  Step " + juce::String (selectedStep + 1),
                             juce::dontSendNotification);
    noteBtn.setButtonText     (ProStepButton::noteName (st.note));
    velocitySlider.setValue   (st.velocity,    juce::dontSendNotification);
    gateSlider.setValue       (st.gate,        juce::dontSendNotification);
    probSlider.setValue       (st.probability, juce::dontSendNotification);
    ratchetBox.setSelectedId  (juce::jmax (1, st.ratchet), juce::dontSendNotification);
}

void ProSequencerPanel::updatePlayModeButtons()
{
    const auto mode = sequencer.getLane (selectedLane).playMode;
    fwdBtn.setToggleState (mode == ProPlayMode::Forward, juce::dontSendNotification);
    revBtn.setToggleState (mode == ProPlayMode::Reverse, juce::dontSendNotification);
    rndBtn.setToggleState (mode == ProPlayMode::Random,  juce::dontSendNotification);
}

void ProSequencerPanel::applyPlayMode (ProPlayMode mode)
{
    sequencer.getLane (selectedLane).playMode = mode;
}

void ProSequencerPanel::showNoteMenu (int stepIdx)
{
    static const char* noteNames[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    juce::PopupMenu menu;

    for (int oct = 1; oct <= 7; ++oct)
    {
        juce::PopupMenu sub;
        for (int n = 0; n < 12; ++n)
        {
            int midi = (oct + 1) * 12 + n;
            if (midi > 127) break;
            sub.addItem (midi + 1, juce::String (noteNames[n]) + juce::String (oct));
        }
        menu.addSubMenu ("Oct " + juce::String (oct), sub);
    }

    menu.showMenuAsync (juce::PopupMenu::Options{}, [this, stepIdx](int result)
    {
        if (result <= 0) return;
        const int midi = result - 1;
        sequencer.getLane (selectedLane).steps[stepIdx].note = midi;
        stepButtons[stepIdx].setNote (midi);
        if (selectedStep == stepIdx)
            noteBtn.setButtonText (ProStepButton::noteName (midi));
    });
}

void ProSequencerPanel::exportMidi()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Export MIDI",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("proseq.mid"),
        "*.mid");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                auto midiFile = sequencer.exportMidi();
                auto stream   = file.createOutputStream();
                if (stream != nullptr)
                    midiFile.writeTo (*stream);
            }
        });
}
