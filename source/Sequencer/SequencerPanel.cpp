#include "SequencerPanel.h"

// ===== StepButton =====

juce::String StepButton::noteName (int midiNote)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int octave = midiNote / 12 - 1;
    return juce::String (names[midiNote % 12]) + juce::String (octave);
}

void StepButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.5f);

    juce::Colour bg, border;
    if (highlighted)
    {
        bg = juce::Colour (0xffcc5500);
        border = juce::Colour (0xffff8844);
    }
    else if (active)
    {
        bg = juce::Colour (0xff2a5faa);
        border = juce::Colour (0xff4488dd);
    }
    else
    {
        bg = juce::Colour (0xff1a1a2e);
        border = juce::Colour (0xff333355);
    }

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

     if (active || highlighted)
     {
         g.setColour (juce::Colours::white.withAlpha (0.9f));
         g.setFont (juce::Font (8.5f, juce::Font::bold));
         g.drawText (noteName (noteNumber), getLocalBounds(), juce::Justification::centred);
     }
}

void StepButton::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (onRightClick) onRightClick();
    }
    else
    {
        active = !active;
        repaint();
        if (onLeftClick) onLeftClick();
    }
}

// ===== SequencerPanel =====

SequencerPanel::SequencerPanel (Sequencer& seq) : sequencer (seq)
{
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        stepButtons[i].setNote (sequencer.getStep (i).noteNumber);
        stepButtons[i].setActive (sequencer.getStep (i).active);

        stepButtons[i].onLeftClick = [this, i]()
        {
            sequencer.setStepActive (i, stepButtons[i].isActive());
        };

        stepButtons[i].onRightClick = [this, i]() { showNoteMenu (i); };

        addAndMakeVisible (stepButtons[i]);
    }

    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    bpmLabel.setColour (juce::Label::textColourId, juce::Colour (0xff9999bb));
    addAndMakeVisible (bpmLabel);

    bpmSlider.setRange (40.0, 240.0, 0.5);
    bpmSlider.setValue (sequencer.getBPM(), juce::dontSendNotification);
    bpmSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    bpmSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 44, 20);
    bpmSlider.onValueChange = [this]()
    {
        sequencer.setBPM (static_cast<float> (bpmSlider.getValue()));
    };
    addAndMakeVisible (bpmSlider);

    playButton.onClick = [this]()
    {
        sequencer.start();
        updatePatternButtons();
    };
    addAndMakeVisible (playButton);

    stopButton.onClick = [this]()
    {
        sequencer.stop();
        if (lastHighlightedStep >= 0 && lastHighlightedStep < MAX_STEPS)
            stepButtons[lastHighlightedStep].setHighlighted (false);
        lastHighlightedStep = -1;
    };
    addAndMakeVisible (stopButton);

    modeButton.setClickingTogglesState (true);
    modeButton.onClick = [this]()
    {
        bool isArp = modeButton.getToggleState();
        sequencer.setMode (isArp ? SequencerMode::Arpeggiator : SequencerMode::Sequencer);
        modeButton.setButtonText (isArp ? "ARP" : "SEQ");
        sequencer.clearArpNotes();
    };
    addAndMakeVisible (modeButton);

    for (int i = 0; i < MAX_PATTERNS; ++i)
    {
        patternButtons[i].setButtonText (juce::String (i + 1));
        patternButtons[i].setClickingTogglesState (true);
        patternButtons[i].onClick = [this, i]()
        {
            sequencer.savePattern (sequencer.getCurrentPatternSlot());
            sequencer.loadPattern (i);
            refreshStepDisplay();
            updatePatternButtons();
        };
        addAndMakeVisible (patternButtons[i]);
    }
    updatePatternButtons();

    exportButton.onClick = [this]() { exportMidi(); };
    addAndMakeVisible (exportButton);

    // Tooltips
    bpmSlider.setTooltip     ("Tempo: beats per minute (40–240 BPM)");
    playButton.setTooltip    ("Start the melodic sequencer");
    stopButton.setTooltip    ("Stop the sequencer and reset to step 1");
    modeButton.setTooltip    ("Toggle between Step Sequencer mode (SEQ) and Arpeggiator mode (ARP)");
    exportButton.setTooltip  ("Export the current pattern as a standard MIDI file");
    for (int i = 0; i < MAX_PATTERNS; ++i)
        patternButtons[i].setTooltip ("Load pattern slot " + juce::String(i + 1));

    startTimer (50);
}

