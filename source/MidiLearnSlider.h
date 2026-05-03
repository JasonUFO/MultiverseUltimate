#pragma once
#include <JuceHeader.h>

class PluginProcessor;

// Drop-in replacement for juce::Slider that shows an orange "L" badge when a
// MIDI CC / pitch-bend / channel-pressure is learned for this parameter, and
// offers a right-click menu to remove the mapping.
//
// Usage:
//   1. Declare as MidiLearnSlider instead of juce::Slider in your panel.
//   2. After creating the APVTS SliderAttachment, call slider.init(proc, "paramID").
//   3. Everything else (setupSlider, addAndMakeVisible, etc.) stays the same.
class MidiLearnSlider : public juce::Slider, private juce::Timer
{
public:
    MidiLearnSlider();
    ~MidiLearnSlider() override;

    // Call once after the APVTS attachment is created.
    void init (PluginProcessor& p, const juce::String& pid);

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    bool isMacroAssigned() const { return macroAssigned; }

private:
    PluginProcessor* proc         = nullptr;
    juce::String     paramID;
    bool             mapped       = false; // cached; updated by timer
    bool             macroAssigned = false;

    void timerCallback() override;
    bool checkHasMapping() const;
    bool checkHasMacro() const;
    void showContextMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiLearnSlider)
};
