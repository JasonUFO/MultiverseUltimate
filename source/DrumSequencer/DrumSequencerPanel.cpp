#include "DrumSequencerPanel.h"
#include "../CyberpunkTheme.h"
#include "../CyberpunkTheme.h"

DrumSequencerPanel::DrumSequencerPanel (DrumSequencer& seq)
    : sequencer (seq)
{
    chainQueue.fill (-1);

    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        for (int s = 0; s < DRUM_STEPS; ++s)
        {
            stepButtons[t][s].track = t;
            stepButtons[t][s].step = s;

            stepButtons[t][s].onLeftClick = [this, t, s]()
            {
                bool newState = !sequencer.getStepActive (t, s);
                sequencer.setStepActive (t, s, newState);
                stepButtons[t][s].setActive (newState);
                if (newState)
                    stepButtons[t][s].setVelocity (sequencer.getStepVelocity (t, s));
            };

            stepButtons[t][s].onRightClick = [this, t, s]()
            {
                showVelocityMenu (t, s);
            };

            addAndMakeVisible (stepButtons[t][s]);
        }

        trackRows.push_back (std::make_unique<TrackRow> (t, sequencer, *this));
        addAndMakeVisible (*trackRows.back());
    }

    bpmLabel.setText ("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    bpmLabel.setColour (juce::Label::textColourId, CyberpunkTheme::textSecondary);
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
        if (lastHighlightedStep >= 0 && lastHighlightedStep < DRUM_STEPS)
        {
            for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
                stepButtons[t][lastHighlightedStep].setHighlighted (false);
        }
        lastHighlightedStep = -1;
        positionLabel.setText ("Step: —", juce::dontSendNotification);
    };
    addAndMakeVisible (stopButton);

    positionLabel.setText ("Step: —", juce::dontSendNotification);
    positionLabel.setColour (juce::Label::textColourId, CyberpunkTheme::textSecondary);
    positionLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (positionLabel);

    copyPatternButton.onClick = [this]()
    {
        sequencer.copyCurrentPattern();
        pastePatternButton.setEnabled (true);
    };
    copyPatternButton.setTooltip ("Copy current pattern to clipboard");
    addAndMakeVisible (copyPatternButton);

    pastePatternButton.setEnabled (sequencer.hasPatternInClipboard());
    pastePatternButton.onClick = [this]()
    {
        sequencer.pasteToCurrentPattern();
        refreshStepDisplay();
    };
    pastePatternButton.setTooltip ("Paste copied pattern into current slot");
    addAndMakeVisible (pastePatternButton);

    // Swing
    swingLabel.setText ("Swing", juce::dontSendNotification);
    swingLabel.setColour (juce::Label::textColourId, CyberpunkTheme::textSecondary);
    swingLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (swingLabel);

    swingSlider.setRange (0.0, 100.0, 1.0);
    swingSlider.setValue (0.0, juce::dontSendNotification);
    swingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    swingSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 38, 20);
    swingSlider.setTextValueSuffix ("%");
    swingSlider.onValueChange = [this]()
    {
        sequencer.setSwing (static_cast<float> (swingSlider.getValue()) / 100.0f);
    };
    swingSlider.setTooltip ("Swing: delays odd steps to create a shuffle feel (0–100%)");
    addAndMakeVisible (swingSlider);

    // Quantization
    quantLabel.setText ("Quant", juce::dontSendNotification);
    quantLabel.setColour (juce::Label::textColourId, CyberpunkTheme::textSecondary);
    quantLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (quantLabel);

    quantCombo.addItem ("1/4",  1);
    quantCombo.addItem ("1/8",  2);
    quantCombo.addItem ("1/16", 3);
    quantCombo.addItem ("1/32", 4);
    quantCombo.setSelectedId (3, juce::dontSendNotification);
    quantCombo.onChange = [this]()
    {
        const float spbValues[] = { 1.0f, 2.0f, 4.0f, 8.0f };
        int id = quantCombo.getSelectedId() - 1;
        if (id >= 0 && id < 4)
            sequencer.setStepsPerBeat (spbValues[id]);
    };
    quantCombo.setTooltip ("Step resolution: 1/4 = quarter, 1/8 = eighth, 1/16 = sixteenth (default), 1/32 = thirty-second");
    addAndMakeVisible (quantCombo);

    // Pattern chain
    chainToggleButton.setClickingTogglesState (true);
    chainToggleButton.setColour (juce::TextButton::buttonOnColourId, CyberpunkTheme::accentGreen.darker (0.2f));
    chainToggleButton.onClick = [this]()
    {
        chainActive = chainToggleButton.getToggleState();
        chainPos = 0;
    };
    chainToggleButton.setTooltip ("Enable chain mode: auto-advance through the pattern queue on the right");
    addAndMakeVisible (chainToggleButton);

    for (int i = 0; i < MAX_DRUM_PATTERNS; ++i)
    {
        chainSlotButtons[i].setTooltip ("Chain slot " + juce::String (i + 1) + ": click to cycle through patterns (1–8) or clear (—)");
        chainSlotButtons[i].onClick = [this, i]()
        {
            int cur = chainQueue[i];
            chainQueue[i] = (cur >= MAX_DRUM_PATTERNS - 1) ? -1 : cur + 1;
            updateChainSlotButtons();
        };
        addAndMakeVisible (chainSlotButtons[i]);
    }
    updateChainSlotButtons();

    for (int i = 0; i < MAX_DRUM_PATTERNS; ++i)
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

    dropLabel.setText ("Drop samples here", juce::dontSendNotification);
    dropLabel.setJustificationType (juce::Justification::centred);
    dropLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));
    dropLabel.setVisible (false);
    addAndMakeVisible (dropLabel);

    // Tooltips
    bpmSlider.setTooltip   ("Tempo: beats per minute (40–240 BPM)");
    playButton.setTooltip  ("Start the drum sequencer");
    stopButton.setTooltip  ("Stop the drum sequencer and reset playhead");
    for (int i = 0; i < MAX_DRUM_PATTERNS; ++i)
        patternButtons[i].setTooltip ("Load drum pattern slot " + juce::String(i + 1));

    startTimer (50);
}

