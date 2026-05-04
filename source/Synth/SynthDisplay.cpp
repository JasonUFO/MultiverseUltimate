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

    // Pitch detection: find peak FFT bin, convert to MIDI note + cents
    {
        const float sr = (processor != nullptr) ? static_cast<float>(processor->getSampleRate()) : 44100.0f;
        const int numBins = FFT_SIZE / 2;
        int   peakBin = 2;
        float peakVal = 0.0f;
        for (int i = 2; i < numBins / 2; ++i)  // only search up to Nyquist/2 for fundamentals
        {
            if (fftData[i] > peakVal) { peakVal = fftData[i]; peakBin = i; }
        }

        if (peakVal > 0.001f * float(FFT_SIZE))  // threshold: meaningful signal
        {
            // Parabolic interpolation for sub-bin frequency accuracy
            const float alpha = (peakBin > 0)          ? fftData[peakBin - 1] : 0.0f;
            const float beta  =                           fftData[peakBin];
            const float gamma = (peakBin < numBins - 1) ? fftData[peakBin + 1] : 0.0f;
            const float denom = alpha - 2.0f * beta + gamma;
            const float refinedBin = (std::abs(denom) > 1e-10f)
                ? float(peakBin) + 0.5f * (alpha - gamma) / denom
                : float(peakBin);

            tunerHz = refinedBin * sr / float(FFT_SIZE);
            if (tunerHz > 20.0f && tunerHz < 8000.0f)
            {
                const float exactNote = 69.0f + 12.0f * std::log2(tunerHz / 440.0f);
                tunerNote  = juce::roundToInt(exactNote);
                tunerCents = (exactNote - float(tunerNote)) * 100.0f;
            }
            else { tunerNote = -1; }
        }
        else { tunerNote = -1; tunerHz = 0.0f; }
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

    // ---- Tuner strip (bottom of scope area) ----
    {
        static const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        const float tunerH = 16.0f;
        auto tunerRect = scopeArea.removeFromBottom(tunerH);

        if (tunerNote >= 0)
        {
            const int octave = tunerNote / 12 - 1;
            const char* noteName = noteNames[tunerNote % 12];
            const bool inTune = std::abs(tunerCents) < 5.0f;

            // Bar background
            const float barW  = tunerRect.getWidth() * 0.55f;
            const float barX  = tunerRect.getX() + 26.0f;
            const float barMid = barX + barW * 0.5f;
            const float barY  = tunerRect.getCentreY() - 2.5f;
            g.setColour(juce::Colour(0xff1a1a2a));
            g.fillRect(barX, barY, barW, 5.0f);

            // Cents fill
            const float centOff = juce::jlimit(-50.0f, 50.0f, tunerCents);
            const float indicW  = barW * 0.5f * std::abs(centOff) / 50.0f;
            g.setColour(inTune ? CyberpunkTheme::neonGreen : CyberpunkTheme::accentAmber);
            if (centOff < 0.0f)
                g.fillRect(barMid - indicW, barY, indicW, 5.0f);
            else
                g.fillRect(barMid, barY, indicW, 5.0f);

            // Centre tick
            g.setColour(CyberpunkTheme::textSecondary);
            g.fillRect(barMid - 0.5f, barY - 1.0f, 1.0f, 7.0f);

            // Note label
            g.setColour(inTune ? CyberpunkTheme::neonGreen : CyberpunkTheme::textPrimary);
            g.setFont(juce::FontOptions(9.0f).withStyle("Bold"));
            g.drawText(juce::String(noteName) + juce::String(octave),
                       (int)tunerRect.getX(), (int)tunerRect.getY(), 26, (int)tunerH,
                       juce::Justification::centredLeft, false);

            // Hz readout
            g.setColour(CyberpunkTheme::textMuted);
            g.setFont(juce::FontOptions(7.5f));
            g.drawText(juce::String((int)tunerHz) + "Hz",
                       (int)(barX + barW + 2.0f), (int)tunerRect.getY(), 36, (int)tunerH,
                       juce::Justification::centredLeft, false);
        }
        else
        {
            // No signal
            g.setColour(CyberpunkTheme::textMuted.withAlpha(0.35f));
            g.setFont(juce::FontOptions(7.5f));
            g.drawText("---", (int)tunerRect.getX(), (int)tunerRect.getY(),
                       (int)tunerRect.getWidth(), (int)tunerH,
                       juce::Justification::centredLeft, false);
        }
    }

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
