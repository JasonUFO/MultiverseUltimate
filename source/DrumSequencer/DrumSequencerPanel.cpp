#include "DrumSequencerPanel.h"
#include "../MultiverseFlatTheme.h"

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
    positionLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
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
    swingLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
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
    quantLabel.setColour (juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
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
    chainToggleButton.setColour (juce::TextButton::buttonOnColourId, MultiverseFlatTheme::accentGreen.darker (0.2f));
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
    g.fillAll (MultiverseFlatTheme::bgBase);

     g.setColour (MultiverseFlatTheme::textSecondary);
     g.setFont (juce::Font (12.0f, juce::Font::bold));
     g.drawText ("DRUM SEQUENCER", getLocalBounds().removeFromTop (24), juce::Justification::centred);

    // Draw neumorphic section cards
    const float cr = 8.0f;
    if (transportBounds.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, transportBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (transportBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (transportBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (swingBounds.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, swingBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (swingBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (swingBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (patternBounds.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, patternBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (patternBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (patternBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }
    if (gridBounds.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, gridBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::bgRaised);
        g.fillRoundedRectangle (gridBounds.toFloat(), cr);
        g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (gridBounds.toFloat().reduced (0.5f), cr, 1.0f);
    }

    if (isDragOver)
    {
        g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (0.25f));
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
        bg = MultiverseFlatTheme::accentAmber;
        border = MultiverseFlatTheme::accentAmber.brighter (0.3f);
    }
    else if (active)
    {
        // Velocity intensity: dark blue (low) → bright cyan (high)
        float v = juce::jlimit (0.0f, 1.0f, velocity);
        bg     = MultiverseFlatTheme::accentBlue.darker (0.5f).interpolatedWith (MultiverseFlatTheme::accentBlue, v);
        border = MultiverseFlatTheme::accentBlue.darker (0.7f).interpolatedWith (MultiverseFlatTheme::accentBlue.brighter (0.3f), v);
    }
    else
    {
        bg = MultiverseFlatTheme::bgRaised;
        border = MultiverseFlatTheme::borderLight;
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
    muteButton.setColour (juce::TextButton::buttonOnColourId, MultiverseFlatTheme::accentAmber.darker (0.3f));
    muteButton.onClick = [this]()
    {
        sequencer.setTrackMuted (trackIndex, muteButton.getToggleState());
    };
    addAndMakeVisible (muteButton);

    soloButton.setClickingTogglesState (true);
    soloButton.setColour (juce::TextButton::buttonOnColourId, MultiverseFlatTheme::accentAmber.darker (0.2f));
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

    levelMeter.setColour (juce::Label::backgroundColourId, MultiverseFlatTheme::bgDeep);
    levelMeter.setColour (juce::Label::textColourId, MultiverseFlatTheme::accentGreen);
    addAndMakeVisible (levelMeter);

    fxButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff223355));
    fxButton.onClick = [this]() { panel.showTrackFXPopup (trackIndex); };
    addAndMakeVisible (fxButton);

    busCombo.addItem ("Main", 1);
    for (int b = 1; b <= DRUM_TRACK_COUNT; ++b)
        busCombo.addItem ("Out " + juce::String (b), b + 1);
    busCombo.setSelectedId (1, juce::dontSendNotification);
    busCombo.onChange = [this]()
    {
        int id = busCombo.getSelectedId();
        int bus = (id <= 1) ? 0 : (8 + id - 1); // Main=0, Out1=9, Out2=10, ...
        sequencer.setTrackOutputBus (trackIndex, bus);
    };
    addAndMakeVisible (busCombo);

    volumeSlider.setTooltip ("Track " + juce::String(idx + 1) + " volume (0–100%)");
    muteButton.setTooltip   ("Mute track " + juce::String(idx + 1) + " (silences this drum track)");
    soloButton.setTooltip   ("Solo track " + juce::String(idx + 1) + " (mutes all other tracks)");
    loadButton.setTooltip   ("Load an audio sample for track " + juce::String(idx + 1));
    fxButton.setTooltip     ("Per-track FX chain for track " + juce::String(idx + 1));
    busCombo.setTooltip     ("Output bus for track " + juce::String(idx + 1) + " (Main or individual Out 1-8)");
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
                          MultiverseFlatTheme::accentGreen.withAlpha (0.3f + level * 0.7f));
}

void DrumSequencerPanel::TrackRow::paint (juce::Graphics& g)
{
    g.setColour (MultiverseFlatTheme::bgRaised.darker (0.2f));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1), 2.0f);

    // Waveform preview in name label area when a sample is loaded
    const auto& buf = sequencer.getTrackSampleBuffer (trackIndex);
    if (buf.getNumSamples() >0)
    {
        auto waveArea = juce::Rectangle<int> (2, 2, 68, getHeight() - 4);
        g.setColour (MultiverseFlatTheme::bgDeep);
        g.fillRect (waveArea);

        const float* samples = buf.getReadPointer (0);
        int numSamples = buf.getNumSamples();
        int w = waveArea.getWidth();
        float cy = waveArea.getCentreY();
        float halfH = (waveArea.getHeight() - 2) *0.45f;

        g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (0.85f));
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
    int volW  = 56;
    int btnW  = 22;
    int fxW   = 28;
    int busW  = 58;

    nameLabel.setBounds    (area.removeFromLeft (nameW));
    area.removeFromLeft (2);
    volumeSlider.setBounds (area.removeFromLeft (volW));
    area.removeFromLeft (2);
    muteButton.setBounds   (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    soloButton.setBounds   (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    loadButton.setBounds   (area.removeFromLeft (btnW));
    area.removeFromLeft (2);
    fxButton.setBounds     (area.removeFromLeft (fxW));
    area.removeFromLeft (2);
    busCombo.setBounds     (area.removeFromLeft (busW));
    area.removeFromLeft (2);
    levelMeter.setBounds   (area.removeFromLeft (16));
}

void DrumSequencerPanel::showTrackFXPopup (int track)
{
    if (track < 0 || track >= DRUM_TRACK_COUNT) return;

    auto& chain = sequencer.getTrackFX (track);

    static const char* names[LayerEffectChain::NumEffects] = {
        "Chorus", "Distortion", "EQ", "Compressor", "Delay", "Reverb"
    };

    auto* content = new juce::Component();
    content->setSize (320, 200);

    for (int fx = 0; fx < LayerEffectChain::NumEffects; ++fx)
    {
        const int x = 8 + (fx % 3) * 104;
        const int y = 8 + (fx / 3) * 90;

        auto* label = new juce::Label();
        label->setText (names[fx], juce::dontSendNotification);
        label->setBounds (x, y, 90, 18);
        content->addAndMakeVisible (label);

        auto* toggle = new juce::ToggleButton ("On");
        toggle->setToggleState (chain.isEnabled (fx), juce::dontSendNotification);
        toggle->setBounds (x, y + 20, 70, 20);
        const int fxIdx = fx;
        toggle->onStateChange = [toggle, &chain, fxIdx]()
        {
            chain.setEnabled (fxIdx, toggle->getToggleState());
        };
        content->addAndMakeVisible (toggle);

        auto* mixSlider = new juce::Slider (juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight);
        mixSlider->setRange (0.0, 1.0);
        mixSlider->setValue (chain.getMix (fx), juce::dontSendNotification);
        mixSlider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 18);
        mixSlider->setBounds (x, y + 46, 96, 22);
        mixSlider->onValueChange = [mixSlider, &chain, fxIdx]()
        {
            chain.setMix (fxIdx, (float)mixSlider->getValue());
        };
        content->addAndMakeVisible (mixSlider);
    }

    auto& row = *trackRows[static_cast<size_t> (track)];
    juce::CallOutBox::launchAsynchronously (
        std::unique_ptr<juce::Component> (content), row.fxButton.getScreenBounds(), nullptr);
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