DrumSequencerPanel::~DrumSequencerPanel()
{
    stopTimer();
}

void DrumSequencerPanel::refreshStepDisplay()
{
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        for (int s = 0; s < DRUM_STEPS; ++s)
        {
            stepButtons[t][s].setActive (sequencer.getStepActive (t, s));
            stepButtons[t][s].setVelocity (sequencer.getStepVelocity (t, s));
        }
    }
}

void DrumSequencerPanel::updatePatternButtons()
{
    int current = sequencer.getCurrentPatternSlot();
    for (int i = 0; i < MAX_DRUM_PATTERNS; ++i)
        patternButtons[i].setToggleState (i == current, juce::dontSendNotification);
}

void DrumSequencerPanel::timerCallback()
{
    bpmSlider.setValue (sequencer.getBPM(), juce::dontSendNotification);

    if (!sequencer.isPlaying())
        return;

    int step = sequencer.getCurrentStep();
    int displayStep = (step - 1 + DRUM_STEPS) % DRUM_STEPS;

    positionLabel.setText ("Step: " + juce::String (displayStep + 1), juce::dontSendNotification);

    // Chain: detect pattern wrap (step goes from 15 back to 0)
    if (chainActive && displayStep == 0 && lastHighlightedStep == DRUM_STEPS - 1)
        advanceChain();

    if (displayStep != lastHighlightedStep)
    {
        if (lastHighlightedStep >= 0 && lastHighlightedStep < DRUM_STEPS)
        {
            for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
                stepButtons[t][lastHighlightedStep].setHighlighted (false);
        }
        for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
            stepButtons[t][displayStep].setHighlighted (true);
        lastHighlightedStep = displayStep;
    }
}

void DrumSequencerPanel::showVelocityMenu (int track, int step)
{
    juce::PopupMenu menu;
    menu.addItem (25, "25%");
    menu.addItem (50, "50%");
    menu.addItem (75, "75%");
    menu.addItem (100, "100%");
    menu.addItem ("Custom...", [this, track, step]()
    {
        float vel = sequencer.getStepVelocity (track, step);
        sequencer.setStepVelocity (track, step, vel);
    });

    menu.showMenuAsync (juce::PopupMenu::Options{}, [this, track, step] (int result)
    {
        if (result > 0)
        {
            float velocity = static_cast<float> (result) / 100.0f;
            sequencer.setStepVelocity (track, step, velocity);
            stepButtons[track][step].setVelocity (velocity);
        }
    });
}

bool DrumSequencerPanel::isInterestedInFileDrag (const juce::StringArray&)
{
    return true;
}