SequencerPanel::~SequencerPanel()
{
    stopTimer();
}

void SequencerPanel::refreshStepDisplay()
{
    for (int s = 0; s < MAX_STEPS; ++s)
    {
        auto step = sequencer.getStep (s);
        stepButtons[s].setActive (step.active);
        stepButtons[s].setNote (step.noteNumber);
    }
}

void SequencerPanel::updatePatternButtons()
{
    int current = sequencer.getCurrentPatternSlot();
    for (int i = 0; i < MAX_PATTERNS; ++i)
        patternButtons[i].setToggleState (i == current, juce::dontSendNotification);
}

void SequencerPanel::timerCallback()
{
    bpmSlider.setValue (sequencer.getBPM(), juce::dontSendNotification);

    if (!sequencer.isPlaying())
        return;

    int step = sequencer.getCurrentStep();
    // getCurrentStep points to the NEXT step to play; display the previous one
    int displayStep = (step - 1 + MAX_STEPS) % MAX_STEPS;

    if (displayStep != lastHighlightedStep)
    {
        if (lastHighlightedStep >= 0 && lastHighlightedStep < MAX_STEPS)
            stepButtons[lastHighlightedStep].setHighlighted (false);
        stepButtons[displayStep].setHighlighted (true);
        lastHighlightedStep = displayStep;
    }
}

void SequencerPanel::showNoteMenu (int stepIndex)
{
    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    juce::PopupMenu menu;
    for (int oct = 1; oct <= 7; ++oct)
    {
        juce::PopupMenu sub;
        for (int n = 0; n < 12; ++n)
        {
            int midiNote = (oct + 1) * 12 + n;
            if (midiNote > 127) break;
            sub.addItem (midiNote + 1, juce::String (noteNames[n]) + juce::String (oct));
        }
        menu.addSubMenu ("Oct " + juce::String (oct), sub);
    }

    menu.showMenuAsync (juce::PopupMenu::Options{}, [this, stepIndex] (int result)
    {
        if (result > 0)
        {
            int midiNote = result - 1;
            sequencer.setStepNote (stepIndex, midiNote);
            stepButtons[stepIndex].setNote (midiNote);
        }
    });
}

void SequencerPanel::exportMidi()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Export MIDI Pattern",
        juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("pattern.mid"),
        "*.mid");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File{})
            {
                auto midiFile = sequencer.exportMidi();
                auto stream = file.createOutputStream();
                if (stream != nullptr)
                    midiFile.writeTo (*stream);
            }
        });
}

void SequencerPanel::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff13132a));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

     g.setColour (juce::Colour (0xff6666aa));
     g.setFont (juce::Font (10.5f, juce::Font::bold));
     g.drawText ("SEQUENCER / ARPEGGIATOR", getLocalBounds().removeFromTop (20), juce::Justification::centred);
}

void SequencerPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (20); // title

    // Controls row
    auto controls = area.removeFromTop (26);
    bpmLabel.setBounds (controls.removeFromLeft (30));
    bpmSlider.setBounds (controls.removeFromLeft (130));
    controls.removeFromLeft (6);
    playButton.setBounds (controls.removeFromLeft (46));
    controls.removeFromLeft (3);
    stopButton.setBounds (controls.removeFromLeft (46));
    controls.removeFromLeft (3);
    modeButton.setBounds (controls.removeFromLeft (46));

    area.removeFromTop (4);

    // Pattern slots row
    auto patRow = area.removeFromTop (22);
    int patW = patRow.getWidth() / MAX_PATTERNS;
    for (auto& btn : patternButtons)
        btn.setBounds (patRow.removeFromLeft (patW).reduced (1, 0));

    area.removeFromTop (5);

    // Step grid
    auto stepRow = area.removeFromTop (58);
    int btnW = stepRow.getWidth() / MAX_STEPS;
    for (auto& btn : stepButtons)
        btn.setBounds (stepRow.removeFromLeft (btnW).reduced (1, 1));

    area.removeFromTop (5);

    // Export button
    auto exportRow = area.removeFromTop (26);
    exportButton.setBounds (exportRow.withSizeKeepingCentre (120, 22));
}
