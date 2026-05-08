#include "SequencerPanel.h"
#include "../MultiverseFlatTheme.h"

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
        bg = MultiverseFlatTheme::accentAmber;
        border = MultiverseFlatTheme::accentAmber.brighter (0.3f);
    }
    else if (active)
    {
        bg = MultiverseFlatTheme::accentBlue.darker (0.2f);
        border = MultiverseFlatTheme::accentBlue;
    }
    else
    {
        bg = MultiverseFlatTheme::bgRaised;
        border = MultiverseFlatTheme::borderLight;
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

    // Probability indicator: small coloured dot at bottom-right when < 100%
    if (probability < 0.99f)
    {
        juce::Colour dotCol = probability < 0.3f ? juce::Colours::red.withAlpha (0.9f)
                            : probability < 0.6f ? MultiverseFlatTheme::accentAmber.withAlpha (0.9f)
                                                 : MultiverseFlatTheme::accentBlue.withAlpha (0.9f);
        g.setColour (dotCol);
        g.fillEllipse (bounds.getRight() - 6.0f, bounds.getBottom() - 6.0f, 5.0f, 5.0f);
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

// ===== Chord detection =====

static juce::String detectChordFromClasses (const std::vector<int>& pcs)
{
    if (pcs.size() < 2)
        return {};

    static const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    // Interval patterns relative to root (sorted pitch classes, 12 rotations)
    struct ChordDef { std::vector<int> intervals; const char* suffix; };
    static const ChordDef defs[] =
    {
        { {0, 4, 7},        ""      },  // major
        { {0, 3, 7},        "m"     },  // minor
        { {0, 4, 7, 11},    "maj7"  },  // major 7
        { {0, 4, 7, 10},    "7"     },  // dominant 7
        { {0, 3, 7, 10},    "m7"    },  // minor 7
        { {0, 3, 6},        "dim"   },  // diminished
        { {0, 4, 8},        "aug"   },  // augmented
        { {0, 3, 6, 10},    "m7b5"  },  // half-dim
        { {0, 5, 7},        "sus4"  },  // sus4
        { {0, 2, 7},        "sus2"  },  // sus2
    };

    // Try each note as root
    for (int root = 0; root < 12; ++root)
    {
        // Build interval set relative to this root
        std::vector<int> intervals;
        for (int pc : pcs)
        {
            int interval = (pc - root + 12) % 12;
            if (std::find (intervals.begin(), intervals.end(), interval) == intervals.end())
                intervals.push_back (interval);
        }
        std::sort (intervals.begin(), intervals.end());

        for (const auto& def : defs)
        {
            bool match = true;
            for (int iv : def.intervals)
                if (std::find (intervals.begin(), intervals.end(), iv) == intervals.end()) { match = false; break; }
            if (match)
                return juce::String (noteNames[root]) + def.suffix;
        }
    }
    return {};
}

// ===== DragMidiButton =====

void DragMidiButton::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    bool hovered = isMouseOver();

    g.setColour (hovered ? MultiverseFlatTheme::bgRaised.brighter (0.15f) : MultiverseFlatTheme::bgRaised);
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (hovered ? 0.9f : 0.5f));
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

    g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (hovered ? 1.0f : 0.7f));
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText ("^ DRAG", getLocalBounds(), juce::Justification::centred);
}

void DragMidiButton::mouseDrag (const juce::MouseEvent& e)
{
    if (dragStarted || e.getDistanceFromDragStart() < 6 || !getMidiFile)
        return;

    dragStarted = true;
    auto file = getMidiFile();
    if (file != juce::File{})
        juce::DragAndDropContainer::performExternalDragDropOfFiles (
            { file.getFullPathName() }, false, this,
            [this]() { dragStarted = false; });
    else
        dragStarted = false;
}

// ===== SequencerPanel =====

