#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"
#include "../NeuKnob.h"
#include "SynthDisplay.h"
#include "EnvelopeDisplay.h"
#include "FilterDisplay.h"
#include "WavetableEditor.h"

class PluginProcessor;

//==============================================================================
class OscDisplay : public juce::Component
{
public:
    OscDisplay() = default;

    void setWaveform(WaveformType wf)
    {
        if (waveform != wf) { waveform = wf; repaint(); }
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colour(0xff0a1820));
        g.fillRoundedRectangle(b, 4.0f);
        g.setColour(juce::Colour(0xff2a3d47));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        const float w  = b.getWidth();
        const float h  = b.getHeight();
        const float ox = b.getX();
        const float cy = b.getY() + h * 0.5f;
        const int   N  = static_cast<int>(w);

        juce::Path path;
        for (int i = 0; i < N; ++i)
        {
            const float t  = i / float(N - 1);
            const float ph = t * juce::MathConstants<float>::twoPi;
            float y = 0.0f;
            switch (waveform)
            {
                case WaveformType::Sine:     y = std::sin(ph); break;
                case WaveformType::Saw:      y = 1.0f - 2.0f * t; break;
                case WaveformType::Square:   y = t < 0.5f ? 1.0f : -1.0f; break;
                case WaveformType::Triangle: y = t < 0.5f ? (4.0f*t - 1.0f) : (3.0f - 4.0f*t); break;
                case WaveformType::Noise:    y = noiseTable[i % 64]; break;
                default: break;
            }
            const float px = ox + t * w;
            const float py = cy - y * h * 0.38f;
            if (i == 0) path.startNewSubPath(px, py);
            else        path.lineTo(px, py);
        }
        g.setColour(juce::Colour(0xff4fc3f7));
        g.strokePath(path, juce::PathStrokeType(1.5f));
    }

private:
    WaveformType waveform = WaveformType::Saw;
    const float noiseTable[64] = {
         0.2f,-0.5f, 0.8f,-0.3f, 0.6f,-0.9f, 0.1f, 0.7f,
        -0.4f, 0.9f,-0.7f, 0.3f,-0.1f, 0.5f,-0.8f, 0.4f,
         0.6f,-0.2f, 0.9f,-0.6f, 0.3f, 0.8f,-0.5f, 0.1f,
        -0.9f, 0.4f,-0.3f, 0.7f,-0.6f, 0.2f, 0.5f,-0.8f,
         0.1f, 0.6f,-0.4f, 0.9f,-0.2f, 0.5f,-0.7f, 0.3f,
         0.8f,-0.1f, 0.4f,-0.9f, 0.6f,-0.3f, 0.7f,-0.5f,
         0.2f,-0.6f, 0.9f,-0.4f, 0.1f, 0.8f,-0.7f, 0.3f,
        -0.5f, 0.6f,-0.2f, 0.9f,-0.8f, 0.4f,-0.1f, 0.7f
    };
};

//==============================================================================
class LFODisplay : public juce::Component, private juce::Timer
{
public:
    LFODisplay()  { startTimerHz(30); }
    ~LFODisplay() override { stopTimer(); }

    void timerCallback() override
    {
        phase = std::fmod(phase + 1.5f / 30.0f, 1.0f);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(2.0f);
        g.setColour(juce::Colour(0xff0a1820));
        g.fillRoundedRectangle(b, 4.0f);
        g.setColour(juce::Colour(0xff2a3d47));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        g.setColour(juce::Colour(0xff607d8b));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText("LFO", (int)b.getX(), (int)b.getY(), 28, (int)b.getHeight(),
                   juce::Justification::centred);

        auto wave = b.withTrimmedLeft(30.0f);
        const float w  = wave.getWidth();
        const float h  = wave.getHeight();
        const float ox = wave.getX();
        const float cy = wave.getY() + h * 0.5f;
        const int   N  = static_cast<int>(w);

        juce::Path path;
        for (int i = 0; i < N; ++i)
        {
            const float t  = i / float(N - 1);
            const float ph = std::fmod(phase + t * 2.0f, 1.0f);
            const float y  = std::sin(ph * juce::MathConstants<float>::twoPi);
            const float px = ox + t * w;
            const float py = cy - y * h * 0.38f;
            if (i == 0) path.startNewSubPath(px, py);
            else        path.lineTo(px, py);
        }
        g.setColour(juce::Colour(0xff80cbc4));
        g.strokePath(path, juce::PathStrokeType(1.5f));

        const float dt  = std::fmod(phase * 0.5f, 1.0f);
        const float dotX = ox + dt * w;
        const float dotY = cy - std::sin(phase * juce::MathConstants<float>::twoPi) * h * 0.38f;
        g.setColour(juce::Colour(0xffe0f7fa));
        g.fillEllipse(dotX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
    }

private:
    float phase = 0.0f;
};

//==============================================================================
class SynthPanel : public juce::Component
{
public:
    explicit SynthPanel(PluginProcessor& p);
    ~SynthPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    SynthEngine&     synthEngine;

    // Header
    juce::Label      modeLabel, waveformLabel;
    juce::ComboBox   modeSelector, waveformSelector;
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton loadPresetButton { "Load" };

