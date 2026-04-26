#include "DrumSequencerPanel.h"

DrumSequencerPanel::DrumSequencerPanel (DrumSequencer& seq)
    : sequencer (seq)
{
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
        if (lastHighlightedStep >= 0 && lastHighlightedStep < DRUM_STEPS)
        {
            for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
                stepButtons[t][lastHighlightedStep].setHighlighted (false);
        }
        lastHighlightedStep = -1;
    };
    addAndMakeVisible (stopButton);

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
    g.setColour (juce::Colour (0xff1a1a2e));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

     g.setColour (juce::Colour (0xff6666aa));
     g.setFont (juce::Font (12.0f, juce::Font::bold));
     g.drawText ("DRUM SEQUENCER", getLocalBounds().removeFromTop (24), juce::Justification::centred);

    if (isDragOver)
    {
        g.setColour (juce::Colour (0x40ffffff));
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (4), 4, 2);
    }
}

void DrumSequencerPanel::resized()
{
    auto area = getLocalBounds().reduced (6);

    area.removeFromTop (24);

    auto controls = area.removeFromTop (28);
    bpmLabel.setBounds (controls.removeFromLeft (32));
    bpmSlider.setBounds (controls.removeFromLeft (140));
    controls.removeFromLeft (8);
    playButton.setBounds (controls.removeFromLeft (50));
    controls.removeFromLeft (4);
    stopButton.setBounds (controls.removeFromLeft (50));

    area.removeFromTop (4);

    auto patRow = area.removeFromTop (22);
    int patW = patRow.getWidth() / MAX_DRUM_PATTERNS;
    for (auto& btn : patternButtons)
        btn.setBounds (patRow.removeFromLeft (patW).reduced (1, 0));

    area.removeFromTop (4);

    int trackRowHeight = area.getHeight() / juce::jmax (1, DRUM_TRACK_COUNT);
    if (trackRowHeight < 18) trackRowHeight = 18;

    // Split area: left quarter for track controls, right three-quarters for step buttons
    auto stepArea = area.removeFromRight (area.getWidth() * 3 / 4);
    int stepWidth = stepArea.getWidth() / DRUM_STEPS;

    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        // Track control row (name, volume, mute, solo, load)
        auto trackRowBounds = area.removeFromTop (trackRowHeight);
        if (t < static_cast<int> (trackRows.size()))
            trackRows[static_cast<size_t> (t)]->setBounds (trackRowBounds);

        // Step button row aligned with the track control row
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
        bg = juce::Colour (0xffff8844);
        border = juce::Colour (0xffffaa66);
    }
    else if (active)
    {
        bg = juce::Colour (0xff2a5faa);
        border = juce::Colour (0xff4488dd);
    }
    else
    {
        bg = juce::Colour (0xff252540);
        border = juce::Colour (0xff404060);
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
    muteButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffaa3333));
    muteButton.onClick = [this]()
    {
        sequencer.setTrackMuted (trackIndex, muteButton.getToggleState());
    };
    addAndMakeVisible (muteButton);

    soloButton.setClickingTogglesState (true);
    soloButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffaa8833));
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

    levelMeter.setColour (juce::Label::backgroundColourId, juce::Colour (0xff1a1a2e));
    levelMeter.setColour (juce::Label::textColourId, juce::Colour (0xff44aa44));
    addAndMakeVisible (levelMeter);
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
                          juce::Colour (0xff44aa44).withAlpha (0.3f + level * 0.7f));
}

void DrumSequencerPanel::TrackRow::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff1f1f35));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (1), 2.0f);
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