SequencerPanel::SequencerPanel (Sequencer& seq) : sequencer (seq)
{
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        auto step = sequencer.getStep (i);
        stepButtons[i].setNote (step.noteNumber);
        stepButtons[i].setActive (step.active);
        stepButtons[i].setProbability (step.probability);

        stepButtons[i].onLeftClick = [this, i]()
        {
            sequencer.setStepActive (i, stepButtons[i].isActive());
        };

        stepButtons[i].onRightClick = [this, i]() { showNoteMenu (i); };

        addAndMakeVisible (stepButtons[i]);
    }

    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    bpmLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
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

    // Step length combo
    stepLengthLabel.setText ("STEP", juce::dontSendNotification);
    stepLengthLabel.setJustificationType (juce::Justification::centredRight);
    stepLengthLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    addAndMakeVisible (stepLengthLabel);

    stepLengthCombo.addItem ("32nd",  1);
    stepLengthCombo.addItem ("16th",  2);
    stepLengthCombo.addItem ("8th",   3);
    stepLengthCombo.addItem ("Qtr",   4);
    stepLengthCombo.addItem ("8T",    5);
    stepLengthCombo.setSelectedId (2, juce::dontSendNotification); // default: 16th
    stepLengthCombo.onChange = [this]()
    {
        float mult = 1.0f;
        switch (stepLengthCombo.getSelectedId())
        {
            case 1: mult = 0.5f;        break; // 32nd
            case 2: mult = 1.0f;        break; // 16th
            case 3: mult = 2.0f;        break; // 8th
            case 4: mult = 4.0f;        break; // quarter
            case 5: mult = 4.0f / 3.0f; break; // 8th triplet
            default: break;
        }
        sequencer.setStepLengthMultiplier (mult);
    };
    stepLengthCombo.setTooltip ("Step length: subdivison of each sequencer step");
    addAndMakeVisible (stepLengthCombo);

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
            // Sync step-length combo to loaded pattern
            float m = sequencer.getStepLengthMultiplier();
            if      (m <= 0.6f)  stepLengthCombo.setSelectedId (1, juce::dontSendNotification);
            else if (m <= 1.1f)  stepLengthCombo.setSelectedId (2, juce::dontSendNotification);
            else if (m <= 2.1f)  stepLengthCombo.setSelectedId (3, juce::dontSendNotification);
            else if (m >= 3.9f)  stepLengthCombo.setSelectedId (4, juce::dontSendNotification);
            else                 stepLengthCombo.setSelectedId (5, juce::dontSendNotification);
        };
        addAndMakeVisible (patternButtons[i]);
    }
    updatePatternButtons();

    exportButton.onClick = [this]() { exportMidi(); };
    addAndMakeVisible (exportButton);

    dragMidiButton.getMidiFile = [this]() -> juce::File
    {
        auto temp = juce::File::getSpecialLocation (juce::File::tempDirectory)
                        .getChildFile ("multiverse_seq_drag.mid");
        auto midiFile = sequencer.exportMidi();
        auto stream = temp.createOutputStream();
        if (stream != nullptr && midiFile.writeTo (*stream))
            return temp;
        return {};
    };
    dragMidiButton.setTooltip ("Drag to drop the current pattern as a MIDI clip into your DAW");
    addAndMakeVisible (dragMidiButton);

    // Chord label
    chordLabel.setJustificationType (juce::Justification::centred);
    chordLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentBlue);
    addAndMakeVisible (chordLabel);

    // Tooltips
    bpmSlider.setTooltip     ("Tempo: beats per minute (40–240 BPM)");
    playButton.setTooltip    ("Start the melodic sequencer");
    stopButton.setTooltip    ("Stop the sequencer and reset to step 1");
    modeButton.setTooltip    ("Toggle between Step Sequencer mode (SEQ) and Arpeggiator mode (ARP)");
    exportButton.setTooltip  ("Export the current pattern as a standard MIDI file");
    for (int i = 0; i < MAX_PATTERNS; ++i)
        patternButtons[i].setTooltip ("Load pattern slot " + juce::String (i + 1));

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
        stepButtons[s].setProbability (step.probability);
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

    // Update chord label from active step notes
    juce::String chord = detectChord();
    chordLabel.setText (chord.isEmpty() ? "" : chord, juce::dontSendNotification);

    if (!sequencer.isPlaying())
        return;

    int step = sequencer.getCurrentStep();
    int displayStep = (step - 1 + MAX_STEPS) % MAX_STEPS;

    if (displayStep != lastHighlightedStep)
    {
        if (lastHighlightedStep >= 0 && lastHighlightedStep < MAX_STEPS)
            stepButtons[lastHighlightedStep].setHighlighted (false);
        stepButtons[displayStep].setHighlighted (true);
        lastHighlightedStep = displayStep;
    }
}

