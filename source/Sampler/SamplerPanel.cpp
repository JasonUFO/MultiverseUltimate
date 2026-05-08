#include "SamplerPanel.h"
#include "../MultiverseFlatTheme.h"
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

    // Root note
    for (int i = 0; i < 128; ++i)
    {
        rootNoteCombo.addItem (juce::MidiMessage::getMidiNoteName (i, true, true, 4), i + 1);
        loNoteCombo.addItem   (juce::MidiMessage::getMidiNoteName (i, true, true, 4), i + 1);
        hiNoteCombo.addItem   (juce::MidiMessage::getMidiNoteName (i, true, true, 4), i + 1);
    }
    rootNoteCombo.setSelectedId (61, juce::dontSendNotification); // C4 = 60 → id 61
    rootNoteCombo.onChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->rootNote = rootNoteCombo.getSelectedId() - 1;
    };
    addAndMakeVisible (rootNoteLabel);
    addAndMakeVisible (rootNoteCombo);

    // Tuning slider (±24 semitones, 0.1 resolution)
    tuningSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    tuningSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
    tuningSlider.setRange (-24.0, 24.0, 0.1);
    tuningSlider.setValue (0.0, juce::dontSendNotification);
    tuningSlider.setEnabled (false);
    tuningSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->tuning = (float) tuningSlider.getValue();
    };
    addAndMakeVisible (tuningLabel);
    addAndMakeVisible (tuningSlider);

    // Speed slider (0.25–4.0 multiplier)
    speedSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    speedSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
    speedSlider.setRange (0.25, 4.0, 0.01);
    speedSlider.setValue (1.0, juce::dontSendNotification);
    speedSlider.setEnabled (false);
    speedSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->speed = (float) speedSlider.getValue();
    };
    addAndMakeVisible (speedLabel);
    addAndMakeVisible (speedSlider);

    // Timestretch toggle
    tstrToggle.setToggleState (false, juce::dontSendNotification);
    tstrToggle.setEnabled (false);
    tstrToggle.setTooltip ("Timestretch: decouple pitch from speed (requires loop mode)");
    tstrToggle.setColour (juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary);
    tstrToggle.setColour (juce::ToggleButton::tickColourId, MultiverseFlatTheme::accentCyan);
    tstrToggle.onClick = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->timestretchEnabled = tstrToggle.getToggleState();
    };
    addAndMakeVisible (tstrToggle);

    // Lo/Hi key range combos
    loNoteCombo.setSelectedId (1, juce::dontSendNotification);
    loNoteCombo.onChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->loNote = loNoteCombo.getSelectedId() - 1;
    };
    hiNoteCombo.setSelectedId (128, juce::dontSendNotification);
    hiNoteCombo.onChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->hiNote = hiNoteCombo.getSelectedId() - 1;
    };
    addAndMakeVisible (loNoteLabel);
    addAndMakeVisible (loNoteCombo);
    addAndMakeVisible (hiNoteLabel);
    addAndMakeVisible (hiNoteCombo);

    // Lo/Hi velocity range sliders
    loVelSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    loVelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
    loVelSlider.setRange (0.0, 127.0, 1.0);
    loVelSlider.setValue (0.0, juce::dontSendNotification);
    loVelSlider.setEnabled (false);
    loVelSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->loVel = static_cast<int> (loVelSlider.getValue());
    };
    hiVelSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    hiVelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
    hiVelSlider.setRange (0.0, 127.0, 1.0);
    hiVelSlider.setValue (127.0, juce::dontSendNotification);
    hiVelSlider.setEnabled (false);
    hiVelSlider.onValueChange = [this]
    {
        if (selectedZoneIndex < 0 || selectedZoneIndex >= static_cast<int> (ownedZones.size())) return;
        ownedZones[static_cast<size_t> (selectedZoneIndex)]->hiVel = static_cast<int> (hiVelSlider.getValue());
    };
    addAndMakeVisible (loVelLabel);
    addAndMakeVisible (loVelSlider);
    addAndMakeVisible (hiVelLabel);
    addAndMakeVisible (hiVelSlider);

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

    // Auto Map button
    autoMapButton.onClick = [this] { autoMap(); };
    addAndMakeVisible (autoMapButton);

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

    // Tooltips
    clearButton.setTooltip       ("Remove all loaded sample zones");
    autoMapButton.setTooltip     ("Distribute zones evenly across the full key range (C-1 to G9)");
    rootNoteCombo.setTooltip     ("Root note: MIDI pitch at which the sample plays at its original speed");
    tuningSlider.setTooltip      ("Tune: pitch offset in semitones (±24) — with TSTR, changes pitch without affecting speed");
    speedSlider.setTooltip       ("Speed: playback rate multiplier (0.25×–4×) — with TSTR, changes speed without affecting pitch");
    loNoteCombo.setTooltip       ("Lo Key: lowest MIDI note this zone responds to");
    hiNoteCombo.setTooltip       ("Hi Key: highest MIDI note this zone responds to");
    loVelSlider.setTooltip       ("Lo Vel: minimum velocity (0–127) this zone responds to");
    hiVelSlider.setTooltip       ("Hi Vel: maximum velocity (0–127) this zone responds to");
    loopModeCombo.setTooltip     ("Loop mode: None (one-shot) / Forward / Ping-Pong");
    loopStartSlider.setTooltip   ("Loop start point (sample index). Drag to set where the loop begins.");
    loopEndSlider.setTooltip     ("Loop end point (sample index). Drag to set where the loop ends.");
    xfadeSlider.setTooltip       ("Loop crossfade length: smoothes the loop boundary to reduce clicks");
    samplerVolumeSlider.setTooltip("Sampler Volume: global output level for all zones (0–100%)");
    samplerPanSlider.setTooltip  ("Sampler Pan: global stereo position (−1=left, 0=centre, +1=right)");
    zoneList.setTooltip          ("Sample zones — drag audio files into this panel to load them");

    updateControlsForSelectedZone();
}

