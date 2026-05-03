#pragma once
#include "WavetableOscillator.h"
#include <JuceHeader.h>

class WavetableEditor : public juce::Component,
                        public juce::Button::Listener,
                        public juce::ComboBox::Listener,
                        public juce::Timer
{
public:
    WavetableEditor(WavetableOscillator& osc);
    ~WavetableEditor() override;

    std::function<void()> onWavetableChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void timerCallback() override;

    // Mouse interaction for drawing
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

private:
    WavetableOscillator& oscillator;

    // UI Components
    juce::ComboBox frameSelector;
    juce::Label frameLabel;

    // Draw tools
    juce::TextButton pencilBtn, lineBtn, curveBtn;
    juce::Label toolLabel;

    // Process menu
    juce::TextButton normalizeBtn, fadeBtn, reverseBtn, fftBtn, clearBtn;
    juce::TextButton formulaSineBtn, formulaSawBtn, formulaSquareBtn, formulaTriBtn;
    juce::Label processLabel;

    // Import
    juce::TextButton importBtn, importMultiBtn;
    juce::Label        importLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Close
    juce::TextButton closeBtn;

    // Drawing state
    bool isDrawing = false;
    juce::Array<juce::Point<float>> drawPoints;
    int currentTool = 0; // 0=pencil, 1=line, 2=curve
    juce::Point<float> lineStart;

    // Display
    juce::Rectangle<int> waveformArea;
    int hoverSample = -1;

    void drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area);
    void applyDrawPoints();
    float sampleToPhase(int sampleIndex) const;
    int phaseToSample(float phase) const;
    void importWavetable(bool multiCycle);
};
