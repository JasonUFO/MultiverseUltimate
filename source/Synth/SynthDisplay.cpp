#include "SynthDisplay.h"
#include "../CyberpunkTheme.h"
#include "../PluginProcessor.h"
#include "../CyberpunkTheme.h"

SynthDisplay::SynthDisplay() : fft(FFT_ORDER)
{
    startTimerHz(30);
}

SynthDisplay::~SynthDisplay()
{
    stopTimer();
}

void SynthDisplay::setProcessor(PluginProcessor* p)
{
    processor = p;
}

void SynthDisplay::timerCallback()
{
    if (processor == nullptr) return;

    float temp[256];
    int got = processor->pullDisplaySamples(temp, 256);
    for (int i = 0; i < got; ++i)
    {
        ring[ringWritePos] = temp[i];
        ringWritePos = (ringWritePos + 1) & (RING_SIZE - 1);
    }

    // Rebuild FFT from the latest FFT_SIZE samples in the ring
    const int startIdx = (ringWritePos - FFT_SIZE + RING_SIZE) & (RING_SIZE - 1);
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        const float w = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi
                                                  * i / float(FFT_SIZE - 1)));
        fftData[i] = ring[(startIdx + i) & (RING_SIZE - 1)] * w;
    }
    juce::zeromem(fftData + FFT_SIZE, sizeof(float) * FFT_SIZE);
    fft.performFrequencyOnlyForwardTransform(fftData, true);

    const int numBins = FFT_SIZE / 2;
    for (int i = 0; i < numBins; ++i)
    {
        const float dB   = 20.0f * std::log10(std::max(fftData[i] / float(FFT_SIZE), 1e-10f));
        const float norm = juce::jlimit(0.0f, 1.0f, (dB + 80.0f) / 80.0f);
        fftMag[i]  = norm > fftMag[i]  ? norm  : fftMag[i]  * 0.92f;
        fftPeak[i] = norm >= fftPeak[i] ? norm  : fftPeak[i] * 0.998f;
    }

    repaint();
}

void SynthDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Inset neumorphic frame (carved-in look)
    g.setColour(juce::Colour(0xff080810).withAlpha(0.9f));
    g.drawRoundedRectangle(bounds.translated(2.0f, 2.0f), 8.0f, 2.0f);
    g.setColour(juce::Colour(0xff303050).withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.translated(-1.0f, -1.0f), 8.0f, 1.5f);

    auto inner = bounds.reduced(2.0f);
    g.setColour(juce::Colour(0xff0D0D18));
    g.fillRoundedRectangle(inner, 6.0f);
    g.setColour(juce::Colour(0xff252535));
    g.drawRoundedRectangle(inner, 6.0f, 1.0f);
    inner = inner.reduced(1.0f);

    // Grid lines — 8% white
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    const float gx = inner.getX(), gy = inner.getY();
    const float gw = inner.getWidth(), gh = inner.getHeight();
    for (float t : {0.25f, 0.5f, 0.75f})
        g.drawHorizontalLine((int)(gy + t * gh), gx, gx + gw);

    // Vertical divider between scope and spectrum
    const float divX = gx + gw * 0.45f;
    g.drawVerticalLine((int)divX, gy, gy + gh);

    auto specArea  = inner;
    auto scopeArea = specArea.removeFromLeft(gw * 0.45f).reduced(4.0f, 4.0f);
    specArea       = specArea.reduced(4.0f, 4.0f);

    // ---- Oscilloscope (left) ----
    {
        constexpr int DISPLAY = 512;
        const int startIdx = (ringWritePos - DISPLAY + RING_SIZE) & (RING_SIZE - 1);
        const float w  = scopeArea.getWidth();
        const float h  = scopeArea.getHeight();
        const float ox = scopeArea.getX();
        const float cy = scopeArea.getCentreY();

        juce::Path wave;
        for (int i = 0; i < DISPLAY; ++i)
        {
            const float s  = ring[(startIdx + i) & (RING_SIZE - 1)];
            const float px = ox + (i / float(DISPLAY - 1)) * w;
            const float py = cy - s * h * 0.44f;
            if (i == 0) wave.startNewSubPath(px, py);
            else        wave.lineTo(px, py);
        }

        // Glow layer then crisp stroke
        g.setColour(CyberpunkTheme::accentBlue.withAlpha(0.18f));
        g.strokePath(wave, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(CyberpunkTheme::accentBlue);
        g.strokePath(wave, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        g.setColour(CyberpunkTheme::textMuted);
        g.setFont(juce::Font(8.0f));
        g.drawText("OSC", (int)scopeArea.getX(), (int)scopeArea.getY(), 24, 10,
                   juce::Justification::centredLeft, false);
    }

    // ---- Spectrum Analyzer (right) ----
    {
        const float sw   = specArea.getWidth();
        const float sh   = specArea.getHeight();
        const float sox  = specArea.getX();
        const float sbot = specArea.getBottom();
        const int   numBins = FFT_SIZE / 2;
        const float sr      = 44100.0f;
        const int   dispW   = (int)sw;

        juce::Path bars;
        for (int x = 0; x < dispW; ++x)
        {
            const float t    = float(x) / float(dispW - 1);
            const float freq = 20.0f * std::pow(22050.0f / 20.0f, t);
            const int   bin  = juce::jlimit(0, numBins - 1, (int)(freq * FFT_SIZE / sr));
            const float barH = fftMag[bin] * sh;
            if (barH >= 1.0f)
                bars.addRectangle(sox + float(x), sbot - barH, 1.5f, barH);
        }

        juce::ColourGradient grad(CyberpunkTheme::accentPurple, sox, sbot,
                                  CyberpunkTheme::accentBlue.brighter(0.4f), sox, specArea.getY(),
                                  false);
        g.setGradientFill(grad);
        g.fillPath(bars);

        // Peak hold — 1px per column
        g.setColour(CyberpunkTheme::accentBlue.brighter(0.6f));
        for (int x = 0; x < dispW; ++x)
        {
            if (fftPeak[(int)(float(x) / float(dispW - 1) * float(numBins - 1))] < 0.01f) continue;
            const float t    = float(x) / float(dispW - 1);
            const float freq = 20.0f * std::pow(22050.0f / 20.0f, t);
            const int   bin  = juce::jlimit(0, numBins - 1, (int)(freq * FFT_SIZE / sr));
            g.fillRect(sox + float(x), sbot - fftPeak[bin] * sh, 1.5f, 1.5f);
        }

        g.setColour(CyberpunkTheme::textMuted);
        g.setFont(juce::Font(8.0f));
        g.drawText("FFT", (int)specArea.getX(), (int)specArea.getY(), 22, 10,
                   juce::Justification::centredLeft, false);
    }
}