    // Classic-mode visualisations
    OscDisplay   oscDisplay;
    LFODisplay   lfoDisplay;
    SynthDisplay synthDisplay;
    EnvelopeDisplay envelopeDisplay;
    FilterDisplay filterDisplay;

    // Section rects: set in resized(), used in paint()
    juce::Rectangle<int> oscSectionRect, unisonSectionRect, filterSectionRect, envSectionRect;
    juce::Rectangle<int> fmSectionRect, voiceSectionRect;
    juce::Rectangle<int> modeBadgeRect;

    // 8 Oscillator strips (first 3 active by default; +/- buttons adjust count)
    struct OscControls
    {
        juce::Label        sectionLabel;
        juce::ComboBox     typeSelector;
        NeuKnob    levelSlider;
        NeuKnob    detuneSlider;
        juce::ComboBox     waveformSelector;
        NeuKnob    wavePosSlider;
        juce::Label        levelLabel, detuneLabel, wavePosLabel;
        juce::TextButton   loadWTButton { "LOAD WT" };
        juce::TextButton   editWTButton { "EDIT WT" };
        juce::Label        wtFileLabel;
        // Wave shaping
        juce::ComboBox     shapeTypeSelector;
        NeuKnob    shapeAmtSlider;
        NeuKnob    selfOscSlider;
        NeuKnob    phaseDistSlider;
        juce::Label        shapeAmtLabel, selfOscLabel, phaseDistLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>    levelAttach, detuneAttach, wavePosAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeTypeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>    shapeAmtAttach, selfOscAttach, phaseDistAttach;
    };
    std::array<OscControls, 8> oscControls;
    juce::TextButton addOscButton { "+ OSC" };
    juce::TextButton removeOscButton { "- OSC" };

    // Envelope
    juce::Label  envSectionLabel;
    NeuKnob attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;

    // Filter
    juce::Label    filterSectionLabel;
    NeuKnob cutoffSlider, resonanceSlider;
    juce::Label    cutoffLabel, resonanceLabel;
    juce::Label    oversamplingLabel;
    juce::ComboBox oversamplingSelector;
    juce::Label    filterTypeLabel;
    juce::ComboBox filterTypeSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAttach;

    void setupSlider(juce::Slider& s, double min, double max, double value, double skew = 1.0);
    void setupLabel(juce::Label& l, const juce::String& text);
    void drawSection(juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const;
    void updateVisibility();

    // FM
    juce::Label    algorithmLabel;
    juce::ComboBox algorithmSelector;

    struct FMOpControls
    {
        juce::Label  sectionLabel;
        NeuKnob ratioSlider, levelSlider, feedbackSlider;
        NeuKnob attackSlider, decaySlider, sustainSlider, releaseSlider;
        juce::Label  ratioLabel, levelLabel, feedbackLabel;
        juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttach, levelAttach, feedbackAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    };
    std::array<FMOpControls, 4> fmOps;

    // Voice mode / portamento (header strip)
    juce::Label           voiceModeLabel, portamentoLabel;
    juce::ComboBox        voiceModeSelector;
    NeuKnob       portamentoSlider;
    juce::ToggleButton    portaAlwaysButton;
    juce::ToggleButton    mpeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   portamentoAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   portaAlwaysAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   mpeAttach;

    // Unison controls
    juce::Label        unisonVoicesLabel, unisonDetuneLabel, unisonWidthLabel;
    juce::ComboBox     unisonVoicesBox;
    NeuKnob    unisonDetuneSlider, unisonWidthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> unisonVoicesAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   unisonDetuneAttach, unisonWidthAttach;

    // Sub oscillator section
    juce::Rectangle<int> subNoiseSectionRect;
    juce::ToggleButton subOscEnableButton, noiseOscEnableButton;
    NeuKnob subOscLevelSlider, noiseOscLevelSlider, noiseOscColorSlider;
    juce::ComboBox subOscWaveSelector;
    juce::Label subOscLabel, noiseOscLabel;
    juce::Label subOscLevelLabel, noiseOscLevelLabel, noiseOscColorLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>  subOscEnableAttach, noiseOscEnableAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  subOscLevelAttach, noiseOscLevelAttach, noiseOscColorAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> subOscWaveAttach;

    // Unison spread mode
    juce::Label    unisonSpreadLabel;
    juce::ComboBox unisonSpreadSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> unisonSpreadAttach;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, resonanceAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmAttach;

    std::unique_ptr<juce::FileChooser> fileChooser;

    // Wavetable editors (one per osc strip, shown as overlays)
    std::array<std::unique_ptr<WavetableEditor>, 8> wavetableEditors;

    // Chord/Strum mode
    juce::Rectangle<int> chordSectionRect;
    juce::ToggleButton    chordEnableButton;
    juce::ComboBox        chordShapeSelector;
    NeuKnob               chordStrumSlider;
    juce::Label           chordShapeLabel, chordStrumLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   chordEnableAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> chordShapeAttach;
    std::unique_ptr<SliderAttachment>                                        chordStrumAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthPanel)
};