void DrumSequencerPanel::filesDropped (const juce::StringArray& files, int x, int y)
{
    isDragOver = false;
    dropLabel.setVisible (false);

    int rowHeight = getLocalBounds().getHeight() / (DRUM_TRACK_COUNT + 2);
    int track = (y - rowHeight) / rowHeight;

    for (const auto& filePath : files)
    {
        juce::File file (filePath);
        if (file.existsAsFile())
        {
            int targetTrack = track;
            if (targetTrack < 0) targetTrack = 0;
            if (targetTrack >= DRUM_TRACK_COUNT) targetTrack = DRUM_TRACK_COUNT - 1;

            sequencer.loadSample (targetTrack, file);
        }
    }
}

void DrumSequencerPanel::fileDragEnter (const juce::StringArray&, int, int)
{
    isDragOver = true;
    dropLabel.setVisible (true);
    repaint();
}

void DrumSequencerPanel::fileDragExit (const juce::StringArray&)
{
    isDragOver = false;
    dropLabel.setVisible (false);
    repaint();
}

void DrumSequencerPanel::paint (juce::Graphics& g)
{
    g.fillAll (CyberpunkTheme::bgBase);

     g.setColour (CyberpunkTheme::textSecondary);
     g.setFont (juce::Font (12.0f, juce::Font::bold));
     g.drawText ("DRUM SEQUENCER", getLocalBounds().removeFromTop (24), juce::Justification::centred);

    // Draw neumorphic section cards
    const float cr = 8.0f;
    if (transportBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect (g, transportBounds.toFloat(), cr, 3.0f);
        g.setColour (CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle (transportBounds.toFloat(), cr);
        g.setColour (CyberpunkTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (transportBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (swingBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect (g, swingBounds.toFloat(), cr, 3.0f);
        g.setColour (CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle (swingBounds.toFloat(), cr);
        g.setColour (CyberpunkTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (swingBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (patternBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect (g, patternBounds.toFloat(), cr, 3.0f);
        g.setColour (CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle (patternBounds.toFloat(), cr);
        g.setColour (CyberpunkTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (patternBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (gridBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect (g, gridBounds.toFloat(), cr, 3.0f);
        g.setColour (CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle (gridBounds.toFloat(), cr);
        g.setColour (CyberpunkTheme::shadowLight.withAlpha (0.3f));
        g.drawRoundedRectangle (gridBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }

    if (isDragOver)
    {
        g.setColour (CyberpunkTheme::accentBlue.withAlpha (0.25f));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (4), 4, 2);
    }
}

void DrumSequencerPanel::resized()
{
    auto area = getLocalBounds().reduced (6);

    area.removeFromTop (24);  // title

    // Row 1: BPM + transport + position + copy/paste (transport card)
    transportBounds = area.withHeight (28);
    auto row1 = area.removeFromTop (28);
    bpmLabel.setBounds (row1.removeFromLeft (36));
    bpmSlider.setBounds (row1.removeFromLeft (140));
    row1.removeFromLeft (6);
    playButton.setBounds (row1.removeFromLeft (50));
    row1.removeFromLeft (4);
    stopButton.setBounds (row1.removeFromLeft (50));
    row1.removeFromLeft (8);
    positionLabel.setBounds (row1.removeFromLeft (72));
    row1.removeFromLeft (8);
    copyPatternButton.setBounds (row1.removeFromLeft (50));
    row1.removeFromLeft (4);
    pastePatternButton.setBounds (row1.removeFromLeft (50));

    area.removeFromTop (4);

    // Row 2: Swing + Quant + Chain (swing card)
    swingBounds = area.withHeight (24);
    auto row2 = area.removeFromTop (24);
    swingLabel.setBounds (row2.removeFromLeft (42));
    swingSlider.setBounds (row2.removeFromLeft (130));
    row2.removeFromLeft (8);
    quantLabel.setBounds (row2.removeFromLeft (42));
    quantCombo.setBounds (row2.removeFromLeft (80));
    row2.removeFromLeft (8);
    chainToggleButton.setBounds (row2.removeFromLeft (54));
    row2.removeFromLeft (4);
    {
        int chainW = row2.getWidth() / MAX_DRUM_PATTERNS;
        for (auto& btn : chainSlotButtons)
            btn.setBounds (row2.removeFromLeft (chainW).reduced (1, 0));
    }

    area.removeFromTop (4);

    // Pattern row (pattern card)
    patternBounds = area.withHeight (22);
    auto patRow = area.removeFromTop (22);
    int patW = patRow.getWidth() / MAX_DRUM_PATTERNS;
    for (auto& btn : patternButtons)
        btn.setBounds (patRow.removeFromLeft (patW).reduced (1, 0));

    area.removeFromTop (4);

    int trackRowHeight = area.getHeight() / juce::jmax (1, DRUM_TRACK_COUNT);
    if (trackRowHeight < 18) trackRowHeight = 18;

    // Step grid card (covers track rows + step grid)
    gridBounds = area.withHeight (trackRowHeight * DRUM_TRACK_COUNT);

    auto stepArea = area.removeFromRight (area.getWidth() * 3 / 4);

    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        auto trackRowBounds = area.removeFromTop (trackRowHeight);
        if (t < static_cast<int> (trackRows.size()))
            trackRows[static_cast<size_t> (t)]->setBounds (trackRowBounds);

        auto stepRow = stepArea.removeFromTop (trackRowHeight);
        int sw = stepRow.getWidth() / DRUM_STEPS;
        for (int s = 0; s < DRUM_STEPS; ++s)
            stepButtons[t][s].setBounds (stepRow.removeFromLeft (sw).reduced (1, 1));
    }

    dropLabel.setBounds (getLocalBounds());
}

// ===== DrumStepButton =====

void DrumSequencerPanel::DrumStepButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);

    juce::Colour bg, border;
    if (highlighted)
    {
        bg = CyberpunkTheme::accentAmber;
        border = CyberpunkTheme::accentAmber.brighter (0.3f);
    }
    else if (active)
    {
        // Velocity intensity: dark blue (low) → bright cyan (high)
        float v = juce::jlimit (0.0f, 1.0f, velocity);
        bg     = CyberpunkTheme::accentBlue.darker (0.5f).interpolatedWith (CyberpunkTheme::accentBlue, v);
        border = CyberpunkTheme::accentBlue.darker (0.7f).interpolatedWith (CyberpunkTheme::accentBlue.brighter (0.3f), v);
    }
    else
    {
        bg = CyberpunkTheme::bgRaised;
        border = CyberpunkTheme::shadowLight;
    }

    g.setColour (bg);
    g.fillRoundedRectangle (bounds, 2.0f);
    g.setColour (border);
    g.drawRoundedRectangle (bounds, 2.0f, 1.0f);
}

void DrumSequencerPanel::DrumStepButton::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown())
    {
        if (onRightClick)
            onRightClick();
    }
    else
    {
        if (onLeftClick)
            onLeftClick();
    }
}

// ===== TrackRow =====

DrumSequencerPanel::TrackRow::TrackRow (int idx, DrumSequencer& seq, DrumSequencerPanel& p)
    : trackIndex (idx), sequencer (seq), panel (p)
{
    nameLabel.setText (sequencer.getTrackName (idx), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.8f));
    addAndMakeVisible (nameLabel);

    volumeSlider.setRange (0.0, 1.0, 0.01);
    volumeSlider.setValue (sequencer.getTrackVolume (idx), juce::dontSendNotification);
    volumeSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 30, 14);
    volumeSlider.onValueChange = [this]()
    {
        sequencer.setTrackVolume (trackIndex, static_cast<float> (volumeSlider.getValue()));
    };
    addAndMakeVisible (volumeSlider);

    muteButton.setClickingTogglesState (true);
    muteButton.setColour (juce::TextButton::buttonOnColourId, CyberpunkTheme::accentAmber.darker (0.3f));
    muteButton.onClick = [this]()
    {
        sequencer.setTrackMuted (trackIndex, muteButton.getToggleState());
    };
    addAndMakeVisible (muteButton);

    soloButton.setClickingTogglesState (true);
    soloButton.setColour (juce::TextButton::buttonOnColourId, CyberpunkTheme::accentAmber.darker (0.2f));
    soloButton.onClick = [this]()
    {
        sequencer.setTrackSolo (trackIndex, soloButton.getToggleState());
    };
    addAndMakeVisible (soloButton);

    loadButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff334466));
    loadButton.onClick = [this]()
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Load Sample for Track " + juce::String (trackIndex + 1),
            juce::File::getSpecialLocation (juce::File::userHomeDirectory),
            "*.wav;*.aiff;*.aif;*.flac;*.ogg;*.mp3");

        chooser->launchAsync (juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File{})
                {
                    sequencer.loadSample (trackIndex, file);
                    nameLabel.setText (sequencer.getTrackName (trackIndex),
                                       juce::dontSendNotification);
                }
            });
    };
    addAndMakeVisible (loadButton);

    levelMeter.setColour (juce::Label::backgroundColourId, CyberpunkTheme::bgDeep);
    levelMeter.setColour (juce::Label::textColourId, CyberpunkTheme::accentGreen);
    addAndMakeVisible (levelMeter);

    volumeSlider.setTooltip ("Track " + juce::String(idx + 1) + " volume (0–100%)");
    muteButton.setTooltip   ("Mute track " + juce::String(idx + 1) + " (silences this drum track)");
    soloButton.setTooltip   ("Solo track " + juce::String(idx + 1) + " (mutes all other tracks)");
    loadButton.setTooltip   ("Load an audio sample for track " + juce::String(idx + 1));
}

