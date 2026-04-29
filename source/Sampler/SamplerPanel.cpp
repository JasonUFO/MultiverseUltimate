#include "SamplerPanel.h"
#include "../PluginProcessor.h"

SamplerPanel::SamplerPanel (PluginProcessor& p, SamplerEngine& engine)
    : samplerEngine (engine), zoneListModel (*this), processorRef (p)
{
    formatManager.registerBasicFormats();

    // Zone list
    zoneList.setModel (&zoneListModel);
    zoneList.setColour (juce::ListBox::backgroundColourId, juce::Colour (28, 28, 38));
    zoneList.setRowHeight (22);
    addAndMakeVisible (zoneList);

    // Clear button
    clearButton.onClick = [this]
    {
        samplerEngine.clearZones();
        ownedZones.clear();
        selectedZoneIndex = -1;
        updateZoneList();
        updateControlsForSelectedZone();
    };
    addAndMakeVisible (clearButton);

    // Loop mode
    loopModeCombo.addItem ("None",      1);
    loopModeCombo.addItem ("Forward",   2);
    loopModeCombo.addItem ("Ping-Pong", 3);
    loopModeCombo.setSelectedId (1, juce::dontSendNotification);
    loopModeCombo.onChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->loopMode =
            static_cast<LoopMode> (loopModeCombo.getSelectedId() - 1);
    };
    addAndMakeVisible (loopModeLabel);
    addAndMakeVisible (loopModeCombo);

    // Root note
    for (int i = 0; i < 128; ++i)
        rootNoteCombo.addItem (juce::MidiMessage::getMidiNoteName (i, true, true, 4), i + 1);
    rootNoteCombo.setSelectedId (61, juce::dontSendNotification); // C4 = 60 → id 61
    rootNoteCombo.onChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->rootNote = rootNoteCombo.getSelectedId() - 1;
    };
    addAndMakeVisible (rootNoteLabel);
    addAndMakeVisible (rootNoteCombo);

    // Loop sliders
    auto setupSlider = [this] (juce::Slider& s, juce::Label& l)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
        s.setEnabled (false);
        addAndMakeVisible (s);
        addAndMakeVisible (l);
    };

    setupSlider (loopStartSlider, loopStartLabel);
    loopStartSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->loopStart = static_cast<int> (loopStartSlider.getValue());
    };

    setupSlider (loopEndSlider, loopEndLabel);
    loopEndSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->loopEnd = static_cast<int> (loopEndSlider.getValue());
    };

    setupSlider (xfadeSlider, xfadeLabel);
    xfadeSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->crossfadeLength = static_cast<int> (xfadeSlider.getValue());
    };

    // Global sampler volume + pan (APVTS-backed, MIDI-learnable)
    auto setupGlobalSlider = [this] (MidiLearnSlider& s, juce::Label& l)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
        addAndMakeVisible (s);
        addAndMakeVisible (l);
    };

    setupGlobalSlider (samplerVolumeSlider, samplerVolumeLabel);
    samplerVolumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "samplerVolume", samplerVolumeSlider);
    samplerVolumeSlider.init (p, "samplerVolume");

    setupGlobalSlider (samplerPanSlider, samplerPanLabel);
    samplerPanAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        p.apvts, "samplerPan", samplerPanSlider);
    samplerPanSlider.init (p, "samplerPan");

    updateControlsForSelectedZone();
}

SamplerPanel::~SamplerPanel()
{
    zoneList.setModel (nullptr);
}

void SamplerPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (22, 22, 32));

    // Title bar
     auto titleArea = getLocalBounds().removeFromTop (24);
     g.setColour (juce::Colour (40, 40, 60));
     g.fillRect (titleArea);
     g.setColour (juce::Colours::white);
     g.setFont (juce::Font (13.0f, juce::Font::bold));
     g.drawText ("SAMPLER", titleArea, juce::Justification::centred);

    // Drop zone (shown when empty or dragging)
    if (ownedZones.empty() || isDragOver)
    {
        auto dropArea = getLocalBounds()
                            .removeFromTop (24 + 54)
                            .removeFromBottom (54)
                            .reduced (8, 4);

        g.setColour (isDragOver ? juce::Colour (60, 110, 180) : juce::Colour (38, 38, 55));
        g.fillRoundedRectangle (dropArea.toFloat(), 5.0f);
        g.setColour (juce::Colours::white.withAlpha (isDragOver ? 0.9f : 0.4f));
        g.drawRoundedRectangle (dropArea.toFloat(), 5.0f, 1.5f);
        g.setFont (12.0f);
        g.drawText (isDragOver ? "Release to load" : "Drop audio files here",
                    dropArea, juce::Justification::centred);
    }

    // Separator above controls
    int sepY = 24 + 54 + 4 + 110 + 4;
    g.setColour (juce::Colour (50, 50, 70));
    g.drawHorizontalLine (sepY, 8.0f, static_cast<float> (getWidth() - 8));
}

