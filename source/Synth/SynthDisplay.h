#pragma once
#include <JuceHeader.h>

class PluginProcessor;

class SynthDisplay : public juce::Component, private juce::Timer
{
public:
    SynthDisplay();
    ~SynthDisplay() override;

    void setProcessor(PluginProcessor* p);
    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    PluginProcessor* processor = nullptr;

    static constexpr int RING_SIZE = 2048;
    float ring[RING_SIZE] {};
    int   ringWritePos    = 0;

    static constexpr int FFT_ORDER = 10;
    static constexpr int FFT_SIZE  = 1 << FFT_ORDER; // 1024
    juce::dsp::FFT fft { FFT_ORDER };
    float fftData[FFT_SIZE * 2] {};
    float fftMag[FFT_SIZE / 2]  {};
    float fftPeak[FFT_SIZE / 2] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthDisplay)
};
