#pragma once
#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"

class EnvelopeDisplay : public juce::Component, private juce::Timer
{
public:
    EnvelopeDisplay() { startTimerHz (15); }
    ~EnvelopeDisplay() override { stopTimer(); }

    void setSliders (juce::Slider* a, juce::Slider* d, juce::Slider* s, juce::Slider* r)
    {
        attackSlider  = a;
        decaySlider   = d;
        sustainSlider = s;
        releaseSlider = r;
    }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);

        g.setColour (MultiverseFlatTheme::bgDeep);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (MultiverseFlatTheme::borderLight);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        auto inner = bounds.reduced (6.0f);
        const float w = inner.getWidth();
        const float h = inner.getHeight();
        const float ox = inner.getX();
        const float oy = inner.getY();

        // Centre line (sustain level reference)
        g.setColour (MultiverseFlatTheme::textMuted.withAlpha (0.3f));
        g.drawHorizontalLine (juce::roundToInt (oy + h * 0.5f), ox, ox + w);

        // Map ADSR to normalized time
        const float aNorm = attack  * 0.25f;
        const float dNorm = decay   * 0.20f;
        const float sHold = 0.25f;
        const float rNorm = release * 0.30f;
        const float total = aNorm + dNorm + sHold + rNorm;
        if (total < 0.001f) return;

        const float scale = w / total;

        // Build the ADSR path
        juce::Path curve;
        curve.startNewSubPath (ox, oy + h);

        // Attack
        const float aX = ox + aNorm * scale;
        curve.lineTo (aX, oy);

        // Decay
        const float dX = aX + dNorm * scale;
        const float sY = oy + h * (1.0f - sustain);
        curve.lineTo (dX, sY);

        // Sustain hold
        const float sX = dX + sHold * scale;
        curve.lineTo (sX, sY);

        // Release
        const float rX = sX + rNorm * scale;
        curve.lineTo (rX, oy + h);

        // Fill under curve
        juce::Path fill = curve;
        fill.lineTo (rX, oy + h);
        fill.lineTo (ox, oy + h);
        fill.closeSubPath();

        g.setColour (MultiverseFlatTheme::accentCyan.withAlpha (0.12f));
        g.fillPath (fill);

        g.setColour (MultiverseFlatTheme::accentCyan);
        g.strokePath (curve, juce::PathStrokeType (1.8f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Segment labels
        g.setFont (MultiverseFlatTheme::labelFont());
        g.setColour (MultiverseFlatTheme::textMuted);

        const float labelY = oy + h + 2.0f;
        if (aNorm > 0.02f)
            g.drawText ("A", juce::Rectangle<float> (ox, labelY, aNorm * scale, 12.0f).toNearestInt(),
                        juce::Justification::centred, false);
        if (dNorm > 0.02f)
            g.drawText ("D", juce::Rectangle<float> (aX, labelY, dNorm * scale, 12.0f).toNearestInt(),
                        juce::Justification::centred, false);
        g.drawText ("S", juce::Rectangle<float> (dX, labelY, sHold * scale, 12.0f).toNearestInt(),
                    juce::Justification::centred, false);
        if (rNorm > 0.02f)
            g.drawText ("R", juce::Rectangle<float> (sX, labelY, rNorm * scale, 12.0f).toNearestInt(),
                        juce::Justification::centred, false);
    }

private:
    void timerCallback() override
    {
        if (attackSlider)  attack  = (float) attackSlider->getValue();
        if (decaySlider)   decay   = (float) decaySlider->getValue();
        if (sustainSlider) sustain = (float) sustainSlider->getValue();
        if (releaseSlider) release = (float) releaseSlider->getValue();
        repaint();
    }

    juce::Slider* attackSlider  = nullptr;
    juce::Slider* decaySlider   = nullptr;
    juce::Slider* sustainSlider = nullptr;
    juce::Slider* releaseSlider = nullptr;

    float attack  = 0.01f;
    float decay   = 0.1f;
    float sustain = 0.7f;
    float release = 0.3f;
};