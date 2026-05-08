#pragma once
#include <JuceHeader.h>

class MultiverseFlatTheme : public juce::LookAndFeel_V4
{
public:
    MultiverseFlatTheme();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool highlight, bool down) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlight, bool down) override;

    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool highlight, bool down) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool down,
                       int bx, int by, int bw, int bh, juce::ComboBox&) override;

    void drawTabButton (juce::TabBarButton&, juce::Graphics&,
                        bool mouseOver, bool mouseDown) override;

    void drawTabAreaBehindFrontButton (juce::TabbedButtonBar&, juce::Graphics&,
                                       int w, int h) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;

    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>&,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcut,
                            const juce::Drawable* icon, const juce::Colour*) override;

    void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int w, int h,
                        bool isVertical, int thumbPos, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

    // Palette — accessible from panels
    static const juce::Colour bgVoid;
    static const juce::Colour bgBase;
    static const juce::Colour bgRaised;
    static const juce::Colour bgDeep;
    static const juce::Colour bgHover;

    // Accent colors
    static const juce::Colour accentCyan;
    static const juce::Colour accentPink;
    static const juce::Colour accentPurple;
    static const juce::Colour accentGreen;
    static const juce::Colour accentAmber;

    // Legacy aliases for backward compatibility
    static const juce::Colour accentBlue;    // alias for accentCyan
    static const juce::Colour neonCyan;     // alias for accentCyan
    static const juce::Colour neonPink;     // alias for accentPink
    static const juce::Colour neonPurple;   // alias for accentPurple
    static const juce::Colour neonGreen;    // alias for accentGreen

    // Borders
    static const juce::Colour borderLight;
    static const juce::Colour borderActive;

    // Text
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour textMuted;
    static const juce::Colour textLabel;

    // Flat card helper (replaces drawNeumorphicRect)
    static void drawCard (juce::Graphics&, juce::Rectangle<float>,
                          float cornerRadius, bool isActive = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiverseFlatTheme)
};