#include "ArpeggiatorPanel.h"
#include "../MultiverseTheme.h"

// =============================================================================
// ArpeggiatorPanel
// =============================================================================

ArpeggiatorPanel::ArpeggiatorPanel (Arpeggiator& arp) : arpeggiator (arp)
{
    titleLabel.setText ("ARPEGGIATOR", juce::dontSendNotification);
    titleLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (titleLabel);

    enableBtn.setClickingTogglesState (true);
    enableBtn.setToggleState (arp.isEnabled(), juce::dontSendNotification);
    enableBtn.onClick = [this]() { arpeggiator.setEnabled (enableBtn.getToggleState()); };
    addAndMakeVisible (enableBtn);

    modeLabel.setText ("Mode:", juce::dontSendNotification);
    modeLabel.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (modeLabel);

    modeBox.addItem ("Up",      1);
    modeBox.addItem ("Down",    2);
    modeBox.addItem ("UpDown",  3);
    modeBox.addItem ("Random",  4);
    modeBox.addItem ("Chord",   5);
    modeBox.setSelectedId (1, juce::dontSendNotification);
    modeBox.onChange = [this]()
    {
        ArpMode m = ArpMode::Up;
        switch (modeBox.getSelectedId())
        {
            case 1: m = ArpMode::Up;      break;
            case 2: m = ArpMode::Down;    break;
            case 3: m = ArpMode::UpDown;  break;
            case 4: m = ArpMode::Random; break;
            case 5: m = ArpMode::Chord;   break;
        }
        arpeggiator.setMode (m);
    };
    addAndMakeVisible (modeBox);

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
            arpeggiator.setNumSteps (n);
    };
    addAndMakeVisible (numStepsBox);

    stepEditorTitle.setText ("Click a step to edit", juce::dontSendNotification);
    stepEditorTitle.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
    addAndMakeVisible (stepEditorTitle);

    auto setupSlider = [this](juce::Slider& sl, juce::Label& lbl, const juce::String& name,
                           double min, double max, double intervals)
    {
        lbl.setText (name, juce::dontSendNotification);
        lbl.setColour (juce::Label::textColourId, MultiverseTheme::textSecondary);
        sl.setSliderStyle (juce::Slider::LinearHorizontal);
        sl.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 50, 18);
        sl.setRange (min, max, intervals);
        addAndMakeVisible (lbl);
        addAndMakeVisible (sl);
    };

    setupSlider (noteOffsetSlider, noteOffsetLabel, "Note:", -12.0, 12.0, 1.0);
    noteOffsetSlider.setValue (0.0, juce::dontSendNotification);
    noteOffsetSlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        arpeggiator.getStep (selectedStep).noteOffset = static_cast<int> (noteOffsetSlider.getValue());
        refreshStepGrid();
    };

    setupSlider (octaveSlider, octaveLabel, "Oct:", -2.0, 2.0, 1.0);
    octaveSlider.setValue (0.0, juce::dontSendNotification);
    octaveSlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        arpeggiator.getStep (selectedStep).octave = static_cast<int> (octaveSlider.getValue());
    };

    setupSlider (velocitySlider, velocityLabel, "Vel:", 0.0, 1.0, 0.01);
    velocitySlider.setValue (0.8, juce::dontSendNotification);
    velocitySlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        arpeggiator.getStep (selectedStep).velocity = static_cast<float> (velocitySlider.getValue());
    };

    setupSlider (gateSlider, gateLabel, "Gate:", 0.01, 0.99, 0.01);
    gateSlider.setValue (0.5, juce::dontSendNotification);
    gateSlider.onValueChange = [this]()
    {
        if (selectedStep < 0) return;
        arpeggiator.getStep (selectedStep).gate = static_cast<float> (gateSlider.getValue());
    };

    tieBtn.setClickingTogglesState (true);
    tieBtn.onClick = [this]() { if (selectedStep >= 0) arpeggiator.getStep (selectedStep).tie = tieBtn.getToggleState(); };
    addAndMakeVisible (tieBtn);

    for (int s = 0; s < ARP_MAX_STEPS; ++s)
    {
        stepButtons[s].setActive (arpeggiator.getStep (s).active);
        stepButtons[s].onToggle = [this, s]()
        {
            arpeggiator.getStep (s).active = stepButtons[s].isActive();
            selectStep (s);
        };
        addAndMakeVisible (stepButtons[s]);
    }

    // Tooltips
    enableBtn.setTooltip          ("Enable or disable the arpeggiator");
    modeBox.setTooltip            ("Arp mode: Up / Down / UpDown / Random / Chord");
    numStepsBox.setTooltip        ("Total number of arp steps (8, 16, or 32)");
    noteOffsetSlider.setTooltip   ("Semitone offset for the selected step (−12 to +12)");
    octaveSlider.setTooltip       ("Octave shift for the selected step (−2 to +2)");
    velocitySlider.setTooltip     ("Velocity for the selected step (0–100%)");
    gateSlider.setTooltip         ("Gate length for the selected step (1–99% of step duration)");
    tieBtn.setTooltip             ("Tie: hold note over into the next step");

    refreshStepGrid();
}

