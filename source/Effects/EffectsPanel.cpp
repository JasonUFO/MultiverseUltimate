#include "EffectsPanel.h"
#include "../PluginProcessor.h"

EffectsPanel::EffectsPanel(PluginProcessor& p)
    : processorRef(p)
{
    // --- Delay section ---
    setupLabel(delaySectionLabel, "DELAY");
    addAndMakeVisible(delaySectionLabel);

    setupSlider(delayTimeSlider,     0.0, 2.0,  0.5, 0.4);
    setupSlider(delayFeedbackSlider, 0.0, 0.95, 0.3);
    setupSlider(delayMixSlider,      0.0, 1.0,  0.5);

    setupLabel(delayTimeLabel,     "Time");
    setupLabel(delayFeedbackLabel, "Feedback");
    setupLabel(delayMixLabel,      "Mix");

    addAndMakeVisible(delayTimeSlider);
    addAndMakeVisible(delayFeedbackSlider);
    addAndMakeVisible(delayMixSlider);
    addAndMakeVisible(delayTimeLabel);
    addAndMakeVisible(delayFeedbackLabel);
    addAndMakeVisible(delayMixLabel);

    // --- Reverb section ---
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

    addAndMakeVisible(reverbRoomSlider);
    addAndMakeVisible(reverbDampSlider);
    addAndMakeVisible(reverbWetSlider);
    addAndMakeVisible(reverbRoomLabel);
    addAndMakeVisible(reverbDampLabel);
    addAndMakeVisible(reverbWetLabel);
    addAndMakeVisible(reverbPreDelaySlider);
    addAndMakeVisible(reverbLFDampSlider);
    addAndMakeVisible(reverbWidthSlider);
    addAndMakeVisible(reverbPreDelayLabel);
    addAndMakeVisible(reverbLFDampLabel);
    addAndMakeVisible(reverbWidthLabel);
    addAndMakeVisible(reverbFreezeButton);
    addAndMakeVisible(reverbFreezeLabel);

    // APVTS attachments
    auto& apvts = processorRef.apvts;
    delayTimeAttach     = std::make_unique<SliderAttachment>(apvts, "delayTime",     delayTimeSlider);
    delayFeedbackAttach = std::make_unique<SliderAttachment>(apvts, "delayFeedback", delayFeedbackSlider);
    delayMixAttach      = std::make_unique<SliderAttachment>(apvts, "delayMix",      delayMixSlider);
    reverbRoomAttach    = std::make_unique<SliderAttachment>(apvts, "reverbRoom",    reverbRoomSlider);
    reverbDampAttach    = std::make_unique<SliderAttachment>(apvts, "reverbDamp",    reverbDampSlider);
    reverbWetAttach     = std::make_unique<SliderAttachment>(apvts, "reverbWet",     reverbWetSlider);
    reverbPreDelayAttach = std::make_unique<SliderAttachment>(apvts, "reverbPreDelay", reverbPreDelaySlider);
    reverbLFDampAttach  = std::make_unique<SliderAttachment>(apvts, "reverbLFDamp", reverbLFDampSlider);
    reverbWidthAttach   = std::make_unique<SliderAttachment>(apvts, "reverbWidth",   reverbWidthSlider);
    reverbFreezeAttach  = std::make_unique<ButtonAttachment>(apvts, "reverbFreeze",  reverbFreezeButton);

    // MIDI Learn indicators — must be called after attachments
    delayTimeSlider.init    (processorRef, "delayTime");
    delayFeedbackSlider.init(processorRef, "delayFeedback");
    delayMixSlider.init     (processorRef, "delayMix");
    reverbRoomSlider.init   (processorRef, "reverbRoom");
    reverbDampSlider.init   (processorRef, "reverbDamp");
    reverbWetSlider.init    (processorRef, "reverbWet");
    reverbPreDelaySlider.init(processorRef, "reverbPreDelay");
    reverbLFDampSlider.init (processorRef, "reverbLFDamp");
    reverbWidthSlider.init  (processorRef, "reverbWidth");
}

void EffectsPanel::setupSlider(juce::Slider& s, double min, double max, double value, double skew)
{
    s.setSliderStyle(juce::Slider::Rotary);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    s.setRange(min, max);
    s.setSkewFactor(skew);
    s.setValue(value, juce::dontSendNotification);
}

void EffectsPanel::setupLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
}

void EffectsPanel::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void EffectsPanel::resized()
{
    auto area = getLocalBounds().reduced(16);
    const int knobSz = 90;
    const int labelH = 20;
    const int gap    = 12;

    auto placeKnob = [&](juce::Rectangle<int>& row, juce::Slider& knob, juce::Label& lbl)
    {
        auto col = row.removeFromLeft(knobSz + gap);
        knob.setBounds(col.removeFromTop(knobSz));
        lbl.setBounds(col.removeFromTop(labelH));
        row.removeFromLeft(gap);
    };

    // Delay row
    auto delayRow = area.removeFromTop(labelH + knobSz + labelH);
    delaySectionLabel.setBounds(delayRow.removeFromTop(labelH));
    placeKnob(delayRow, delayTimeSlider,     delayTimeLabel);
    placeKnob(delayRow, delayFeedbackSlider, delayFeedbackLabel);
    placeKnob(delayRow, delayMixSlider,      delayMixLabel);

    area.removeFromTop(gap);

    // Reverb row 1: Room, HF Damp, Wet
    auto reverbRow1 = area.removeFromTop(labelH + knobSz + labelH);
    reverbSectionLabel.setBounds(reverbRow1.removeFromTop(labelH));
    placeKnob(reverbRow1, reverbRoomSlider, reverbRoomLabel);
    placeKnob(reverbRow1, reverbDampSlider, reverbDampLabel);
    placeKnob(reverbRow1, reverbWetSlider,  reverbWetLabel);

    area.removeFromTop(gap / 2);

    // Reverb row 2: Pre-Delay, LF Damp, Width, Freeze
    auto reverbRow2 = area.removeFromTop(knobSz + labelH);
    placeKnob(reverbRow2, reverbPreDelaySlider, reverbPreDelayLabel);
    placeKnob(reverbRow2, reverbLFDampSlider,   reverbLFDampLabel);
    placeKnob(reverbRow2, reverbWidthSlider,    reverbWidthLabel);

    // Freeze button in the remaining space of row 2
    auto freezeCol = reverbRow2.removeFromLeft(knobSz);
    reverbFreezeButton.setBounds(freezeCol.removeFromTop(knobSz / 2).reduced(4));
    reverbFreezeLabel.setBounds(freezeCol.removeFromTop(labelH));
}
