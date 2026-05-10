#pragma once
#include <JuceHeader.h>
#include "SkinManager.h"

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

    // ── Skin-aware colour access ──────────────────────────────────────────
    // All colour getters now delegate to the current skin via SkinManager.
    // These static methods replace the old static const Colour members.

    static const Skin& skin() { return SkinManager::instance().current(); }

    // Backgrounds
    static juce::Colour bgVoid()      { return skin().bgVoid; }
    static juce::Colour bgBase()      { return skin().bgBase; }
    static juce::Colour bgRaised()    { return skin().bgRaised; }
    static juce::Colour bgDeep()      { return skin().bgDeep; }
    static juce::Colour bgHover()     { return skin().bgHover; }

    // Accents
    static juce::Colour accent1()     { return skin().accent1; }
    static juce::Colour accent2()     { return skin().accent2; }
    static juce::Colour accent3()     { return skin().accent3; }
    static juce::Colour accent4()     { return skin().accent4; }
    static juce::Colour accent5()     { return skin().accent5; }

    // Legacy aliases
    static juce::Colour accentCyan()   { return skin().accentCyan; }
    static juce::Colour accentPink()   { return skin().accentPink; }
    static juce::Colour accentPurple() { return skin().accentPurple; }
    static juce::Colour accentGreen()  { return skin().accentGreen; }
    static juce::Colour accentAmber()  { return skin().accentAmber; }
    static juce::Colour accentBlue()   { return skin().accent1; }   // alias
    static juce::Colour neonCyan()     { return skin().accent1; }   // alias
    static juce::Colour neonPink()     { return skin().accent2; }   // alias
    static juce::Colour neonPurple()   { return skin().accent3; }   // alias
    static juce::Colour neonGreen()    { return skin().accent4; }   // alias

    // Borders
    static juce::Colour borderLight()  { return skin().borderLight; }
    static juce::Colour borderActive() { return skin().borderActive; }

    // Text
    static juce::Colour textPrimary()   { return skin().textPrimary; }
    static juce::Colour textSecondary() { return skin().textSecondary; }
    static juce::Colour textMuted()     { return skin().textMuted; }
    static juce::Colour textLabel()     { return skin().textLabel; }

    // Glow / extras
    static juce::Colour glowStrong()   { return skin().glowStrong; }
    static juce::Colour glowSoft()      { return skin().glowSoft; }
    static juce::Colour keyboardWhite() { return skin().keyboardWhite; }
    static juce::Colour keyboardBlack() { return skin().keyboardBlack; }
    static juce::Colour keyboardHighlight() { return skin().keyboardHighlight; }
    static juce::Colour wheelTrack()    { return skin().wheelTrack; }
    static juce::Colour wheelFill()     { return skin().wheelFill; }

    // 3D depth properties
    static juce::Colour shadowDark()      { return skin().shadowDark; }
    static juce::Colour shadowLight()     { return skin().shadowLight; }
    static juce::Colour highlightSpec()   { return skin().highlightSpec; }
    static juce::Colour panelGradient1()  { return skin().panelGradient1; }
    static juce::Colour panelGradient2()  { return skin().panelGradient2; }
    static juce::Colour insetBg()         { return skin().insetBg; }
    static float bevelStrength()          { return skin().bevelStrength; }
    static float glowIntensity()         { return skin().glowIntensity; }

    // Tab colours
    static juce::Colour tabPrimaryBg()   { return skin().tabPrimaryBg; }
    static juce::Colour tabSecondaryBg() { return skin().tabSecondaryBg; }
    static juce::Colour tabActiveBg()     { return skin().tabActiveBg; }
    static juce::Colour tabActiveGlow()   { return skin().tabActiveGlow; }

    // Card/panel background (image-based with procedural overlay)
    static void drawCard (juce::Graphics&, juce::Rectangle<float>,
                          float cornerRadius, bool isActive = false,
                          juce::Colour fillColor = juce::Colour());

    // Content area background (image-based)
    static void drawContentBackground (juce::Graphics&, juce::Rectangle<float>);

    // 3D rendering primitives
    static void drawBevel (juce::Graphics& g, juce::Rectangle<float> bounds,
                           float cornerRadius, float strength = 1.0f);
    static void drawInset (juce::Graphics& g, juce::Rectangle<float> bounds,
                          float cornerRadius, float strength = 1.0f);
    static void drawGlow (juce::Graphics& g, juce::Rectangle<float> bounds,
                         juce::Colour colour, float radius, float alpha = 0.5f);
    static void draw3DSeparator (juce::Graphics& g, float y, float x1, float x2);
    static void drawGradientFill (juce::Graphics& g, juce::Rectangle<float> bounds);

    // Overlay backdrop
    static void drawOverlayBackdrop (juce::Graphics& g, juce::Rectangle<float> bounds);

    // Two-tier tab drawing methods
    static void drawPrimaryTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                      const juce::String& text, bool isActive, bool isHover);
    static void drawSecondaryTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                         const juce::String& text, bool isActive, bool isHover);
    static void drawSubTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                   const juce::String& text, bool isActive, bool isHover);

    // Section divider line (image-based)
    static void drawDivider (juce::Graphics& g, float y, float x1, float x2);

    // Image-based backgrounds
    static void drawHeaderBackground (juce::Graphics& g, juce::Rectangle<float> bounds);
    static void drawTabBarBackground (juce::Graphics& g, juce::Rectangle<float> bounds);

    // Font getters
    static juce::Font headerFont();
    static juce::Font labelFont();
    static juce::Font valueFont();
    static juce::Font titleFont();

    // Design system constants
    struct Metrics
    {
        static constexpr float fontHeader    = 12.0f;
        static constexpr float fontLabel      = 10.0f;
        static constexpr float fontValue      = 11.0f;
        static constexpr float fontTitle      = 14.0f;

        static constexpr int outerMargin      = 10;
        static constexpr int sectionPadding   = 10;
        static constexpr int sectionGap       = 10;
        static constexpr int smallGap          = 5;
        static constexpr int sectionHeaderH   = 20;

        static constexpr float dividerAlpha    = 0.6f;

        static constexpr int headerH          = 36;
        static constexpr int primaryTabH     = 36;   // was 28, now bigger
        static constexpr int secondaryTabH   = 28;   // was 24, now bigger
        static constexpr int tabBarH         = 64;   // was 52, now primaryTabH + secondaryTabH
        static constexpr int modBarH         = 160;
        static constexpr int modSubTabH      = 28;
    };

    // Re-apply all JUCE colours from current skin (call after skin change)
    void applySkinColours();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiverseFlatTheme)
};