ArpeggiatorPanel::~ArpeggiatorPanel()
{
}

// =============================================================================
// Layout
// =============================================================================

void ArpeggiatorPanel::paint (juce::Graphics& g)
{
    g.fillAll (MultiverseTheme::bgBase);

    if (selectedStep >= 0)
    {
        auto box = getLocalBounds().removeFromBottom (100).reduced (6, 4).toFloat();
        g.setColour (MultiverseTheme::bgDeep);
        g.fillRoundedRectangle (box, 4.0f);
        g.setColour (MultiverseTheme::shadowLight);
        g.drawRoundedRectangle (box, 4.0f, 1.0f);
    }
}

void ArpeggiatorPanel::resized()
{
    auto area = getLocalBounds().reduced (6);

    // ── Header row ───────────────────────────────────────────────────────────
    titleLabel.setBounds (area.removeFromTop (28).reduced (10, 0));

    auto ctrlRow = area.removeFromTop (24);
    enableBtn.setBounds (ctrlRow.removeFromLeft (70).reduced (0, 2));

    ctrlRow.removeFromLeft (16);
    modeLabel.setBounds (ctrlRow.removeFromLeft (44).reduced (0, 4));
    modeBox.setBounds   (ctrlRow.removeFromLeft (80).reduced (2, 2));

    ctrlRow.removeFromLeft (16);
    numStepsLabel.setBounds (ctrlRow.removeFromLeft (44).reduced (0, 4));
    numStepsBox.setBounds   (ctrlRow.removeFromLeft (70).reduced (2, 2));

    area.removeFromTop (8);

    // ── Step editor (bottom) ─────────────────────────────────────────────────
    auto editorArea  = area.removeFromBottom (100).reduced (0, 4);
    stepEditorTitle.setBounds (editorArea.removeFromTop (20));

    auto placeCtrl = [&](juce::Label& lbl, juce::Component& ctrl, int lw, int cw)
    {
        lbl.setBounds  (editorArea.removeFromLeft (lw).reduced (0, 4));
        ctrl.setBounds (editorArea.removeFromLeft (cw).reduced (2, 1));
        editorArea.removeFromLeft (8);
    };

    auto sliders = editorArea.removeFromTop (22);
    placeCtrl (noteOffsetLabel, noteOffsetSlider, 36, 90);
    placeCtrl (octaveLabel,  octaveSlider,     32, 70);
    placeCtrl (velocityLabel, velocitySlider, 32, 70);
    placeCtrl (gateLabel,    gateSlider,     36, 80);
    tieBtn.setBounds (editorArea.removeFromLeft (48).reduced (2, 1));

    area.removeFromTop (8);

    // ── Step grid (2 rows of 16) ────────���─��──────────────────────────────
    auto grid1 = area.removeFromTop (48);
    const int btnW = grid1.getWidth() / 16;
    for (int s = 0; s < 16; ++s)
        stepButtons[s].setBounds (grid1.removeFromLeft (btnW).reduced (1));

    area.removeFromTop (4);

    auto grid2 = area.removeFromTop (48);
    for (int s = 16; s < 32; ++s)
        stepButtons[s].setBounds (grid2.removeFromLeft (btnW).reduced (1));
}

// =============================================================================
// Private helpers
// =============================================================================

void ArpeggiatorPanel::selectStep (int step)
{
    selectedStep = step;
    refreshStepEditor();
    repaint();
}

void ArpeggiatorPanel::refreshStepEditor()
{
    if (selectedStep < 0)
    {
        stepEditorTitle.setText ("Click a step to edit", juce::dontSendNotification);
        return;
    }

    const auto& st = arpeggiator.getStep (selectedStep);
    stepEditorTitle.setText ("Step " + juce::String (selectedStep + 1), juce::dontSendNotification);
    noteOffsetSlider.setValue (st.noteOffset, juce::dontSendNotification);
    octaveSlider.setValue     (st.octave,     juce::dontSendNotification);
    velocitySlider.setValue   (st.velocity,   juce::dontSendNotification);
    gateSlider.setValue       (st.gate,       juce::dontSendNotification);
    tieBtn.setToggleState    (st.tie,       juce::dontSendNotification);
}

void ArpeggiatorPanel::refreshStepGrid()
{
    for (int s = 0; s < ARP_MAX_STEPS; ++s)
        stepButtons[s].setActive (arpeggiator.getStep (s).active);
}