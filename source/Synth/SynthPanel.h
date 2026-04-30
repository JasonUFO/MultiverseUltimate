#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"
#include "../MidiLearnSlider.h"

class PluginProcessor;

//==============================================================================
class SynthLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId,    juce::Colour(0xff4fc3f7));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff1a2c36));
        setColour(juce::Slider::textBoxTextColourId,         juce::Colour(0xffb0bec5));
        setColour(juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId,                 juce::Colour(0xff90a4ae));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override
    {
        const float r  = juce::jmin(width * 0.5f, height * 0.5f) - 4.0f;
        const float cx = x + width  * 0.5f;
        const float cy = y + height * 0.5f;

        juce::ColourGradient grad(juce::Colour(0xff37474f), cx, cy - r,
                                  juce::Colour(0xff162028), cx, cy + r, false);
        g.setGradientFill(grad);
        g.fillEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f);

        g.setColour(juce::Colour(0xff546e7a));
        g.drawEllipse(cx - r, cy - r, r * 2.0f, r * 2.0f, 1.0f);

        const float tr = r - 6.0f;
        const float tw = 4.0f;

        juce::Path track;
        track.addCentredArc(cx, cy, tr, tr, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff1a2e3a));
        g.strokePath(track, juce::PathStrokeType(tw, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arc;
        arc.addCentredArc(cx, cy, tr, tr, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(juce::Colour(0xff4fc3f7));
        g.strokePath(arc, juce::PathStrokeType(tw, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        juce::Path ptr;
        ptr.startNewSubPath(0.0f, -(r - 6.0f));
        ptr.lineTo(0.0f, -(r - 6.0f) * 0.45f);
        ptr.applyTransform(juce::AffineTransform::rotation(toAngle).translated(cx, cy));
        g.setColour(juce::Colours::white);
        g.strokePath(ptr, juce::PathStrokeType(2.0f));
    }
};

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
    ~SynthPanel() override { setLookAndFeel(nullptr); }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    SynthEngine&     synthEngine;

    SynthLookAndFeel synthLookAndFeel;

    // Header
    juce::Label      modeLabel, waveformLabel;
    juce::ComboBox   modeSelector, waveformSelector;
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton loadPresetButton { "Load" };

    // Classic-mode visualisations
    OscDisplay oscDisplay;
    LFODisplay lfoDisplay;

    // Section rects: set in resized(), used in paint()
    juce::Rectangle<int> oscSectionRect, filterSectionRect, envSectionRect;
    juce::Rectangle<int> modeBadgeRect;

    // 3 Oscillator strips
    struct OscControls
    {
        juce::Label        sectionLabel;
        juce::ComboBox     typeSelector;
        MidiLearnSlider    levelSlider;
        MidiLearnSlider    detuneSlider;
        juce::ComboBox     waveformSelector;
        MidiLearnSlider    wavePosSlider;
        juce::Label        levelLabel, detuneLabel, wavePosLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>    levelAttach, detuneAttach, wavePosAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttach;
    };
    std::array<OscControls, 3> oscControls;

    // Envelope
    juce::Label  envSectionLabel;
    MidiLearnSlider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;

    // Filter
    juce::Label    filterSectionLabel;
    MidiLearnSlider cutoffSlider, resonanceSlider;
    juce::Label    cutoffLabel, resonanceLabel;
    juce::Label    oversamplingLabel;
    juce::ComboBox oversamplingSelector;

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
        MidiLearnSlider ratioSlider, levelSlider, feedbackSlider;
        MidiLearnSlider attackSlider, decaySlider, sustainSlider, releaseSlider;
        juce::Label  ratioLabel, levelLabel, feedbackLabel;
        juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttach, levelAttach, feedbackAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    };
    std::array<FMOpControls, 4> fmOps;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<SliderAttachment> cutoffAttach, resonanceAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmAttach;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthPanel)
};
