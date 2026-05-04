#include "EffectsPanel.h"
#include "../PluginProcessor.h"
#include "../CyberpunkTheme.h"

//==============================================================================
// EffectChainStrip

const char* EffectChainStrip::effectName(int id)
{
    switch (id)
    {
        case 0: return "CHORUS";
        case 1: return "DIST";
        case 2: return "EQ";
        case 3: return "COMP";
        case 4: return "DELAY";
        case 5: return "REVERB";
        default: return "?";
    }
}

EffectChainStrip::EffectChainStrip(PluginProcessor& p) : proc(p)
{
    setMouseCursor(juce::MouseCursor::DraggingHandCursor);
}

juce::Rectangle<int> EffectChainStrip::tileRect(int i) const
{
    const int labelH = 15;
    const int tileW  = getWidth() / 6;
    return { i * tileW + 2, labelH, tileW - 4, getHeight() - labelH - 2 };
}

int EffectChainStrip::slotAt(int x) const
{
    const int tileW = juce::jmax(1, getWidth() / 6);
    return juce::jlimit(0, 5, x / tileW);
}

void EffectChainStrip::paint(juce::Graphics& g)
{
    g.fillAll(CyberpunkTheme::bgBase);

    g.setColour(CyberpunkTheme::textSecondary.withAlpha(0.7f));
    g.setFont(juce::Font(9.5f));
    g.drawText("EFFECT CHAIN  \xe2\x80\x94  drag tiles to reorder",
               getLocalBounds().removeFromTop(15).reduced(6, 0),
               juce::Justification::centredLeft);

    for (int i = 0; i < 6; ++i)
    {
        const int id = proc.getChainSlot(i);
        const auto tile = tileRect(i);

        const bool isSource = dragging && (i == dragSource);
        const bool isTarget = dragging && (i == dragOver) && (i != dragSource);

        g.setColour(isTarget  ? CyberpunkTheme::accentAmber.withAlpha(0.85f)
                   : isSource ? CyberpunkTheme::bgRaised.darker(0.3f)
                              : CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(tile.toFloat(), 5.f);

        g.setColour(juce::Colours::white.withAlpha(isSource ? 0.45f : 0.90f));
        g.setFont(juce::Font(10.f, juce::Font::bold));
        g.drawFittedText(effectName(id), tile, juce::Justification::centred, 1);

        // Step number
        g.setColour(CyberpunkTheme::textMuted);
        g.setFont(juce::Font(7.5f));
        g.drawText(juce::String(i + 1), tile.withTrimmedBottom(tile.getHeight() - 10).translated(3, 1),
                   juce::Justification::topLeft);
    }
}

void EffectChainStrip::mouseDown(const juce::MouseEvent& e)
{
    dragSource = slotAt(e.x);
    dragOver   = dragSource;
    dragging   = true;
    repaint();
}

void EffectChainStrip::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragging) return;
    const int over = slotAt(e.x);
    if (over != dragOver)
    {
        dragOver = over;
        repaint();
    }
}

void EffectChainStrip::mouseUp(const juce::MouseEvent& e)
{
    if (dragging && dragSource >= 0 && dragOver >= 0 && dragSource != dragOver)
        proc.swapChainSlots(dragSource, dragOver);

    dragging   = false;
    dragSource = -1;
    dragOver   = -1;
    repaint();
}

void EffectChainStrip::mouseExit(const juce::MouseEvent&)
{
    if (dragging) { dragging = false; dragSource = dragOver = -1; repaint(); }
}

//==============================================================================
// EffectsPanel

