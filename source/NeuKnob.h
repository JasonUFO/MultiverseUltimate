#pragma once
#include "MidiLearnSlider.h"
#include "CyberpunkTheme.h"

// Extends MidiLearnSlider (MIDI learn + macro badge) with two visual extras:
//   1. Value pill — rounded rect above the knob showing the current value
//      while the mouse is over or dragging.
//   2. Amber arc — when assigned to a macro the fill arc switches from
//      accentBlue to accentAmber so macro-linked knobs are instantly obvious.
//
// Safe to use as a drop-in for linear sliders too; the pill only renders
// for Rotary styles and the amber arc works for any style.
class NeuKnob : public MidiLearnSlider
{
public:
    NeuKnob();
    ~NeuKnob() override;

    void paint (juce::Graphics& g) override;

private:
    bool lastMacroState = false;

    struct ArcTimer : juce::Timer
    {
        NeuKnob& owner;
        explicit ArcTimer (NeuKnob& o) : owner (o) {}
        void timerCallback() override { owner.arcTimerTick(); }
    };
    ArcTimer arcTimer { *this };

    void arcTimerTick();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeuKnob)
};