void DrumSequencerPanel::TrackRow::updateFromSequencer()
{
    nameLabel.setText (sequencer.getTrackName (trackIndex), juce::dontSendNotification);
    volumeSlider.setValue (sequencer.getTrackVolume (trackIndex), juce::dontSendNotification);
    muteButton.setToggleState (sequencer.getTrackMuted (trackIndex), juce::dontSendNotification);
    soloButton.setToggleState (sequencer.getTrackSolo (trackIndex), juce::dontSendNotification);

    float level = sequencer.getTrackLevel (trackIndex);
    int meterHeight = static_cast<int> (level * getHeight());
    levelMeter.setColour (juce::Label::backgroundColourId,
                          CyberpunkTheme::accentGreen.withAlpha (0.3f + level * 0.7f));
}

void DrumSequencerPanel::TrackRow::paint (juce::Graphics& g)
{
    g.setColour (CyberpunkTheme::bgRaised.darker (0.2f));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1), 2.0f);

    // Waveform preview in name label area when a sample is loaded
    const auto& buf = sequencer.getTrackSampleBuffer (trackIndex);
    if (buf.getNumSamples() >0)
    {
        auto waveArea = juce::Rectangle<int> (2, 2, 68, getHeight() - 4);
        g.setColour (CyberpunkTheme::bgDeep);
        g.fillRect (waveArea);

        const float* samples = buf.getReadPointer (0);
        int numSamples = buf.getNumSamples();
        int w = waveArea.getWidth();
        float cy = waveArea.getCentreY();
        float halfH = (waveArea.getHeight() - 2) *0.45f;

        g.setColour (CyberpunkTheme::accentBlue.withAlpha (0.85f));
        juce::Path path;
        for (int px = 0; px < w; ++px)
        {
            int idx = juce::jlimit (0, numSamples - 1,
                                    static_cast<int> ((float) px / w * numSamples));
            float s = juce::jlimit (-1.0f, 1.0f, samples[idx]);
            float y = cy - s * halfH;
            if (px == 0) path.startNewSubPath ((float) (waveArea.getX() + px), y);
            else         path.lineTo          ((float) (waveArea.getX() + px), y);
        }
        g.strokePath (path, juce::PathStrokeType (1.0f));
    }
}

