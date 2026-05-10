#include "WavetableEditor.h"
#include "../MultiverseFlatTheme.h"
#include <cmath>

WavetableEditor::WavetableEditor(WavetableOscillator& osc)
    : oscillator(osc)
{
    // Frame selector
    frameLabel.setText("Frame:", juce::dontSendNotification);
    addAndMakeVisible(frameLabel);

    for (int i = 0; i < oscillator.getTableCount(); ++i)
        frameSelector.addItem("Frame " + juce::String(i + 1), i + 1);
    frameSelector.setSelectedId(1);
    frameSelector.addListener(this);
    addAndMakeVisible(frameSelector);

    // Draw tools
    toolLabel.setText("Tool:", juce::dontSendNotification);
    addAndMakeVisible(toolLabel);

    pencilBtn.setButtonText("Pencil");
    pencilBtn.setToggleState(true, juce::dontSendNotification);
    pencilBtn.setClickingTogglesState(true);
    pencilBtn.addListener(this);
    addAndMakeVisible(pencilBtn);

    lineBtn.setButtonText("Line");
    lineBtn.setClickingTogglesState(true);
    lineBtn.addListener(this);
    addAndMakeVisible(lineBtn);

    curveBtn.setButtonText("Curve");
    curveBtn.setClickingTogglesState(true);
    curveBtn.addListener(this);
    addAndMakeVisible(curveBtn);

    // Process menu
    processLabel.setText("Process:", juce::dontSendNotification);
    addAndMakeVisible(processLabel);

    normalizeBtn.setButtonText("Normalize");
    normalizeBtn.addListener(this);
    addAndMakeVisible(normalizeBtn);

    fadeBtn.setButtonText("Fade");
    fadeBtn.addListener(this);
    addAndMakeVisible(fadeBtn);

    reverseBtn.setButtonText("Reverse");
    reverseBtn.addListener(this);
    addAndMakeVisible(reverseBtn);

    fftBtn.setButtonText("FFT");
    fftBtn.addListener(this);
    addAndMakeVisible(fftBtn);

    clearBtn.setButtonText("Clear");
    clearBtn.addListener(this);
    addAndMakeVisible(clearBtn);

    // Formula generators
    formulaSineBtn.setButtonText("Sin");
    formulaSineBtn.addListener(this);
    addAndMakeVisible(formulaSineBtn);

    formulaSawBtn.setButtonText("Saw");
    formulaSawBtn.addListener(this);
    addAndMakeVisible(formulaSawBtn);

    formulaSquareBtn.setButtonText("Sqr");
    formulaSquareBtn.addListener(this);
    addAndMakeVisible(formulaSquareBtn);

    formulaTriBtn.setButtonText("Tri");
    formulaTriBtn.addListener(this);
    addAndMakeVisible(formulaTriBtn);

    // Import
    importLabel.setText("Import:", juce::dontSendNotification);
    addAndMakeVisible(importLabel);

    importBtn.setButtonText("Load WAV");
    importBtn.addListener(this);
    addAndMakeVisible(importBtn);

    importMultiBtn.setButtonText("Multi-Cycle");
    importMultiBtn.addListener(this);
    addAndMakeVisible(importMultiBtn);

    // Close
    closeBtn.setButtonText("Close");
    closeBtn.addListener(this);
    addAndMakeVisible(closeBtn);

    startTimer(30); // 30 Hz update for hover
}

WavetableEditor::~WavetableEditor()
{
    stopTimer();
}

void WavetableEditor::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep());

    // Draw waveform area background
    g.setColour(MultiverseFlatTheme::bgVoid());
    g.fillRect(waveformArea);

    // Draw waveform
    drawWaveform(g, waveformArea);

    // Draw hover indicator
    if (hoverSample >= 0 && hoverSample < oscillator.getTableSize())
    {
        int x = waveformArea.getX() + (hoverSample * waveformArea.getWidth()) / oscillator.getTableSize();
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawVerticalLine(x, static_cast<float>(waveformArea.getY()), static_cast<float>(waveformArea.getBottom()));
    }

    // Draw draw points
    if (drawPoints.size() > 1)
    {
        g.setColour(MultiverseFlatTheme::accent5());
        for (int i = 1; i < drawPoints.size(); ++i)
        {
            g.drawLine(drawPoints[i-1].x, drawPoints[i-1].y, drawPoints[i].x, drawPoints[i].y, 2.0f);
        }
    }

    // Border
    g.setColour(MultiverseFlatTheme::borderLight());
    g.drawRect(getLocalBounds(), 2);
}

void WavetableEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    int rowH = 30;

    // Top row: frame selector
    auto topRow = area.removeFromTop(rowH);
    frameLabel.setBounds(topRow.removeFromLeft(60));
    frameSelector.setBounds(topRow.removeFromLeft(120));

    closeBtn.setBounds(topRow.removeFromRight(80));

    area.removeFromTop(10);

    // Waveform area
    waveformArea = area.removeFromTop(area.getHeight() - 200);

    area.removeFromTop(10);

    // Tool row
    auto toolRow = area.removeFromTop(rowH);
    toolLabel.setBounds(toolRow.removeFromLeft(60));
    pencilBtn.setBounds(toolRow.removeFromLeft(80));
    lineBtn.setBounds(toolRow.removeFromLeft(80));
    curveBtn.setBounds(toolRow.removeFromLeft(80));

    area.removeFromTop(5);

    // Process row 1
    auto procRow1 = area.removeFromTop(rowH);
    processLabel.setBounds(procRow1.removeFromLeft(60));
    normalizeBtn.setBounds(procRow1.removeFromLeft(100));
    fadeBtn.setBounds(procRow1.removeFromLeft(80));
    reverseBtn.setBounds(procRow1.removeFromLeft(90));
    fftBtn.setBounds(procRow1.removeFromLeft(80));
    clearBtn.setBounds(procRow1.removeFromLeft(80));

    area.removeFromTop(5);

    // Formula row
    auto formulaRow = area.removeFromTop(rowH);
    juce::Label formulaLabel("Formula:", "Formula:");
    formulaLabel.setBounds(formulaRow.removeFromLeft(60));
    addAndMakeVisible(formulaLabel);
    formulaSineBtn.setBounds(formulaRow.removeFromLeft(60));
    formulaSawBtn.setBounds(formulaRow.removeFromLeft(60));
    formulaSquareBtn.setBounds(formulaRow.removeFromLeft(60));
    formulaTriBtn.setBounds(formulaRow.removeFromLeft(60));

    area.removeFromTop(5);

    // Import row
    auto importRow = area.removeFromTop(rowH);
    importLabel.setBounds(importRow.removeFromLeft(60));
    importBtn.setBounds(importRow.removeFromLeft(100));
    importMultiBtn.setBounds(importRow.removeFromLeft(120));
}

void WavetableEditor::buttonClicked(juce::Button* button)
{
    int frame = frameSelector.getSelectedId() - 1;

    if (button == &pencilBtn)
    {
        currentTool = 0;
        pencilBtn.setToggleState(true, juce::dontSendNotification);
        lineBtn.setToggleState(false, juce::dontSendNotification);
        curveBtn.setToggleState(false, juce::dontSendNotification);
    }
    else if (button == &lineBtn)
    {
        currentTool = 1;
        pencilBtn.setToggleState(false, juce::dontSendNotification);
        lineBtn.setToggleState(true, juce::dontSendNotification);
        curveBtn.setToggleState(false, juce::dontSendNotification);
    }
    else if (button == &curveBtn)
    {
        currentTool = 2;
        pencilBtn.setToggleState(false, juce::dontSendNotification);
        lineBtn.setToggleState(false, juce::dontSendNotification);
        curveBtn.setToggleState(true, juce::dontSendNotification);
    }
    else if (button == &normalizeBtn)
    {
        oscillator.normalizeFrame(frame);
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &fadeBtn)
    {
        oscillator.fadeFrame(frame, 1.0f, 0.0f);
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &reverseBtn)
    {
        oscillator.reverseFrame(frame);
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &fftBtn)
    {
        oscillator.processFFT(frame);
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &clearBtn)
    {
        oscillator.clearFrame(frame);
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &formulaSineBtn)
    {
        oscillator.generateFormula(frame, [](float ph) { return std::sin(ph * 2.0f * M_PI); });
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &formulaSawBtn)
    {
        oscillator.generateFormula(frame, [](float ph) { return 2.0f * ph - 1.0f; });
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &formulaSquareBtn)
    {
        oscillator.generateFormula(frame, [](float ph) { return ph < 0.5f ? 1.0f : -1.0f; });
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &formulaTriBtn)
    {
        oscillator.generateFormula(frame, [](float ph) { return 2.0f * std::abs(2.0f * ph - 1.0f) - 1.0f; });
        repaint(); if (onWavetableChanged) onWavetableChanged();
    }
    else if (button == &importBtn)
    {
        importWavetable(false);
    }
    else if (button == &importMultiBtn)
    {
        importWavetable(true);
    }
    else if (button == &closeBtn)
    {
        setVisible(false);
    }
}

void WavetableEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    repaint(); // Redraw waveform for new frame
}

void WavetableEditor::timerCallback()
{
    repaint(); // Update hover position
}

void WavetableEditor::mouseDown(const juce::MouseEvent& e)
{
    if (!waveformArea.contains(e.getPosition())) return;

    isDrawing = true;
    drawPoints.clear();

    float phase = static_cast<float>(e.x - waveformArea.getX()) / static_cast<float>(waveformArea.getWidth());
    float value = 1.0f - (static_cast<float>(e.y - waveformArea.getY()) / static_cast<float>(waveformArea.getHeight()));
    value = juce::jlimit(-1.0f, 1.0f, 2.0f * value - 1.0f);

    int sample = phaseToSample(phase);
    int frame = frameSelector.getSelectedId() - 1;

    if (currentTool == 1) // Line tool
    {
        lineStart = {phase, value};
    }
    else
    {
        oscillator.setSample(frame, sample, value);
        drawPoints.add({phase, value});
    }
    repaint();
}

void WavetableEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDrawing) return;
    if (!waveformArea.contains(e.getPosition())) return;

    float phase = static_cast<float>(e.x - waveformArea.getX()) / static_cast<float>(waveformArea.getWidth());
    float value = 1.0f - (static_cast<float>(e.y - waveformArea.getY()) / static_cast<float>(waveformArea.getHeight()));
    value = juce::jlimit(-1.0f, 1.0f, 2.0f * value - 1.0f);

    int frame = frameSelector.getSelectedId() - 1;

    if (currentTool == 1) // Line tool
    {
        // Draw line from start to current
        drawPoints.clear();
        drawPoints.add(lineStart);
        drawPoints.add({phase, value});
        repaint();
    }
    else if (currentTool == 2) // Curve tool (simple quadratic)
    {
        oscillator.setSample(frame, phaseToSample(phase), value);
        drawPoints.add({phase, value});
    }
    else // Pencil tool
    {
        oscillator.setSample(frame, phaseToSample(phase), value);
        drawPoints.add({phase, value});
    }
}

void WavetableEditor::mouseUp(const juce::MouseEvent& e)
{
    if (!isDrawing) return;
    isDrawing = false;

    if (currentTool == 1) // Line tool - apply the line
    {
        int frame = frameSelector.getSelectedId() - 1;
        float phaseA = drawPoints[0].x;
        float valueA = drawPoints[0].y;
        float phaseB = drawPoints[1].x;
        float valueB = drawPoints[1].y;

        int sampA = phaseToSample(phaseA);
        int sampB = phaseToSample(phaseB);
        if (sampA > sampB) std::swap(sampA, sampB);

        for (int s = sampA; s <= sampB; ++s)
        {
            float t = static_cast<float>(s - sampA) / static_cast<float>(sampB - sampA + 1);
            oscillator.setSample(frame, s, valueA + t * (valueB - valueA));
        }
    }

    applyDrawPoints();
    drawPoints.clear();
    repaint();
    if (onWavetableChanged) onWavetableChanged();
}

void WavetableEditor::drawWaveform(juce::Graphics& g, const juce::Rectangle<int>& area)
{
    int frame = frameSelector.getSelectedId() - 1;
    int tableSize = oscillator.getTableSize();

    g.setColour(MultiverseFlatTheme::accent1());
    juce::Path waveform;
    bool first = true;

    for (int i = 0; i < tableSize; i += 4) // Draw every 4th sample for performance
    {
        float sample = oscillator.getSample(frame, i);
        float x = area.getX() + (static_cast<float>(i) / static_cast<float>(tableSize)) * area.getWidth();
        float y = area.getCentreY() - sample * area.getHeight() / 2.5f;

        if (first)
        {
            waveform.startNewSubPath(x, y);
            first = false;
        }
        else
        {
            waveform.lineTo(x, y);
        }
    }
    waveform.closeSubPath();
    g.strokePath(waveform, juce::PathStrokeType(2.0f));
}

void WavetableEditor::applyDrawPoints()
{
    // Already applied in mouseDrag for pencil/curve
    // Line tool applied in mouseUp
}

float WavetableEditor::sampleToPhase(int sampleIndex) const
{
    return static_cast<float>(sampleIndex) / static_cast<float>(oscillator.getTableSize());
}

int WavetableEditor::phaseToSample(float phase) const
{
    return juce::jlimit(0, oscillator.getTableSize() - 1, static_cast<int>(phase * oscillator.getTableSize()));
}

void WavetableEditor::importWavetable(bool multiCycle)
{
    int frame = frameSelector.getSelectedId() - 1;
    fileChooser = std::make_unique<juce::FileChooser>(
        "Import WAV file",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff");
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, multiCycle, frame](const juce::FileChooser& fc)
        {
            auto f = fc.getResult();
            if (!f.existsAsFile()) return;
            juce::AudioFormatManager mgr;
            mgr.registerBasicFormats();
            std::unique_ptr<juce::AudioFormatReader> reader(mgr.createReaderFor(f));
            if (!reader) return;
            juce::AudioBuffer<float> buffer(1, static_cast<int>(reader->lengthInSamples));
            reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            if (multiCycle)
                oscillator.loadMultiCycleWavetable(buffer, 4);
            else
                oscillator.loadWavetable(buffer, 2048);
            repaint();
            if (onWavetableChanged) onWavetableChanged();
        });
}

void WavetableEditor::mouseMove(const juce::MouseEvent& e)
{
    if (waveformArea.contains(e.getPosition()))
    {
        hoverSample = phaseToSample(static_cast<float>(e.x - waveformArea.getX()) / static_cast<float>(waveformArea.getWidth()));
    }
    else
    {
        hoverSample = -1;
    }
}