SamplerPanel::~SamplerPanel()
{
    zoneList.setModel (nullptr);
}

 void SamplerPanel::paint (juce::Graphics& g)
{
     g.fillAll (MultiverseFlatTheme::bgBase);

      // Title bar
       auto titleArea = getLocalBounds().removeFromTop (24);
       g.setColour (MultiverseFlatTheme::bgRaised.darker (0.2f));
       g.fillRect (titleArea);
       g.setColour (MultiverseFlatTheme::textPrimary);
       g.setFont (juce::Font (13.0f, juce::Font::bold));
       g.drawText ("SAMPLER", titleArea, juce::Justification::centred);

      // Draw neumorphic section cards
      const float cr = 8.0f;
      if (dropZoneBounds.getHeight() > 0)
      {
          MultiverseFlatTheme::drawCard (g, dropZoneBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::bgRaised);
          g.fillRoundedRectangle (dropZoneBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
          g.drawRoundedRectangle (dropZoneBounds.toFloat().reduced (0.5f), cr, 1.0f);
      }
      if (zoneListBounds.getHeight() > 0)
      {
          MultiverseFlatTheme::drawCard (g, zoneListBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::bgRaised);
          g.fillRoundedRectangle (zoneListBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
          g.drawRoundedRectangle (zoneListBounds.toFloat().reduced (0.5f), cr, 1.0f);
      }
      if (controlsBounds.getHeight() > 0)
      {
          MultiverseFlatTheme::drawCard (g, controlsBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::bgRaised);
          g.fillRoundedRectangle (controlsBounds.toFloat(), cr);
          g.setColour (MultiverseFlatTheme::borderLight.withAlpha (0.3f));
          g.drawRoundedRectangle (controlsBounds.toFloat().reduced (0.5f), cr, 1.0f);
      }

      // Drop zone (shown when empty or dragging) — drawn on top of card
      if (ownedZones.empty() || isDragOver)
      {
          auto dropArea = dropZoneBounds.reduced (8, 4);

          g.setColour (isDragOver ? MultiverseFlatTheme::accentBlue.withAlpha (0.3f) : MultiverseFlatTheme::bgRaised);
          g.fillRoundedRectangle (dropArea.toFloat(), 5.0f);
          g.setColour (isDragOver ? MultiverseFlatTheme::accentBlue.withAlpha (0.6f) : MultiverseFlatTheme::textMuted);
          g.drawRoundedRectangle (dropArea.toFloat(), 5.0f, 1.5f);
          g.setColour (isDragOver ? MultiverseFlatTheme::textPrimary : MultiverseFlatTheme::textMuted);
          g.setFont (12.0f);
          g.drawText (isDragOver ? "Release to load" : "Drop audio files here",
                      dropArea, juce::Justification::centred);
      }
}

void SamplerPanel::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (24); // title

    // Drop zone
    dropZoneBounds = area.removeFromTop (54);
    area.removeFromTop (4);

    // Zone list
    zoneListBounds = area.removeFromTop (110);
    zoneList.setBounds (zoneListBounds);
    area.removeFromTop (8);

    // Controls section
    controlsBounds = area;
    controlsBounds = controlsBounds.withHeight (controlsBounds.getHeight() - 26 - 6);

    auto row = [&] (juce::Component& lbl, juce::Component& ctrl)
    {
        auto r = area.removeFromTop (24);
        lbl.setBounds (r.removeFromLeft (54));
        ctrl.setBounds (r);
        area.removeFromTop (3);
    };

    row (samplerVolumeLabel, samplerVolumeSlider);
    row (samplerPanLabel,    samplerPanSlider);
    row (rootNoteLabel,      rootNoteCombo);
    row (tuningLabel,        tuningSlider);
    row (speedLabel,         speedSlider);
    tstrToggle.setBounds (area.removeFromTop (22).withTrimmedLeft (50));
    area.removeFromTop (3);
    row (loNoteLabel,        loNoteCombo);
    row (hiNoteLabel,        hiNoteCombo);
    row (loVelLabel,         loVelSlider);
    row (hiVelLabel,         hiVelSlider);
    row (loopModeLabel,      loopModeCombo);
    row (loopStartLabel,     loopStartSlider);
    row (loopEndLabel,       loopEndSlider);
    row (xfadeLabel,         xfadeSlider);

    area.removeFromTop (6);
    auto btnRow = area.removeFromTop (26);
    clearButton.setBounds   (btnRow.removeFromLeft (btnRow.getWidth() / 2).withTrimmedRight (2));
    autoMapButton.setBounds (btnRow.withTrimmedLeft (2));
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
    tuningSlider.setEnabled (valid);
    speedSlider.setEnabled (valid);
    tstrToggle.setEnabled (valid);
    loNoteCombo.setEnabled (valid);
    hiNoteCombo.setEnabled (valid);
    loVelSlider.setEnabled (valid);
    hiVelSlider.setEnabled (valid);
    loopModeCombo.setEnabled (valid);
    loopStartSlider.setEnabled (valid);
    loopEndSlider.setEnabled (valid);
    xfadeSlider.setEnabled (valid);

    if (!valid)
        return;

    const auto& zone = *ownedZones[static_cast<size_t> (selectedZoneIndex)];
    int total = zone.audioData.getNumSamples();

    rootNoteCombo.setSelectedId (zone.rootNote + 1,                          juce::dontSendNotification);
    tuningSlider.setValue       ((double) zone.tuning,                       juce::dontSendNotification);
    speedSlider.setValue        ((double) zone.speed,                        juce::dontSendNotification);
    tstrToggle.setToggleState   (zone.timestretchEnabled,                    juce::dontSendNotification);
    loNoteCombo.setSelectedId   (zone.loNote + 1,                            juce::dontSendNotification);
    hiNoteCombo.setSelectedId   (zone.hiNote + 1,                            juce::dontSendNotification);
    loVelSlider.setValue        ((double) zone.loVel,                        juce::dontSendNotification);
    hiVelSlider.setValue        ((double) zone.hiVel,                        juce::dontSendNotification);
    loopModeCombo.setSelectedId (static_cast<int> (zone.loopMode) + 1,      juce::dontSendNotification);

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
     g.fillAll (selected ? MultiverseFlatTheme::accentBlue.withAlpha (0.25f) : MultiverseFlatTheme::bgDeep);

     if (row < 0 || row >= static_cast<int> (panel.ownedZones.size()))
         return;

     const auto& zone = *panel.ownedZones[static_cast<size_t> (row)];
     g.setColour (MultiverseFlatTheme::textPrimary.withAlpha (0.85f));
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

// ---------------------------------------------------------------------------
// Auto Map
// ---------------------------------------------------------------------------

void SamplerPanel::autoMap()
{
    int n = static_cast<int> (ownedZones.size());
    if (n == 0) return;

    int notesPerZone = 128 / n;
    for (int i = 0; i < n; ++i)
    {
        ownedZones[i]->loNote = i * notesPerZone;
        ownedZones[i]->hiNote = (i == n - 1) ? 127 : (i + 1) * notesPerZone - 1;
    }
    updateZoneList();
    updateControlsForSelectedZone();
}
