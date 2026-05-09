#pragma once
#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"
#include "../PluginProcessor.h"

class FilterDisplay : public juce::Component, private juce::Timer
{
public:
    FilterDisplay() { startTimerHz(20); }
    ~FilterDisplay() override { stopTimer(); }

    void setProcessor(PluginProcessor* p) { processor = p; }

    void setFilterType(int type)  // 0=LP, 1=HP, 2=BP, 3=Notch
    {
        if (type != filterType)
        {
            filterType = type;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        g.setColour(MultiverseFlatTheme::bgDeep);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(MultiverseFlatTheme::borderLight);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        auto inner = bounds.reduced(4.0f, 4.0f);
        const float w  = inner.getWidth();
        const float h  = inner.getHeight();
        const float ox = inner.getX();
        const float oy = inner.getY();

        // Horizontal grid lines (quiet, subtle)
        g.setColour(MultiverseFlatTheme::textMuted.withAlpha(0.15f));
        for (float t : {0.25f, 0.5f, 0.75f})
            g.drawHorizontalLine(juce::roundToInt(oy + t * h), ox, ox + w);

        // Frequency labels
        g.setFont(MultiverseFlatTheme::labelFont());
        g.setColour(MultiverseFlatTheme::textMuted.withAlpha(0.5f));
        const char* freqLabels[] = {"100", "1k", "10k"};
        for (int i = 0; i < 3; ++i)
        {
            float t = (i + 1) / 4.0f;
            float x = ox + t * w;
            g.drawVerticalLine(juce::roundToInt(x), oy, oy + h);
            g.drawText(freqLabels[i], juce::Rectangle<float>(x - 12, oy + h - 10, 24, 10).toNearestInt(),
                        juce::Justification::centred, false);
        }

        // Draw filter response curve
        const int numPoints = static_cast<int>(w);
        if (numPoints < 2) return;

        juce::Path curvePath;
        juce::Path fillPath;
        curvePath.startNewSubPath(ox, oy + h);
        fillPath.startNewSubPath(ox, oy + h);

        const float sr = 44100.0f;
        for (int i = 0; i < numPoints; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
            const float freq = 20.0f * std::pow(22050.0f / 20.0f, t);
            const float response = computeResponse(freq, sr);
            const float normalized = juce::jlimit(0.0f, 1.0f, response);
            const float px = ox + t * w;
            const float py = oy + h - normalized * h;
            curvePath.lineTo(px, py);
            fillPath.lineTo(px, py);
        }

        fillPath.lineTo(ox + w, oy + h);
        fillPath.closeSubPath();

        // Fill under curve
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.12f));
        g.fillPath(fillPath);

        // Stroke the curve
        g.setColour(MultiverseFlatTheme::accentCyan);
        g.strokePath(curvePath, juce::PathStrokeType(1.8f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Filter type label
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.7f));
        g.setFont(MultiverseFlatTheme::headerFont());
        const char* typeNames[] = {"LP", "HP", "BP", "NOTCH"};
        if (filterType >= 0 && filterType <= 3)
            g.drawText(typeNames[filterType], juce::Rectangle<int>((int)ox + 4, (int)oy + 2, 30, 12),
                        juce::Justification::centredLeft, false);
    }

private:
    void timerCallback() override
    {
        if (!processor) return;

        // Read current filter parameters from APVTS
        auto* cutoffParam = processor->apvts.getRawParameterValue("filterCutoff");
        auto* resParam = processor->apvts.getRawParameterValue("filterResonance");
        auto* typeParam = processor->apvts.getRawParameterValue("filterType");

        if (cutoffParam)
        {
            float newCutoff = cutoffParam->load();
            float newRes = resParam ? resParam->load() : 0.707f;
            int newType = typeParam ? static_cast<int>(typeParam->load() + 0.5f) : 0;

            bool changed = std::abs(newCutoff - lastCutoff) > 1.0f
                        || std::abs(newRes - lastRes) > 0.001f
                        || newType != lastType;

            if (changed)
            {
                lastCutoff = newCutoff;
                lastRes = newRes;
                lastType = newType;
                filterType = newType;
                repaint();
            }
        }
    }

    float computeResponse(float freq, float sr) const
    {
        // Simplified biquad response approximation
        // Returns 0-1 where 1 = unity gain, 0 = full attenuation
        const float w0 = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        const float wc = 2.0f * juce::MathConstants<float>::pi * lastCutoff / sr;
        const float Q = lastRes > 0.01f ? 1.0f / lastRes : 10.0f;

        // Normalized frequency relative to cutoff
        const float ratio = freq / lastCutoff;

        float gain = 1.0f;

        switch (filterType)
        {
            case 0: // LP
                if (ratio < 0.1f)
                    gain = 1.0f;
                else if (ratio < 1.0f)
                    gain = 1.0f / std::sqrt(1.0f + std::pow(ratio, 2) / (Q * Q));
                else
                    gain = 1.0f / std::sqrt(1.0f + Q * Q * std::pow(ratio - 1.0f / ratio, 2));
                // Simplified: low-pass roll-off
                gain = 1.0f / std::sqrt(1.0f + std::pow(ratio, 4));
                if (Q > 0.7f)
                    gain *= (1.0f + (Q - 0.7f) * 0.5f * std::exp(-std::pow((ratio - 1.0f) * 3.0f, 2)));
                break;

            case 1: // HP
                if (ratio > 10.0f)
                    gain = 1.0f;
                else if (ratio > 1.0f)
                    gain = 1.0f / std::sqrt(1.0f + std::pow(1.0f / ratio, 4));
                else
                    gain = std::pow(ratio, 2) / std::sqrt(1.0f + std::pow(ratio, 4));
                if (Q > 0.7f && ratio > 0.5f && ratio < 2.0f)
                    gain *= (1.0f + (Q - 0.7f) * 0.5f);
                break;

            case 2: // BP
            {
                const float bw = 1.0f / Q;
                gain = 1.0f / std::sqrt(1.0f + std::pow((ratio - 1.0f / ratio) / bw, 2));
                break;
            }

            case 3: // Notch
            {
                const float bw = 1.0f / Q;
                gain = std::abs(ratio - 1.0f / ratio) / std::sqrt(std::pow(ratio - 1.0f / ratio, 2) + bw * bw);
                if (ratio < 0.01f || ratio > 100.0f) gain = 1.0f;
                break;
            }

            default:
                gain = 1.0f;
                break;
        }

        // Convert to dB then normalize
        float dB = 20.0f * std::log10(std::max(gain, 1e-6f));
        return juce::jlimit(0.0f, 1.0f, (dB + 30.0f) / 30.0f);
    }

    PluginProcessor* processor = nullptr;
    int filterType = 0;
    float lastCutoff = 20000.0f;
    float lastRes = 0.707f;
    int lastType = 0;
};