juce::String SequencerPanel::detectChord() const
{
    std::vector<int> pcs;
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        auto step = sequencer.getStep (i);
        if (step.active)
        {
            int pc = step.noteNumber % 12;
            if (std::find (pcs.begin(), pcs.end(), pc) == pcs.end())
                pcs.push_back (pc);
        }
    }
    return detectChordFromClasses (pcs);
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

    menu.addSeparator();

    // Probability sub-menu (IDs 1000-1004)
    juce::PopupMenu probMenu;
    float curProb = sequencer.getStep (stepIndex).probability;
    probMenu.addItem (1001, "100%", true, curProb >= 0.99f);
    probMenu.addItem (1002,  "75%", true, curProb >= 0.74f && curProb < 0.99f);
    probMenu.addItem (1003,  "50%", true, curProb >= 0.49f && curProb < 0.74f);
    probMenu.addItem (1004,  "25%", true, curProb >= 0.24f && curProb < 0.49f);
    probMenu.addItem (1005,   "0%", true, curProb < 0.01f);
    menu.addSubMenu ("Probability", probMenu);

    menu.showMenuAsync (juce::PopupMenu::Options{}, [this, stepIndex] (int result)
    {
        if (result > 0 && result <= 128)
        {
            int midiNote = result - 1;
            sequencer.setStepNote (stepIndex, midiNote);
            stepButtons[stepIndex].setNote (midiNote);
        }
        else if (result >= 1001 && result <= 1005)
        {
            static const float probValues[] = { 1.0f, 0.75f, 0.5f, 0.25f, 0.0f };
            float p = probValues[result - 1001];
            sequencer.setStepProbability (stepIndex, p);
            stepButtons[stepIndex].setProbability (p);
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

// ===== MIDI drag-and-drop =====

bool SequencerPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        juce::String lower = f.toLowerCase();
        if (lower.endsWith (".mid") || lower.endsWith (".midi"))
            return true;
    }
    return false;
}

void SequencerPanel::fileDragEnter (const juce::StringArray&, int, int)
{
    dragOver = true;
    repaint();
}

void SequencerPanel::fileDragExit (const juce::StringArray&)
{
    dragOver = false;
    repaint();
}

void SequencerPanel::filesDropped (const juce::StringArray& files, int, int)
{
    dragOver = false;
    repaint();

    for (const auto& path : files)
    {
        juce::File f (path);
        juce::String lower = path.toLowerCase();
        if (lower.endsWith (".mid") || lower.endsWith (".midi"))
        {
            importMidiFile (f);
            break;
        }
    }
}

void SequencerPanel::importMidiFile (const juce::File& file)
{
    juce::FileInputStream stream (file);
    if (!stream.openedOk())
        return;

    juce::MidiFile midiFile;
    if (!midiFile.readFrom (stream))
        return;

    midiFile.convertTimestampTicksToSeconds();

    // Find usable track: prefer first track with note events
    const juce::MidiMessageSequence* track = nullptr;
    for (int t = 0; t < midiFile.getNumTracks(); ++t)
    {
        auto* candidate = midiFile.getTrack (t);
        for (int e = 0; e < candidate->getNumEvents(); ++e)
        {
            if (candidate->getEventPointer (e)->message.isNoteOn())
            {
                track = candidate;
                break;
            }
        }
        if (track != nullptr) break;
    }

    if (track == nullptr)
        return;

    // Find the total time span of note-on events
    double maxTime = 0.0;
    for (int e = 0; e < track->getNumEvents(); ++e)
    {
        const auto& msg = track->getEventPointer (e)->message;
        if (msg.isNoteOn())
            maxTime = juce::jmax (maxTime, msg.getTimeStamp());
    }
    if (maxTime <= 0.0) maxTime = 1.0;

    // Clear steps, then quantize note-ons onto 16-step grid
    for (int s = 0; s < MAX_STEPS; ++s)
    {
        sequencer.setStepActive (s, false);
        sequencer.setStepNote (s, 60);
        sequencer.setStepVelocity (s, 0.8f);
    }

    int numSteps = sequencer.getNumSteps();
    for (int e = 0; e < track->getNumEvents(); ++e)
    {
        const auto& msg = track->getEventPointer (e)->message;
        if (!msg.isNoteOn()) continue;

        int slot = static_cast<int> (msg.getTimeStamp() / maxTime * numSteps + 0.5);
        slot = juce::jlimit (0, numSteps - 1, slot);

        if (!sequencer.getStep (slot).active)
        {
            sequencer.setStepActive (slot, true);
            sequencer.setStepNote (slot, msg.getNoteNumber());
            sequencer.setStepVelocity (slot, msg.getVelocity() / 127.0f);
        }
    }

    refreshStepDisplay();
}

// ===== Paint & layout =====

void SequencerPanel::paint (juce::Graphics& g)
{
    g.fillAll (MultiverseFlatTheme::bgBase);

    const float cr = 8.0f;
    auto drawCard = [&] (juce::Rectangle<int> bounds)
    {
        if (bounds.getHeight() <= 0) return;
        MultiverseFlatTheme::drawCard (g, bounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (bounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), cr, 1.0f);
    };

    drawCard (transportBounds);
    drawCard (patternBounds);

    // Step grid card — cyan border glow when drag is active
    if (stepGridBounds.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, stepGridBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (stepGridBounds.toFloat(), cr);
        if (dragOver)
        {
            g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (0.6f));
            g.drawRoundedRectangle (stepGridBounds.toFloat().reduced (0.5f), cr, 2.0f);
        }
        else
        {
            g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
            g.drawRoundedRectangle (stepGridBounds.toFloat().reduced (0.5f), cr, 1.0f);
        }
    }

    drawCard (exportBounds);

    g.setColour (MultiverseFlatTheme::textSecondary);
    g.setFont (juce::Font (10.5f, juce::Font::bold));
    g.drawText ("SEQUENCER / ARPEGGIATOR", getLocalBounds().removeFromTop (20), juce::Justification::centred);
}