EffectsPanel::EffectsPanel(PluginProcessor& p)
    : processorRef(p), chainStrip(p)
{
    addAndMakeVisible(chainStrip);

    // --- Chorus ---
    setupLabel(chorusSectionLabel, "CHORUS");
    addAndMakeVisible(chorusSectionLabel);
    setupSlider(chorusRateSlider,  0.1, 5.0, 0.5);
    setupSlider(chorusDepthSlider, 0.0, 1.0, 0.5);
    setupSlider(chorusMixSlider,   0.0, 1.0, 0.5);
    setupLabel(chorusRateLabel,  "Rate");
    setupLabel(chorusDepthLabel, "Depth");
    setupLabel(chorusMixLabel,   "Mix");
    addAndMakeVisible(chorusRateSlider);   addAndMakeVisible(chorusRateLabel);
    addAndMakeVisible(chorusDepthSlider);  addAndMakeVisible(chorusDepthLabel);
    addAndMakeVisible(chorusMixSlider);    addAndMakeVisible(chorusMixLabel);

    // --- Distortion ---
    setupLabel(distortionSectionLabel, "DISTORTION");
    addAndMakeVisible(distortionSectionLabel);
    setupSlider(distDriveSlider, 1.0, 100.0, 10.0, 0.3);
    setupSlider(distToneSlider,  0.0,   1.0,  0.7);
    setupSlider(distMixSlider,   0.0,   1.0,  0.5);
    setupLabel(distDriveLabel, "Drive");
    setupLabel(distToneLabel,  "Tone");
    setupLabel(distMixLabel,   "Mix");
    addAndMakeVisible(distDriveSlider); addAndMakeVisible(distDriveLabel);
    addAndMakeVisible(distToneSlider);  addAndMakeVisible(distToneLabel);
    addAndMakeVisible(distMixSlider);   addAndMakeVisible(distMixLabel);

    // --- EQ ---
    setupLabel(eqSectionLabel, "3-BAND EQ");
    addAndMakeVisible(eqSectionLabel);
    setupSlider(eqLowSlider,  -12.0, 12.0, 0.0);
    setupSlider(eqMidSlider,  -12.0, 12.0, 0.0);
    setupSlider(eqHighSlider, -12.0, 12.0, 0.0);
    setupLabel(eqLowLabel,  "Low");
    setupLabel(eqMidLabel,  "Mid");
    setupLabel(eqHighLabel, "High");
    addAndMakeVisible(eqLowSlider);   addAndMakeVisible(eqLowLabel);
    addAndMakeVisible(eqMidSlider);   addAndMakeVisible(eqMidLabel);
    addAndMakeVisible(eqHighSlider);  addAndMakeVisible(eqHighLabel);

    // --- Compressor ---
    setupLabel(compressorSectionLabel, "COMPRESSOR");
    addAndMakeVisible(compressorSectionLabel);
    setupSlider(compThreshSlider,  -60.0,    0.0, -20.0);
    setupSlider(compRatioSlider,     1.0,   20.0,   4.0, 0.4);
    setupSlider(compAttackSlider,    0.1,  100.0,  10.0, 0.5);
    setupSlider(compReleaseSlider,  10.0, 1000.0, 100.0, 0.4);
    setupSlider(compMakeupSlider,    0.0,   24.0,   0.0);
    setupLabel(compThreshLabel,  "Thresh");
    setupLabel(compRatioLabel,   "Ratio");
    setupLabel(compAttackLabel,  "Attack");
    setupLabel(compReleaseLabel, "Release");
    setupLabel(compMakeupLabel,  "Makeup");
    addAndMakeVisible(compThreshSlider);  addAndMakeVisible(compThreshLabel);
    addAndMakeVisible(compRatioSlider);   addAndMakeVisible(compRatioLabel);
    addAndMakeVisible(compAttackSlider);  addAndMakeVisible(compAttackLabel);
    addAndMakeVisible(compReleaseSlider); addAndMakeVisible(compReleaseLabel);
    addAndMakeVisible(compMakeupSlider);  addAndMakeVisible(compMakeupLabel);

    // --- Delay ---
    setupLabel(delaySectionLabel, "DELAY");
    addAndMakeVisible(delaySectionLabel);
    setupSlider(delayTimeSlider,     0.0, 2.0,  0.5, 0.4);
    setupSlider(delayFeedbackSlider, 0.0, 0.95, 0.3);
    setupSlider(delayMixSlider,      0.0, 1.0,  0.5);
    setupLabel(delayTimeLabel,     "Time");
    setupLabel(delayFeedbackLabel, "Feedback");
    setupLabel(delayMixLabel,      "Mix");
    addAndMakeVisible(delayTimeSlider);     addAndMakeVisible(delayTimeLabel);
    addAndMakeVisible(delayFeedbackSlider); addAndMakeVisible(delayFeedbackLabel);
    addAndMakeVisible(delayMixSlider);      addAndMakeVisible(delayMixLabel);

    // --- Reverb ---
    setupLabel(reverbSectionLabel, "REVERB");
    addAndMakeVisible(reverbSectionLabel);
    setupSlider(reverbRoomSlider,     0.0, 1.0,   0.5);
    setupSlider(reverbDampSlider,     0.0, 1.0,   0.5);
    setupSlider(reverbWetSlider,      0.0, 1.0,   0.33);
    setupSlider(reverbPreDelaySlider, 0.0, 200.0, 0.0);
    setupSlider(reverbLFDampSlider,   0.0, 1.0,   0.0);
    setupSlider(reverbWidthSlider,    0.0, 1.0,   1.0);
    setupLabel(reverbRoomLabel,     "Room");
    setupLabel(reverbDampLabel,     "HF Damp");
    setupLabel(reverbWetLabel,      "Wet");
    setupLabel(reverbPreDelayLabel, "Pre-Delay");
    setupLabel(reverbLFDampLabel,   "LF Damp");
    setupLabel(reverbWidthLabel,    "Width");
    setupLabel(reverbFreezeLabel,   "Freeze");
    reverbFreezeButton.setButtonText("Freeze");
    reverbFreezeButton.setClickingTogglesState(true);
    addAndMakeVisible(reverbRoomSlider);     addAndMakeVisible(reverbRoomLabel);
    addAndMakeVisible(reverbDampSlider);     addAndMakeVisible(reverbDampLabel);
    addAndMakeVisible(reverbWetSlider);      addAndMakeVisible(reverbWetLabel);
    addAndMakeVisible(reverbPreDelaySlider); addAndMakeVisible(reverbPreDelayLabel);
    addAndMakeVisible(reverbLFDampSlider);   addAndMakeVisible(reverbLFDampLabel);
    addAndMakeVisible(reverbWidthSlider);    addAndMakeVisible(reverbWidthLabel);
    addAndMakeVisible(reverbFreezeButton);   addAndMakeVisible(reverbFreezeLabel);

    // --- APVTS attachments ---
    auto& apvts = processorRef.apvts;

    chorusRateAttach  = std::make_unique<SliderAttachment>(apvts, "chorusRate",  chorusRateSlider);
    chorusDepthAttach = std::make_unique<SliderAttachment>(apvts, "chorusDepth", chorusDepthSlider);
    chorusMixAttach   = std::make_unique<SliderAttachment>(apvts, "chorusMix",   chorusMixSlider);

    distDriveAttach   = std::make_unique<SliderAttachment>(apvts, "distDrive",   distDriveSlider);
    distToneAttach    = std::make_unique<SliderAttachment>(apvts, "distTone",    distToneSlider);
    distMixAttach     = std::make_unique<SliderAttachment>(apvts, "distMix",     distMixSlider);

    eqLowAttach       = std::make_unique<SliderAttachment>(apvts, "eqLowGain",   eqLowSlider);
    eqMidAttach       = std::make_unique<SliderAttachment>(apvts, "eqMidGain",   eqMidSlider);
    eqHighAttach      = std::make_unique<SliderAttachment>(apvts, "eqHighGain",  eqHighSlider);

    compThreshAttach  = std::make_unique<SliderAttachment>(apvts, "compThreshold", compThreshSlider);
    compRatioAttach   = std::make_unique<SliderAttachment>(apvts, "compRatio",     compRatioSlider);
    compAttackAttach  = std::make_unique<SliderAttachment>(apvts, "compAttack",    compAttackSlider);
    compReleaseAttach = std::make_unique<SliderAttachment>(apvts, "compRelease",   compReleaseSlider);
    compMakeupAttach  = std::make_unique<SliderAttachment>(apvts, "compMakeup",    compMakeupSlider);

    delayTimeAttach     = std::make_unique<SliderAttachment>(apvts, "delayTime",     delayTimeSlider);
    delayFeedbackAttach = std::make_unique<SliderAttachment>(apvts, "delayFeedback", delayFeedbackSlider);
    delayMixAttach      = std::make_unique<SliderAttachment>(apvts, "delayMix",      delayMixSlider);

    reverbRoomAttach     = std::make_unique<SliderAttachment>(apvts, "reverbRoom",     reverbRoomSlider);
    reverbDampAttach     = std::make_unique<SliderAttachment>(apvts, "reverbDamp",     reverbDampSlider);
    reverbWetAttach      = std::make_unique<SliderAttachment>(apvts, "reverbWet",      reverbWetSlider);
    reverbPreDelayAttach = std::make_unique<SliderAttachment>(apvts, "reverbPreDelay", reverbPreDelaySlider);
    reverbLFDampAttach   = std::make_unique<SliderAttachment>(apvts, "reverbLFDamp",   reverbLFDampSlider);
    reverbWidthAttach    = std::make_unique<SliderAttachment>(apvts, "reverbWidth",    reverbWidthSlider);
    reverbFreezeAttach   = std::make_unique<ButtonAttachment>(apvts, "reverbFreeze",   reverbFreezeButton);

    // --- MIDI Learn init (after attachments) ---
    chorusRateSlider.init (processorRef, "chorusRate");
    chorusDepthSlider.init(processorRef, "chorusDepth");
    chorusMixSlider.init  (processorRef, "chorusMix");

    distDriveSlider.init(processorRef, "distDrive");
    distToneSlider.init (processorRef, "distTone");
    distMixSlider.init  (processorRef, "distMix");

    eqLowSlider.init (processorRef, "eqLowGain");
    eqMidSlider.init (processorRef, "eqMidGain");
    eqHighSlider.init(processorRef, "eqHighGain");

    compThreshSlider.init (processorRef, "compThreshold");
    compRatioSlider.init  (processorRef, "compRatio");
    compAttackSlider.init (processorRef, "compAttack");
    compReleaseSlider.init(processorRef, "compRelease");
    compMakeupSlider.init (processorRef, "compMakeup");

    delayTimeSlider.init    (processorRef, "delayTime");
    delayFeedbackSlider.init(processorRef, "delayFeedback");
    delayMixSlider.init     (processorRef, "delayMix");

    reverbRoomSlider.init    (processorRef, "reverbRoom");
    reverbDampSlider.init    (processorRef, "reverbDamp");
    reverbWetSlider.init     (processorRef, "reverbWet");
    reverbPreDelaySlider.init(processorRef, "reverbPreDelay");
    reverbLFDampSlider.init  (processorRef, "reverbLFDamp");
    reverbWidthSlider.init   (processorRef, "reverbWidth");

    // Tooltips
    chorusRateSlider.setTooltip  ("Chorus Rate: LFO speed (0.1–5 Hz). Higher = faster modulation.");
    chorusDepthSlider.setTooltip ("Chorus Depth: modulation depth (0–100%). Higher = wider pitch wobble.");
    chorusMixSlider.setTooltip   ("Chorus Mix: wet/dry blend (0=dry, 1=fully wet)");
    distDriveSlider.setTooltip   ("Distortion Drive: saturation amount (1–100×). Higher = more clipping.");
    distToneSlider.setTooltip    ("Distortion Tone: low-pass filter on distorted signal (0=dark, 1=bright)");
    distMixSlider.setTooltip     ("Distortion Mix: wet/dry blend");
    eqLowSlider.setTooltip       ("EQ Low Shelf: gain at 250 Hz (±12 dB)");
    eqMidSlider.setTooltip       ("EQ Mid Peak: gain at 1 kHz (±12 dB)");
    eqHighSlider.setTooltip      ("EQ High Shelf: gain at 4 kHz (±12 dB)");
    compThreshSlider.setTooltip  ("Compressor Threshold: level above which compression starts (−60–0 dB)");
    compRatioSlider.setTooltip   ("Compressor Ratio: gain reduction above threshold (1:1–20:1)");
    compAttackSlider.setTooltip  ("Compressor Attack: time to engage compression (0.1–100 ms)");
    compReleaseSlider.setTooltip ("Compressor Release: time to disengage compression (10–1000 ms)");
    compMakeupSlider.setTooltip  ("Compressor Makeup Gain: output boost to compensate for gain reduction (0–24 dB)");
    delayTimeSlider.setTooltip   ("Delay Time: echo delay length (0–2 s)");
    delayFeedbackSlider.setTooltip("Delay Feedback: echo repeat decay (0–95%). High values = long tail.");
    delayMixSlider.setTooltip    ("Delay Mix: wet/dry blend");
    reverbRoomSlider.setTooltip  ("Reverb Room Size: acoustic size of the simulated space (0–100%)");
    reverbDampSlider.setTooltip  ("Reverb HF Damping: high-frequency absorption (0=bright, 1=dark)");
    reverbWetSlider.setTooltip   ("Reverb Wet Level: amount of reverb in the output (0–100%)");
    reverbPreDelaySlider.setTooltip("Reverb Pre-Delay: time before reverb onset (0–200 ms)");
    reverbLFDampSlider.setTooltip("Reverb LF Damping: low-frequency damping on the wet signal");
    reverbWidthSlider.setTooltip ("Reverb Width: stereo spread (0=mono, 1=full stereo)");
    reverbFreezeButton.setTooltip("Reverb Freeze: hold the reverb tail indefinitely");
}

