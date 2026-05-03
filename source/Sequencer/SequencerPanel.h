#pragma once
#include <JuceHeader.h>
#include "Sequencer.h"

class SequencerPanel;

class StepButton : public juce::Component
{
public:
    std::function<void()> onLeftClick;
    std::function<void()> onRightClick;

    void setActive (bool a) { active = a; repaint(); }
    bool isActive() const { return active; }
    void setNote (int n) { noteNumber = n; repaint(); }
    int getNote() const { return noteNumber; }
    void setHighlighted (bool h) { highlighted = h; repaint(); }

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    static juce::String noteName (int midiNote);

private:
    bool active = false;
    int noteNumber = 60;
    bool highlighted = false;
};

class SequencerPanel : public juce::Component, public juce::Timer
{
public:
    explicit SequencerPanel (Sequencer& seq);
    ~SequencerPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    Sequencer& sequencer;

    std::array<StepButton, MAX_STEPS> stepButtons;

    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton modeButton { "SEQ" };
    std::array<juce::TextButton, MAX_PATTERNS> patternButtons;
    juce::TextButton exportButton { "Export MIDI" };

    std::unique_ptr<juce::FileChooser> fileChooser;
    int lastHighlightedStep = -1;

    // Section card bounds
    juce::Rectangle<int> transportBounds, patternBounds, stepGridBounds, exportBounds;

    void showNoteMenu (int stepIndex);
    void exportMidi();
    void updatePatternButtons();
    void refreshStepDisplay();
};