void SequencerPanel::resized()
{
    auto area = getLocalBounds().reduced (6);
    area.removeFromTop (20); // title

    // Transport card
    transportBounds = area.withHeight (26);
    auto controls = area.removeFromTop (26);
    bpmLabel.setBounds (controls.removeFromLeft (30));
    bpmSlider.setBounds (controls.removeFromLeft (120));
    controls.removeFromLeft (4);
    playButton.setBounds (controls.removeFromLeft (42));
    controls.removeFromLeft (3);
    stopButton.setBounds (controls.removeFromLeft (42));
    controls.removeFromLeft (3);
    modeButton.setBounds (controls.removeFromLeft (42));
    controls.removeFromLeft (8);
    stepLengthLabel.setBounds (controls.removeFromLeft (32));
    stepLengthCombo.setBounds (controls.removeFromLeft (60));

    area.removeFromTop (4);

    // Pattern slots row
    patternBounds = area.withHeight (22);
    auto patRow = area.removeFromTop (22);
    int patW = patRow.getWidth() / MAX_PATTERNS;
    for (auto& btn : patternButtons)
        btn.setBounds (patRow.removeFromLeft (patW).reduced (1, 0));

    area.removeFromTop (5);

    // Step grid
    stepGridBounds = area.withHeight (58);
    auto stepRow = area.removeFromTop (58);
    int btnW = stepRow.getWidth() / MAX_STEPS;
    for (auto& btn : stepButtons)
        btn.setBounds (stepRow.removeFromLeft (btnW).reduced (1, 1));

    area.removeFromTop (5);

    // Export/info row
    exportBounds = area.withHeight (26);
    auto exportRow = area.removeFromTop (26);
    exportButton.setBounds (exportRow.removeFromLeft (110).withSizeKeepingCentre (100, 22));
    exportRow.removeFromLeft (6);
    dragMidiButton.setBounds (exportRow.removeFromLeft (90).withSizeKeepingCentre (84, 22));
    exportRow.removeFromLeft (6);
    chordLabel.setBounds (exportRow);
}
