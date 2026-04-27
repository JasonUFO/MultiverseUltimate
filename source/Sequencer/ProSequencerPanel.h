#pragma once
#include <JuceHeader.h>
#include "ProSequencer.h"

// Single step button — active/highlighted states, velocity bar, note name
class ProStepButton : public juce::Component
{
public:
    std::function<void()> onToggle;
    std::function<void()> onRightClick;

    void setActive      (bool a)  { active = a; repaint(); }
    void setNote        (int n)   { note = n;   repaint(); }
    void setVelocity    (float v) { velocity = v; repaint(); }
    void setHighlighted (bool h)  { highlighted = h; repaint(); }
    bool isActive() const { return active; }

    void paint       (juce::Graphics& g) override;
    void mouseDown   (const juce::MouseEvent& e) override;

    static juce::String noteName (int midi);

private:
    bool  active      = false;
    bool  highlighted = false;
    int   note        = 60;
    float velocity    = 0.8f;
};

// Full 4-lane × 32-step panel with per-step editor
class ProSequencerPanel : public juce::Component, public juce::Timer
{
public:
    explicit ProSequencerPanel (ProSequencer& seq);
    ~ProSequencerPanel() override;

    void paint   (juce::Graphics& g) override;
    void resized ()                  override;
    void timerCallback ()            override;

private:
    ProSequencer& sequencer;
    int selectedLane = 0;
    int selectedStep = -1;
    int lastHighlightedStep = -1;

    // Lane selector
    std::array<juce::TextButton, PRO_SEQ_LANES> laneButtons;

    // Play mode
    juce::Label     playModeLabel;
    juce::TextButton fwdBtn { "FWD" };
    juce::TextButton revBtn { "REV" };
    juce::TextButton rndBtn { "RND" };

    // Num steps
    juce::Label    numStepsLabel;
    juce::ComboBox numStepsBox;

    // Row labels
    juce::Label row1Label, row2Label;

    // Step grid (32 buttons, laid out in 2 rows of 16)
    std::array<ProStepButton, PRO_SEQ_STEPS> stepButtons;

    // Step editor
    juce::Label stepEditorTitle;

    juce::Label      noteLabel;
    juce::TextButton noteBtn { "C4" };

    juce::Slider velocitySlider, gateSlider, probSlider;
    juce::Label  velocityLabel,  gateLabel,  probLabel;

    juce::Label    ratchetLabel;
    juce::ComboBox ratchetBox;

    // Helpers
    void selectLane          (int lane);
    void selectStep          (int step);
    void refreshStepGrid     ();
    void refreshStepEditor   ();
    void updatePlayModeButtons ();
    void applyPlayMode       (ProPlayMode mode);
    void showNoteMenu        (int stepIdx);
};