void DrumSequencerPanel::TrackRow::resized()
{
    auto area = getLocalBounds().reduced (2);
    int nameW = 70;
    int volW = 60;
    int btnW = 22;

    nameLabel.setBounds (area.removeFromLeft (nameW));
    area.removeFromLeft (2);
    volumeSlider.setBounds (area.removeFromLeft (volW));
    area.removeFromLeft (2);
    muteButton.setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    soloButton.setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    loadButton.setBounds (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    levelMeter.setBounds (area.removeFromLeft (16));
}

// ===== Chain helpers =====

void DrumSequencerPanel::advanceChain()
{
    for (int i = 1; i <= MAX_DRUM_PATTERNS; ++i)
    {
        int nextSlot = (chainPos + i) % MAX_DRUM_PATTERNS;
        if (chainQueue[nextSlot] >= 0)
        {
            chainPos = nextSlot;
            int patIdx = chainQueue[nextSlot];
            sequencer.savePattern (sequencer.getCurrentPatternSlot());
            sequencer.loadPattern (patIdx);
            refreshStepDisplay();
            updatePatternButtons();
            return;
        }
    }
}

void DrumSequencerPanel::updateChainSlotButtons()
{
    for (int i = 0; i < MAX_DRUM_PATTERNS; ++i)
    {
        int pat = chainQueue[i];
        chainSlotButtons[i].setButtonText (pat < 0 ? juce::String (juce::CharPointer_UTF8 ("\xe2\x80\x94"))
                                                    : juce::String (pat + 1));
    }
}