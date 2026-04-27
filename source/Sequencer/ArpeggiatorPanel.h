#pragma once
#include <JuceHeader.h>
#include "Arpeggiator.h"

class ArpStepButton : public juce::Component
{
public:
    std::function<void()> onToggle;

    void setActive      (bool a)  { active = a; repaint(); }
    bool isActive() const { return active; }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.5f);

        juce::Colour bg = active ? juce::Colour (0xff2a5faa) : juce::Colour (0xff1a1a2e);
        juce::Colour border = active ? juce::Colour (0xff4488dd) : juce::Colour (0xff333355);

        g.setColour (bg);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (border);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        active = !active;
        repaint();
        if (onToggle) onToggle();
    }

private:
    bool active = false;
};

class ArpeggiatorPanel : public juce::Component
{
public:
    explicit ArpeggiatorPanel (Arpeggiator& arp);
    ~ArpeggiatorPanel() override;

    void paint   (juce::Graphics& g) override;
    void resized ()                  override;

private:
    Arpeggiator& arpeggiator;
    int selectedStep = -1;

    juce::Label   titleLabel;
    juce::ToggleButton enableBtn { "Arp On" };

    juce::Label   modeLabel;
    juce::ComboBox modeBox;

    juce::Label   numStepsLabel;
    juce::ComboBox numStepsBox;

    juce::Label stepEditorTitle;
    std::array<ArpStepButton, ARP_MAX_STEPS> stepButtons;

    juce::Label noteOffsetLabel, octaveLabel, velocityLabel, gateLabel, tieLabel;
    juce::Slider noteOffsetSlider, octaveSlider, velocitySlider, gateSlider;
    juce::ToggleButton tieBtn { "Tie" };

    void selectStep (int step);
    void refreshStepEditor();
    void refreshStepGrid();
};