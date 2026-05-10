#pragma once
#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"

class PluginProcessor;

//==============================================================================
// Compact horizontal signal-flow strip:  OSC → SUB → FLT → SHP → ENV → AMP → OUT
// Active blocks highlighted in accentCyan; inactive blocks dimmed.
// Arrow lines connect blocks. Click on a block to scroll to that section.
class SignalFlowBar : public juce::Component, private juce::Timer
{
public:
    SignalFlowBar() { startTimerHz(10); }
    ~SignalFlowBar() override { stopTimer(); }

    void setProcessor(PluginProcessor* p) { processor = p; }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour(MultiverseFlatTheme::bgDeep());
        g.fillRoundedRectangle(b, 4.0f);
        g.setColour(MultiverseFlatTheme::borderLight().withAlpha(0.4f));
        g.drawRoundedRectangle(b, 4.0f, 1.0f);

        const int numBlocks = 7;
        const float blockW  = 52.0f;
        const float blockH  = 20.0f;
        const float gap      = 12.0f;
        const float totalW   = numBlocks * blockW + (numBlocks - 1) * gap;
        const float startX   = b.getX() + (b.getWidth() - totalW) * 0.5f;
        const float startY   = b.getY() + (b.getHeight() - blockH) * 0.5f;

        static const char* labels[] = { "OSC", "SUB", "FLT", "SHP", "ENV", "AMP", "OUT" };

        for (int i = 0; i < numBlocks; ++i)
        {
            const float x = startX + i * (blockW + gap);
            const float y = startY;
            const bool active = activeFlags[i];

            auto blockRect = juce::Rectangle<float>(x, y, blockW, blockH);
            if (active)
            {
                g.setColour(MultiverseFlatTheme::accentCyan().withAlpha(0.18f));
                g.fillRoundedRectangle(blockRect, 3.0f);
                g.setColour(MultiverseFlatTheme::accentCyan().withAlpha(0.6f));
                g.drawRoundedRectangle(blockRect, 3.0f, 1.0f);
                g.setColour(MultiverseFlatTheme::accentCyan());
            }
            else
            {
                g.setColour(MultiverseFlatTheme::bgRaised());
                g.fillRoundedRectangle(blockRect, 3.0f);
                g.setColour(MultiverseFlatTheme::borderLight().withAlpha(0.5f));
                g.drawRoundedRectangle(blockRect, 3.0f, 1.0f);
                g.setColour(MultiverseFlatTheme::textMuted());
            }

            g.setFont(MultiverseFlatTheme::headerFont());
            g.drawText(labels[i], blockRect.toNearestInt(),
                       juce::Justification::centred, false);

            // Arrow between blocks
            if (i < numBlocks - 1)
            {
                const float arrowX = x + blockW + 2.0f;
                const float arrowY = y + blockH * 0.5f;
                const float arrowEnd = arrowX + gap - 4.0f;
                g.setColour(active ? MultiverseFlatTheme::accentCyan().withAlpha(0.5f)
                                   : MultiverseFlatTheme::borderLight().withAlpha(0.3f));
                g.drawLine(arrowX, arrowY, arrowEnd, arrowY, 1.5f);
                g.drawLine(arrowEnd, arrowY, arrowEnd - 3.0f, arrowY - 3.0f, 1.5f);
                g.drawLine(arrowEnd, arrowY, arrowEnd - 3.0f, arrowY + 3.0f, 1.5f);
            }
        }
    }

    int clickedBlock = -1;

    void mouseDown(const juce::MouseEvent& e) override
    {
        const int numBlocks = 7;
        const float blockW  = 52.0f;
        const float gap      = 12.0f;
        const float totalW   = numBlocks * blockW + (numBlocks - 1) * gap;
        const float startX   = (getWidth() - totalW) * 0.5f;
        const float startY   = (getHeight() - 20.0f) * 0.5f;

        clickedBlock = -1;
        for (int i = 0; i < numBlocks; ++i)
        {
            const float x = startX + i * (blockW + gap);
            auto blockRect = juce::Rectangle<float>(x, startY, blockW, 20.0f);
            if (blockRect.contains(e.position.toFloat()))
            {
                clickedBlock = i;
                repaint();
                if (onBlockClicked)
                    onBlockClicked(i);
                break;
            }
        }
    }

    std::function<void(int blockIndex)> onBlockClicked;

private:
    PluginProcessor* processor = nullptr;

    // 0=OSC, 1=SUB, 2=FLT, 3=SHP, 4=ENV, 5=AMP, 6=OUT
    bool activeFlags[7] = { true, false, true, false, true, true, true };

    void timerCallback() override
    {
        if (!processor) return;

        auto& apvts = processor->apvts;

        // OSC: always active in Classic mode (bar hidden in FM)
        activeFlags[0] = true;

        // SUB: active if sub or noise oscillator enabled
        const bool subOn  = *apvts.getRawParameterValue("subOscEnable") > 0.5f;
        const bool noiseOn = *apvts.getRawParameterValue("noiseOscEnable") > 0.5f;
        activeFlags[1] = subOn || noiseOn;

        // FLT: always in signal path
        activeFlags[2] = true;

        // SHP: active if any osc has wave shaping enabled (ShapeType > Off)
        bool hasShape = false;
        for (int i = 0; i < 8; ++i)
        {
            auto* p = apvts.getRawParameterValue("osc" + juce::String(i + 1) + "ShapeType");
            if (p && static_cast<int>(*p) > 0) { hasShape = true; break; }
        }
        activeFlags[3] = hasShape;

        // ENV, AMP, OUT: always in signal path
        activeFlags[4] = true;
        activeFlags[5] = true;
        activeFlags[6] = true;

        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalFlowBar)
};