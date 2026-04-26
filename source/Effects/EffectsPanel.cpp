#include "EffectsPanel.h"
#include "../PluginProcessor.h"

EffectsPanel::EffectsPanel(PluginProcessor& p)
    : processorRef(p)
{
    // --- Delay section ---
    setupLabel(delaySectionLabel, "DELAY");
    addAndMakeVisible(delaySectionLabel);

    setupSlider(delayTimeSlider,     0.0, 2.0,  processorRef.getBaseDelayTime(),     0.4);
    setupSlider(delayFeedbackSlider, 0.0, 0.95, processorRef.getBaseDelayFeedback());
    setupSlider(delayMixSlider,      0.0, 1.0,  processorRef.getBaseDelayMix());

    delayTimeSlider.onValueChange     = [this] { processorRef.setBaseDelayTime    ((float)delayTimeSlider.getValue()); };
    delayFeedbackSlider.onValueChange = [this] { processorRef.setBaseDelayFeedback((float)delayFeedbackSlider.getValue()); };
    delayMixSlider.onValueChange      = [this] { processorRef.setBaseDelayMix     ((float)delayMixSlider.getValue()); };

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

    setupSlider(reverbRoomSlider, 0.0, 1.0, processorRef.getBaseReverbRoom());
    setupSlider(reverbDampSlider, 0.0, 1.0, processorRef.getBaseReverbDamp());
    setupSlider(reverbWetSlider,  0.0, 1.0, processorRef.getBaseReverbWet());

    reverbRoomSlider.onValueChange = [this] { processorRef.setBaseReverbRoom((float)reverbRoomSlider.getValue()); };
    reverbDampSlider.onValueChange = [this] { processorRef.setBaseReverbDamp((float)reverbDampSlider.getValue()); };
    reverbWetSlider.onValueChange  = [this] { processorRef.setBaseReverbWet ((float)reverbWetSlider.getValue()); };

    setupLabel(reverbRoomLabel, "Room");
    setupLabel(reverbDampLabel, "Damp");
    setupLabel(reverbWetLabel,  "Wet");

    addAndMakeVisible(reverbRoomSlider);
    addAndMakeVisible(reverbDampSlider);
    addAndMakeVisible(reverbWetSlider);
    addAndMakeVisible(reverbRoomLabel);
    addAndMakeVisible(reverbDampLabel);
    addAndMakeVisible(reverbWetLabel);
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
    const int rowH   = 30;
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

    // Reverb row
    auto reverbRow = area.removeFromTop(labelH + knobSz + labelH);
    reverbSectionLabel.setBounds(reverbRow.removeFromTop(labelH));
    placeKnob(reverbRow, reverbRoomSlider, reverbRoomLabel);
    placeKnob(reverbRow, reverbDampSlider, reverbDampLabel);
    placeKnob(reverbRow, reverbWetSlider,  reverbWetLabel);

    (void) rowH; // used implicitly via labelH + knobSz
}
