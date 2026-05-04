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
    void setProbability (float p) { probability = p; repaint(); }

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;

    static juce::String noteName (int midiNote);

private:
    bool active = false;
    int noteNumber = 60;
    bool highlighted = false;
    float probability = 1.0f;
};

class SequencerPanel : public juce::Component,
                       public juce::Timer,
                       public juce::FileDragAndDropTarget
{
public:
    explicit SequencerPanel (Sequencer& seq);
    ~SequencerPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    Sequencer& sequencer;

    std::array<StepButton, MAX_STEPS> stepButtons;

    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton modeButton { "SEQ" };
    juce::ComboBox stepLengthCombo;
    juce::Label stepLengthLabel;
    std::array<juce::TextButton, MAX_PATTERNS> patternButtons;
    juce::TextButton exportButton { "Export MIDI" };

    juce::Label chordLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;
    int lastHighlightedStep = -1;
    bool dragOver = false;

    // Section card bounds
    juce::Rectangle<int> transportBounds, patternBounds, stepGridBounds, exportBounds;

    void showNoteMenu (int stepIndex);
    void exportMidi();
    void updatePatternButtons();
    void refreshStepDisplay();
    void importMidiFile (const juce::File& file);
    juce::String detectChord() const;
};
