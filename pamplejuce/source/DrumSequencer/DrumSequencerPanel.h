#pragma once
#include "DrumSequencer.h"
#include <juce_gui_basics/juce_gui_basics.h>

class DrumSequencerPanel : public juce::Component,
                           public juce::Timer,
                           public juce::FileDragAndDropTarget
{
public:
    explicit DrumSequencerPanel (DrumSequencer& seq);
    ~DrumSequencerPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;

private:
    class DrumStepButton : public juce::Component
    {
    public:
        int track = 0;
        int step = 0;
        std::function<void()> onLeftClick;
        std::function<void()> onRightClick;

        void setActive (bool a) { active = a; repaint(); }
        bool isActive() const { return active; }
        void setHighlighted (bool h) { highlighted = h; repaint(); }

        void paint (juce::Graphics& g) override;
        void mouseDown (const juce::MouseEvent& e) override;

    private:
        bool active = false;
        bool highlighted = false;
    };

    class TrackRow : public juce::Component
    {
    public:
        int trackIndex = 0;
        DrumSequencer& sequencer;
        DrumSequencerPanel& panel;

        juce::Label nameLabel;
        juce::Slider volumeSlider;
        juce::TextButton muteButton { "M" };
        juce::TextButton soloButton { "S" };
        juce::TextButton loadButton { "+" };
        juce::Label levelMeter;

        TrackRow (int idx, DrumSequencer& seq, DrumSequencerPanel& p);

        void updateFromSequencer();
        void paint (juce::Graphics& g) override;
        void resized() override;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackRow)
    };

    DrumSequencer& sequencer;

    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::TextButton playButton { "Play" };
    juce::TextButton stopButton { "Stop" };

    std::array<juce::TextButton, MAX_DRUM_PATTERNS> patternButtons;

    std::array<std::array<DrumStepButton, DRUM_STEPS>, DRUM_TRACK_COUNT> stepButtons;
    std::vector<std::unique_ptr<TrackRow>> trackRows;

    int lastHighlightedStep = -1;
    bool isDragOver = false;
    juce::Label dropLabel;

    void updatePatternButtons();
    void refreshStepDisplay();
    void showVelocityMenu (int track, int step);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumSequencerPanel)
};