void SamplerPanel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (24); // title
    area.removeFromTop (54); // drop zone
    area.removeFromTop (4);

    zoneList.setBounds (area.removeFromTop (110));
    area.removeFromTop (8);

    auto row = [&] (juce::Component& lbl, juce::Component& ctrl)
    {
        auto r = area.removeFromTop (24);
        lbl.setBounds (r.removeFromLeft (54));
        ctrl.setBounds (r);
        area.removeFromTop (3);
    };

    row (samplerVolumeLabel, samplerVolumeSlider);
    row (samplerPanLabel,    samplerPanSlider);
    row (rootNoteLabel,  rootNoteCombo);
    row (loopModeLabel,  loopModeCombo);
    row (loopStartLabel, loopStartSlider);
    row (loopEndLabel,   loopEndSlider);
    row (xfadeLabel,     xfadeSlider);

    area.removeFromTop (6);
    clearButton.setBounds (area.removeFromTop (26));
}

bool SamplerPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        auto ext = juce::File (f).getFileExtension().toLowerCase();
        if (ext == ".wav" || ext == ".aiff" || ext == ".aif"
            || ext == ".flac" || ext == ".ogg" || ext == ".mp3")
            return true;
    }
    return false;
}

void SamplerPanel::filesDropped (const juce::StringArray& files, int, int)
{
    isDragOver = false;
    for (const auto& f : files)
        loadFile (juce::File (f));
    repaint();
}

void SamplerPanel::fileDragEnter (const juce::StringArray&, int, int)
{
    isDragOver = true;
    repaint();
}

void SamplerPanel::fileDragExit (const juce::StringArray&)
{
    isDragOver = false;
    repaint();
}

void SamplerPanel::loadFile (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return;

    auto zone = std::make_shared<SamplerZone>();
    zone->name          = file.getFileNameWithoutExtension();
    zone->fileSampleRate = reader->sampleRate;
    zone->loNote        = 0;
    zone->hiNote        = 127;
    zone->loVel         = 0;
    zone->hiVel         = 127;
    zone->rootNote      = 60;

    auto numSamples  = static_cast<int> (reader->lengthInSamples);
    auto numChannels = static_cast<int> (reader->numChannels);
    zone->audioData.setSize (numChannels, numSamples);
    reader->read (&zone->audioData, 0, numSamples, 0, true, true);

    zone->loopStart       = 0;
    zone->loopEnd         = numSamples;
    zone->crossfadeLength = 0;
    zone->loopMode        = LoopMode::None;

    ownedZones.push_back (zone);
    samplerEngine.addZone (zone);

    selectedZoneIndex = static_cast<int> (ownedZones.size()) - 1;
    updateZoneList();
    updateControlsForSelectedZone();
}

void SamplerPanel::updateZoneList()
{
    zoneList.updateContent();
    zoneList.repaint();
    repaint();
}

void SamplerPanel::updateControlsForSelectedZone()
{
    bool valid = selectedZoneIndex >= 0 && selectedZoneIndex < static_cast<int> (ownedZones.size());

    rootNoteCombo.setEnabled (valid);
    loopModeCombo.setEnabled (valid);
    loopStartSlider.setEnabled (valid);
    loopEndSlider.setEnabled (valid);
    xfadeSlider.setEnabled (valid);

    if (!valid)
        return;

    const auto& zone = *ownedZones[static_cast<size_t> (selectedZoneIndex)];
    int total = zone.audioData.getNumSamples();

    rootNoteCombo.setSelectedId (zone.rootNote + 1, juce::dontSendNotification);
    loopModeCombo.setSelectedId (static_cast<int> (zone.loopMode) + 1, juce::dontSendNotification);

    loopStartSlider.setRange (0.0, total - 1, 1.0);
    loopStartSlider.setValue (zone.loopStart, juce::dontSendNotification);

    loopEndSlider.setRange (1.0, total, 1.0);
    loopEndSlider.setValue (zone.loopEnd > 0 ? zone.loopEnd : total, juce::dontSendNotification);

    int xfadeMax = juce::jmin (total / 4, static_cast<int> (zone.fileSampleRate));
    xfadeSlider.setRange (0.0, xfadeMax, 1.0);
    xfadeSlider.setValue (zone.crossfadeLength, juce::dontSendNotification);
}

// ---------------------------------------------------------------------------
// ZoneListModel
// ---------------------------------------------------------------------------

int SamplerPanel::ZoneListModel::getNumRows()
{
    return static_cast<int> (panel.ownedZones.size());
}

void SamplerPanel::ZoneListModel::paintListBoxItem (
    int row, juce::Graphics& g, int w, int h, bool selected)
{
    g.fillAll (selected ? juce::Colour (55, 85, 130) : juce::Colour (28, 28, 38));

    if (row < 0 || row >= static_cast<int> (panel.ownedZones.size()))
        return;

    const auto& zone = *panel.ownedZones[static_cast<size_t> (row)];
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (11.5f);

    juce::String text = zone.name + "   ["
        + juce::String (zone.loNote) + "-" + juce::String (zone.hiNote) + "]"
        + "  vel " + juce::String (zone.loVel) + "-" + juce::String (zone.hiVel);
    g.drawText (text, 6, 0, w - 6, h, juce::Justification::centredLeft, true);
}

void SamplerPanel::ZoneListModel::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    panel.selectedZoneIndex = row;
    panel.updateControlsForSelectedZone();
}