void EffectsPanel::setupSlider(juce::Slider& s, double min, double max, double value, double skew)
{
    s.setSliderStyle(juce::Slider::Rotary);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    s.setRange(min, max);
    s.setSkewFactor(skew);
    s.setValue(value, juce::dontSendNotification);
}

void EffectsPanel::setupLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, CyberpunkTheme::textSecondary);
}

void EffectsPanel::paint(juce::Graphics& g)
{
    g.fillAll(CyberpunkTheme::bgBase);

    // Draw neumorphic section cards
    const float cr = 8.0f;
    if (chorusSectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, chorusSectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(chorusSectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(chorusSectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
    if (distortionSectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, distortionSectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(distortionSectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(distortionSectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
    if (eqSectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, eqSectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(eqSectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(eqSectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
    if (compressorSectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, compressorSectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(compressorSectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(compressorSectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
    if (delaySectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, delaySectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(delaySectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(delaySectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
    if (reverbSectionBounds.getHeight() > 0)
    {
        CyberpunkTheme::drawNeumorphicRect(g, reverbSectionBounds.toFloat(), cr, 3.0f);
        g.setColour(CyberpunkTheme::bgRaised);
        g.fillRoundedRectangle(reverbSectionBounds.toFloat(), cr);
        g.setColour(CyberpunkTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(reverbSectionBounds.toFloat().reduced(0.5f), cr, 1.0f);
    }
}

void EffectsPanel::resized()
{
    auto area = getLocalBounds().reduced(16);
    const int knobSz  = 80;
    const int labelH  = 18;
    const int sectionH = labelH + knobSz + labelH; // 116 px
    const int gap      = 10;

    // Chain strip at top
    chainStrip.setBounds(area.removeFromTop(56));
    area.removeFromTop(gap);

    // Two-column split
    const int colGap = 12;
    auto leftCol  = area.removeFromLeft((area.getWidth() - colGap) / 2);
    area.removeFromLeft(colGap);
    auto rightCol = area;

    auto placeKnob = [&](juce::Rectangle<int>& row, juce::Slider& knob, juce::Label& lbl)
    {
        auto col = row.removeFromLeft(knobSz + gap);
        knob.setBounds(col.removeFromTop(knobSz));
        lbl .setBounds(col.removeFromTop(labelH));
        row.removeFromLeft(gap);
    };

    // ── LEFT: Chorus, Distortion, EQ ─────────────────────────────────────────

    // Chorus
    {
        auto sec = leftCol.removeFromTop(sectionH);
        chorusSectionBounds = sec;
        chorusSectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, chorusRateSlider,  chorusRateLabel);
        placeKnob(sec, chorusDepthSlider, chorusDepthLabel);
        placeKnob(sec, chorusMixSlider,   chorusMixLabel);
    }
    leftCol.removeFromTop(gap);

    // Distortion
    {
        auto sec = leftCol.removeFromTop(sectionH);
        distortionSectionBounds = sec;
        distortionSectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, distDriveSlider, distDriveLabel);
        placeKnob(sec, distToneSlider,  distToneLabel);
        placeKnob(sec, distMixSlider,   distMixLabel);
    }
    leftCol.removeFromTop(gap);

    // EQ
    {
        auto sec = leftCol.removeFromTop(sectionH);
        eqSectionBounds = sec;
        eqSectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, eqLowSlider,  eqLowLabel);
        placeKnob(sec, eqMidSlider,  eqMidLabel);
        placeKnob(sec, eqHighSlider, eqHighLabel);
    }

    // ── RIGHT: Compressor, Delay, Reverb ─────────────────────────────────────

    // Compressor (5 knobs)
    {
        auto sec = rightCol.removeFromTop(sectionH);
        compressorSectionBounds = sec;
        compressorSectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, compThreshSlider,  compThreshLabel);
        placeKnob(sec, compRatioSlider,   compRatioLabel);
        placeKnob(sec, compAttackSlider,  compAttackLabel);
        placeKnob(sec, compReleaseSlider, compReleaseLabel);
        placeKnob(sec, compMakeupSlider,  compMakeupLabel);
    }
    rightCol.removeFromTop(gap);

    // Delay
    {
        auto sec = rightCol.removeFromTop(sectionH);
        delaySectionBounds = sec;
        delaySectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, delayTimeSlider,     delayTimeLabel);
        placeKnob(sec, delayFeedbackSlider, delayFeedbackLabel);
        placeKnob(sec, delayMixSlider,      delayMixLabel);
    }
    rightCol.removeFromTop(gap);

    // Reverb row 1
    {
        auto sec = rightCol.removeFromTop(sectionH);
        reverbSectionBounds = sec;
        reverbSectionLabel.setBounds(sec.removeFromTop(labelH));
        placeKnob(sec, reverbRoomSlider, reverbRoomLabel);
        placeKnob(sec, reverbDampSlider, reverbDampLabel);
        placeKnob(sec, reverbWetSlider,  reverbWetLabel);
    }
    rightCol.removeFromTop(gap / 2);

    // Reverb row 2: Pre-Delay, LF Damp, Width, Freeze
    {
        auto row2 = rightCol.removeFromTop(knobSz + labelH);
        placeKnob(row2, reverbPreDelaySlider, reverbPreDelayLabel);
        placeKnob(row2, reverbLFDampSlider,   reverbLFDampLabel);
        placeKnob(row2, reverbWidthSlider,    reverbWidthLabel);
        auto freezeCol = row2.removeFromLeft(knobSz);
        reverbFreezeButton.setBounds(freezeCol.removeFromTop(knobSz / 2).reduced(4));
        reverbFreezeLabel .setBounds(freezeCol.removeFromTop(labelH));
    }
}
