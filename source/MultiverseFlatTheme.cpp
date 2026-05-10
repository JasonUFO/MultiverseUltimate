#include "MultiverseFlatTheme.h"
#include "Assets/AssetManager.h"

//==============================================================================
// Constructor — applies current skin colours to JUCE LookAndFeel

MultiverseFlatTheme::MultiverseFlatTheme()
{
    applySkinColours();
}

void MultiverseFlatTheme::applySkinColours()
{
    const Skin& s = skin();

    // Window / panel backgrounds
    setColour (juce::ResizableWindow::backgroundColourId, s.bgBase);
    setColour (juce::DocumentWindow::backgroundColourId,  s.bgBase);

    // Labels
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,       s.textPrimary);
    setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Label::textWhenEditingColourId, s.textPrimary);

    // Sliders
    setColour (juce::Slider::backgroundColourId,         s.bgDeep);
    setColour (juce::Slider::thumbColourId,              s.bgRaised);
    setColour (juce::Slider::trackColourId,              s.accent1);
    setColour (juce::Slider::rotarySliderFillColourId,   s.accent1);
    setColour (juce::Slider::rotarySliderOutlineColourId, s.borderLight);
    setColour (juce::Slider::textBoxTextColourId,        s.textSecondary);
    setColour (juce::Slider::textBoxBackgroundColourId,  s.bgDeep);
    setColour (juce::Slider::textBoxOutlineColourId,     juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,   s.accent1.withAlpha (0.3f));

    // TextButton
    setColour (juce::TextButton::buttonColourId,   s.bgRaised);
    setColour (juce::TextButton::buttonOnColourId, s.bgDeep);
    setColour (juce::TextButton::textColourOffId,  s.textSecondary);
    setColour (juce::TextButton::textColourOnId,   s.accent1);

    // ToggleButton
    setColour (juce::ToggleButton::textColourId,        s.textSecondary);
    setColour (juce::ToggleButton::tickColourId,        s.accent1);
    setColour (juce::ToggleButton::tickDisabledColourId, s.textMuted);

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,    s.bgRaised);
    setColour (juce::ComboBox::textColourId,          s.textPrimary);
    setColour (juce::ComboBox::outlineColourId,       juce::Colours::transparentBlack);
    setColour (juce::ComboBox::buttonColourId,        s.bgRaised);
    setColour (juce::ComboBox::arrowColourId,         s.textSecondary);
    setColour (juce::ComboBox::focusedOutlineColourId, s.accent1);

    // PopupMenu
    setColour (juce::PopupMenu::backgroundColourId,           s.bgRaised);
    setColour (juce::PopupMenu::textColourId,                 s.textPrimary);
    setColour (juce::PopupMenu::headerTextColourId,           s.textSecondary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, s.accent1.withAlpha (0.15f));
    setColour (juce::PopupMenu::highlightedTextColourId,       s.textPrimary);

    // Tabs
    setColour (juce::TabbedButtonBar::tabTextColourId,    s.textSecondary);
    setColour (juce::TabbedButtonBar::frontTextColourId,  s.accent1);
    setColour (juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::TabbedComponent::backgroundColourId, s.bgBase);
    setColour (juce::TabbedComponent::outlineColourId,    juce::Colours::transparentBlack);

    // TextEditor
    setColour (juce::TextEditor::backgroundColourId,      s.bgDeep);
    setColour (juce::TextEditor::textColourId,            s.textPrimary);
    setColour (juce::TextEditor::highlightColourId,       s.accent1.withAlpha (0.3f));
    setColour (juce::TextEditor::highlightedTextColourId,  s.textPrimary);
    setColour (juce::TextEditor::outlineColourId,         juce::Colours::transparentBlack);
    setColour (juce::TextEditor::focusedOutlineColourId,  s.accent1.withAlpha (0.5f));

    // ListBox
    setColour (juce::ListBox::backgroundColourId, s.bgDeep);
    setColour (juce::ListBox::textColourId,       s.textPrimary);
    setColour (juce::ListBox::outlineColourId,    juce::Colours::transparentBlack);

    // ScrollBar
    setColour (juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId,      s.accent1.withAlpha (0.4f));
    setColour (juce::ScrollBar::trackColourId,      s.bgDeep);

    // GroupComponent
    setColour (juce::GroupComponent::textColourId,    s.textLabel);
    setColour (juce::GroupComponent::outlineColourId, s.borderLight);

    // Tooltip
    setColour (juce::TooltipWindow::backgroundColourId, s.bgRaised);
    setColour (juce::TooltipWindow::textColourId,       s.textPrimary);
    setColour (juce::TooltipWindow::outlineColourId,    s.borderLight);

    // TableHeader
    setColour (juce::TableHeaderComponent::backgroundColourId,      s.bgRaised);
    setColour (juce::TableHeaderComponent::textColourId,            s.textSecondary);
    setColour (juce::TableHeaderComponent::outlineColourId,         s.borderLight);
    setColour (juce::TableHeaderComponent::highlightColourId,       s.accent1.withAlpha (0.12f));
}

//==============================================================================
void MultiverseFlatTheme::drawCard (juce::Graphics& g,
                                     juce::Rectangle<float> bounds,
                                     float cornerRadius, bool isActive,
                                     juce::Colour fillColor)
{
    const Skin& s = skin();
    juce::Colour fill = fillColor.isTransparent() ? s.bgRaised : fillColor;

    if (isActive)
        drawGlow (g, bounds, s.accent1, bounds.getWidth() * 0.5f, 0.12f);

    auto gradient = juce::ColourGradient (fill.brighter (0.03f), bounds.getX(), bounds.getY(),
                                          fill.darker (0.06f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRoundedRectangle (bounds, cornerRadius);
    drawBevel (g, bounds, cornerRadius);

    if (isActive)
    {
        g.setColour (s.accent1.withAlpha (0.5f * glowIntensity()));
        g.drawRoundedRectangle (bounds.expanded (1.0f).reduced (0.5f), cornerRadius + 1.0f, 1.0f);
    }

    g.setColour (isActive ? s.borderActive : s.borderLight);
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, 1.0f);
}

//==============================================================================

void MultiverseFlatTheme::drawContentBackground (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const Skin& s = skin();
    g.setColour (s.bgBase);
    g.fillRect (bounds);
    drawInset (g, bounds, 0.0f);
}

//==============================================================================
void MultiverseFlatTheme::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int w, int h,
                                             float sliderPos,
                                             float startAngle, float endAngle,
                                             juce::Slider& slider)
{
    const Skin& s = skin();
    const float radius = juce::jmin (w / 2.0f, h / 2.0f) - 4.0f;
    if (radius < 4.0f) return;

    const float cx = x + w / 2.0f;
    const float cy = y + h / 2.0f;

    // ── Draw knob image ───────────────────────────────────────────
    auto& assets = AssetManager::instance();
    const bool useSmall = (std::min (w, h) < 60);
    juce::Image knobImg = useSmall ? assets.getKnobSmall() : assets.getKnobLarge();

    if (knobImg.isValid())
    {
        const float knobW = static_cast<float> (knobImg.getWidth());
        const float knobH = static_cast<float> (knobImg.getHeight());

        // Knob visual center: the image has shadow below the knob circle.
        // Large knob (420x484): center ~210px from top, visual center at ~210
        // Small knob (178x207): center ~89px from top, visual center at ~89
        const float centerYOffset = useSmall ? AssetManager::KNOB_SMALL_CENTER_Y_OFFSET
                                              : AssetManager::KNOB_LARGE_CENTER_Y_OFFSET;
        const float imgCenterX = knobW / 2.0f;
        const float imgCenterY = knobH / 2.0f - centerYOffset;

        // Scale the knob to fit the available space
        const float scale = std::min (static_cast<float> (w), static_cast<float> (h))
                            / (useSmall ? 178.0f : 420.0f) * 0.85f;

        // Rotation angle from slider position
        const float angle = startAngle + sliderPos * (endAngle - startAngle);

        // Build transform: translate to center, scale, rotate around image center
        auto transform = juce::AffineTransform::translation (cx, cy)
                           .scaled (scale)
                           .rotated (angle)
                           .translated (-imgCenterX, -imgCenterY);

        g.drawImageTransformed (knobImg, transform, true);
    }
    else
    {
        // Fallback: draw circle if image not available
        g.setColour (s.bgDeep);
        g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (s.borderLight);
        g.drawEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.0f);
    }

    // ── Procedural accent arc on top (skin-aware) ──────────────
    const float arcRadius  = radius * 0.85f;
    const float trackWidth = 2.0f;

    // Arc groove (track)
    {
        juce::Path groove;
        groove.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f,
                               startAngle, endAngle, true);
        g.setColour (s.bgDeep.withAlpha (0.4f));
        g.strokePath (groove, juce::PathStrokeType (trackWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Fill arc + glow
    const juce::Colour arcColour = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);
    if (sliderPos > 0.001f)
    {
        juce::Path fillArc;
        fillArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f,
                                startAngle, currentAngle, true);

        for (int i = 3; i >= 1; --i)
        {
            g.setColour (arcColour.withAlpha (0.06f / (float)i));
            g.strokePath (fillArc, juce::PathStrokeType (trackWidth + i * 3.0f,
                juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        g.setColour (arcColour);
        g.strokePath (fillArc, juce::PathStrokeType (trackWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float tipX = cx + arcRadius * std::sin (currentAngle);
        const float tipY = cy - arcRadius * std::cos (currentAngle);

        // 3D bloom layers — outer glow, mid glow, inner dot, specular core
        g.setColour (arcColour.withAlpha (0.06f));
        g.fillEllipse (tipX - 14.0f, tipY - 14.0f, 28.0f, 28.0f);
        g.setColour (arcColour.withAlpha (0.15f));
        g.fillEllipse (tipX - 9.0f, tipY - 9.0f, 18.0f, 18.0f);
        g.setColour (arcColour.withAlpha (0.35f));
        g.fillEllipse (tipX - 5.5f, tipY - 5.5f, 11.0f, 11.0f);
        g.setColour (arcColour);
        g.fillEllipse (tipX - 3.0f, tipY - 3.0f, 6.0f, 6.0f);
        // Specular white core for LED brightness
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.fillEllipse (tipX - 1.5f, tipY - 1.5f, 3.0f, 3.0f);
    }
}

//==============================================================================
void MultiverseFlatTheme::drawLinearSlider (juce::Graphics& g,
                                             int x, int y, int w, int h,
                                             float sliderPos, float minPos, float maxPos,
                                             juce::Slider::SliderStyle style,
                                             juce::Slider& slider)
{
    const Skin& s = skin();
    const bool isHoriz = (style == juce::Slider::LinearHorizontal ||
                           style == juce::Slider::LinearBar);
    const bool isVert  = (style == juce::Slider::LinearVertical);

    if (isHoriz)
    {
        const float trackH = 4.0f;
        const float trackY = y + h / 2.0f - trackH / 2.0f;
        auto track = juce::Rectangle<float> ((float)x, trackY, (float)w, trackH);

        g.setColour (s.bgDeep);
        g.fillRoundedRectangle (track, trackH / 2.0f);

        if (sliderPos > minPos + 1.0f)
        {
            auto fill = track.withRight (sliderPos);
            g.setColour (s.accent1.withAlpha (0.15f));
            g.fillRoundedRectangle (fill.expanded (0.0f, 2.0f), trackH / 2.0f + 2.0f);
            g.setColour (s.accent1);
            g.fillRoundedRectangle (fill, trackH / 2.0f);
        }

        const float thumbW = 10.0f;
        const float thumbH = 16.0f;
        auto thumb = juce::Rectangle<float> (sliderPos - thumbW / 2.0f,
                                              y + h / 2.0f - thumbH / 2.0f,
                                              thumbW, thumbH);
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else if (isVert)
    {
        const float trackW = 4.0f;
        const float trackX = x + w / 2.0f - trackW / 2.0f;
        auto track = juce::Rectangle<float> (trackX, (float)y, trackW, (float)h);

        g.setColour (s.bgDeep);
        g.fillRoundedRectangle (track, trackW / 2.0f);

        if (sliderPos < minPos - 1.0f)
        {
            auto fill = track.withTop (sliderPos).withBottom (minPos);
            g.setColour (s.accent1.withAlpha (0.15f));
            g.fillRoundedRectangle (fill.expanded (2.0f, 0.0f), trackW / 2.0f + 2.0f);
            g.setColour (s.accent1);
            g.fillRoundedRectangle (fill, trackW / 2.0f);
        }

        const float thumbW = 16.0f;
        const float thumbH = 10.0f;
        auto thumb = juce::Rectangle<float> (x + w / 2.0f - thumbW / 2.0f,
                                              sliderPos - thumbH / 2.0f,
                                              thumbW, thumbH);
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, slider);
    }
}

//==============================================================================
void MultiverseFlatTheme::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool /*highlight*/, bool /*down*/)
{
    const Skin& s = skin();
    const bool on = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    const float corner = bounds.getHeight() / 2.0f;

    // ── 3D procedural toggle ─────────────────────────────────────────
    // Background: raised surface (off) or inset surface with glow (on)
    if (on)
    {
        // Outer glow
        drawGlow (g, bounds.expanded (3.0f), s.accent1, corner + 3.0f, 0.2f);
        // Inset fill
        drawInset (g, bounds, corner);
        g.setColour (s.accent1.withAlpha (0.15f));
        g.fillRoundedRectangle (bounds, corner);
    }
    else
    {
        // Raised surface
        drawBevel (g, bounds, corner);
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (bounds, corner);
    }

    // Border
    g.setColour (on ? s.accent1.withAlpha (0.6f) : s.borderLight);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

    // ── Dot indicator ──────────────────────────────────────────────
    const float dotSize = 6.0f;
    const float dotX = bounds.getX() + 8.0f;
    const float dotY = bounds.getCentreY() - dotSize / 2.0f;
    if (on)
    {
        g.setColour (s.accent1.withAlpha (0.3f));
        g.fillEllipse (dotX - 3.0f, dotY - 3.0f, dotSize + 6.0f, dotSize + 6.0f);
    }
    g.setColour (on ? s.accent1 : s.textMuted);
    g.fillEllipse (dotX, dotY, dotSize, dotSize);

    // ── Text label ─────────────────────────────────────────────────
    g.setColour (on ? s.textPrimary : s.textSecondary);
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (11.0f)));
    const int textX = (int)(dotX + dotSize + 5.0f);
    g.drawFittedText (button.getButtonText(),
                      textX, 0, (int)(bounds.getRight()) - textX - 4, (int)bounds.getHeight(),
                      juce::Justification::centredLeft, 1);
}

//==============================================================================
void MultiverseFlatTheme::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& /*bgColour*/,
                                                  bool highlight, bool isDown)
{
    const Skin& s = skin();
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const float corner = 4.0f;
    const bool active = isDown || button.getToggleState();
    const bool enabled = button.isEnabled();

    // ── 3D procedural button ─────────────────────────────────────────
    if (!enabled)
    {
        // Disabled: flat, dimmed
        g.setColour (s.bgRaised.withAlpha (0.5f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight.withAlpha (0.3f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
        return;
    }

    if (active)
    {
        // Pressed/active: inset with accent glow
        drawGlow (g, bounds.expanded (3.0f), s.accent1, corner + 2.0f, 0.15f);
        drawInset (g, bounds, corner);
        g.setColour (s.accent1.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else if (highlight)
    {
        // Hover: raised with subtle glow
        drawBevel (g, bounds, corner);
        g.setColour (s.bgHover);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        // Normal: raised surface with gradient
        drawBevel (g, bounds, corner);
        drawGradientFill (g, bounds.reduced (1.0f));
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
}

void MultiverseFlatTheme::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool highlight, bool isDown)
{
    const Skin& s = skin();
    const bool active = isDown || button.getToggleState();
    g.setColour (active ? s.accent1
                        : (highlight ? s.textPrimary : s.textSecondary));
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (11.0f)));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseFlatTheme::drawComboBox (juce::Graphics& g, int width, int height,
                                         bool isButtonDown, int buttonX, int buttonY,
                                         int buttonW, int /*buttonH*/, juce::ComboBox& box)
{
    const Skin& s = skin();
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float)width, (float)height).reduced (1.0f);
    const float corner = 4.0f;
    const bool hasFocus = box.hasKeyboardFocus (true);

    // ── 3D procedural dropdown ────────────────────────────────────────
    if (isButtonDown)
    {
        // Pressed: inset with accent glow
        drawInset (g, bounds, corner);
        g.setColour (s.accent1.withAlpha (0.1f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else if (hasFocus)
    {
        // Focused: raised with glow ring
        drawGlow (g, bounds.expanded (3.0f), s.accent1, corner + 2.0f, 0.15f);
        drawBevel (g, bounds, corner);
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        // Normal: raised surface
        drawBevel (g, bounds, corner);
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }

    // ── Procedural dropdown arrow ─────────────────────────────────────
    const float arrowX = static_cast<float> (width) - 16.0f;
    const float arrowY = static_cast<float> (height) / 2.0f;
    juce::Path arrow;
    arrow.addTriangle (arrowX - 4.0f, arrowY - 2.0f,
                       arrowX + 4.0f, arrowY - 2.0f,
                       arrowX,        arrowY + 3.0f);
    g.setColour (hasFocus ? s.accent1 : s.textSecondary);
    g.fillPath (arrow);
}

//==============================================================================
void MultiverseFlatTheme::drawTabAreaBehindFrontButton (juce::TabbedButtonBar& /*bar*/,
                                                          juce::Graphics& g, int w, int h)
{
    const Skin& s = skin();
    g.setColour (s.bgDeep);
    g.fillRect (0, 0, w, h);
    g.setColour (s.borderLight);
    g.drawLine (0.0f, (float)h - 0.5f, (float)w, (float)h - 0.5f, 1.0f);
}

void MultiverseFlatTheme::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                           bool mouseOver, bool /*mouseDown*/)
{
    const Skin& s = skin();
    const bool front = button.isFrontTab();
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f, 2.0f);

    if (front)
    {
        g.setColour (s.bgBase);
        g.fillRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f);
        g.setColour (s.accent1);
        g.fillRect (bounds.getX() + 4.0f, bounds.getBottom() - 2.0f, bounds.getWidth() - 8.0f, 2.0f);
    }
    else
    {
        g.setColour (mouseOver ? s.bgHover : s.bgRaised.withAlpha (0.9f));
        g.fillRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f, 0.8f);
    }

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f).withStyle ("Bold")));
    g.setColour (front  ? s.accent1
               : mouseOver ? s.textPrimary
                           : s.textSecondary);
    g.drawFittedText (button.getButtonText(), bounds.toNearestInt().reduced (4, 0),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseFlatTheme::drawLabel (juce::Graphics& g, juce::Label& label)
{
    const Skin& s = skin();
    if (label.isBeingEdited())
    {
        g.setColour (s.accent1.withAlpha (0.35f));
        g.drawRoundedRectangle (label.getLocalBounds().toFloat().reduced (1.0f), 3.0f, 1.0f);
        return;
    }

    if (label.isEnabled())
        g.setColour (label.findColour (juce::Label::textColourId));
    else
        g.setColour (s.textMuted);

    g.setFont (label.getFont());
    g.drawFittedText (label.getText(),
                      label.getBorderSize().subtractedFrom (label.getLocalBounds()),
                      label.getJustificationType(),
                      juce::jmax (1, (int)(label.getHeight() / label.getFont().getHeight())),
                      label.getMinimumHorizontalScale());
}

//==============================================================================
void MultiverseFlatTheme::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    const Skin& s = skin();
    g.setColour (s.bgRaised);
    g.fillRoundedRectangle (0.0f, 0.0f, (float)width, (float)height, 4.0f);
    g.setColour (s.borderLight);
    g.drawRoundedRectangle (0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 4.0f, 1.0f);
}

void MultiverseFlatTheme::drawPopupMenuItem (juce::Graphics& g,
                                               const juce::Rectangle<int>& area,
                                               bool isSeparator, bool isActive,
                                               bool isHighlighted, bool isTicked,
                                               bool hasSubMenu,
                                               const juce::String& text,
                                               const juce::String& shortcut,
                                               const juce::Drawable* /*icon*/,
                                               const juce::Colour* textColour)
{
    const Skin& s = skin();
    if (isSeparator)
    {
        g.setColour (s.borderLight);
        g.drawLine ((float)area.getX() + 6, area.getCentreY(),
                    (float)area.getRight() - 6, area.getCentreY(), 1.0f);
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour (s.accent1.withAlpha (0.12f));
        g.fillRoundedRectangle (area.toFloat().reduced (2.0f, 1.0f), 3.0f);
    }

    const juce::Colour col = textColour ? *textColour
                                        : (isActive ? s.textPrimary : s.textMuted);
    g.setColour (col);
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f)));

    auto textArea = area.toFloat().reduced (8.0f, 0.0f);

    if (isTicked)
    {
        g.setColour (s.accent1);
        g.fillEllipse (textArea.getX(), textArea.getCentreY() - 3.0f, 6.0f, 6.0f);
        textArea = textArea.withLeft (textArea.getX() + 12.0f);
        g.setColour (col);
    }

    g.drawFittedText (text, textArea.toNearestInt(),
                      juce::Justification::centredLeft, 1);

    if (hasSubMenu)
    {
        const float ax = textArea.getRight() - 10.0f;
        const float ay = textArea.getCentreY();
        juce::Path arrow;
        arrow.addTriangle (ax, ay - 4.0f, ax, ay + 4.0f, ax + 6.0f, ay);
        g.setColour (s.textSecondary);
        g.fillPath (arrow);
    }

    if (!shortcut.isEmpty())
    {
        g.setColour (s.textMuted);
        g.setFont (juce::Font (juce::FontOptions{}.withHeight (10.0f)));
        g.drawText (shortcut, area.reduced (8, 0), juce::Justification::centredRight, true);
    }
}

//==============================================================================
void MultiverseFlatTheme::drawScrollbar (juce::Graphics& g, juce::ScrollBar& /*bar*/,
                                          int x, int y, int w, int h,
                                          bool isVertical, int thumbPos, int thumbSize,
                                          bool isMouseOver, bool /*isMouseDown*/)
{
    const Skin& s = skin();
    g.setColour (s.bgDeep.withAlpha (0.5f));
    g.fillRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 2.0f);

    if (thumbSize <= 0) return;

    juce::Rectangle<float> thumb;
    if (isVertical)
        thumb = { (float)x + 1.0f, (float)(y + thumbPos), (float)w - 2.0f, (float)thumbSize };
    else
        thumb = { (float)(x + thumbPos), (float)y + 1.0f, (float)thumbSize, (float)h - 2.0f };

    g.setColour (s.accent1.withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.fillRoundedRectangle (thumb, 2.0f);
}

//==============================================================================
void MultiverseFlatTheme::drawDivider (juce::Graphics& g, float y, float x1, float x2)
{
    draw3DSeparator (g, y, x1, x2);
}

//==============================================================================
// 3D rendering primitives

void MultiverseFlatTheme::drawBevel (juce::Graphics& g, juce::Rectangle<float> bounds,
                                       float cornerRadius, float strength)
{
    const float s = bevelStrength() * strength;

    // Top-left highlight (light from upper-left)
    g.setColour (shadowLight().withAlpha (0.25f * s));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, 1.0f);

    // Bottom-right shadow (dark on lower-right)
    auto shadowBounds = bounds.translated (0.5f, 0.5f);
    g.setColour (shadowDark().withAlpha (0.35f * s));
    g.drawRoundedRectangle (shadowBounds.reduced (0.5f), cornerRadius, 1.0f);

    // Inner highlight at top edge
    g.setColour (shadowLight().withAlpha (0.12f * s));
    g.drawHorizontalLine (static_cast<int>(bounds.getY() + 1.0f),
                          bounds.getX() + cornerRadius, bounds.getRight() - cornerRadius);
}

void MultiverseFlatTheme::drawInset (juce::Graphics& g, juce::Rectangle<float> bounds,
                                       float cornerRadius, float strength)
{
    const float s = bevelStrength() * strength;

    // Inner shadow all around (sunken effect)
    auto inner = bounds.reduced (1.0f);

    // Top shadow
    g.setColour (shadowDark().withAlpha (0.3f * s));
    g.drawHorizontalLine (static_cast<int>(bounds.getY()),
                          bounds.getX() + cornerRadius, bounds.getRight() - cornerRadius);

    // Left shadow
    g.drawVerticalLine (static_cast<int>(bounds.getX()),
                        bounds.getY() + cornerRadius, bounds.getBottom() - cornerRadius);

    // Bottom highlight
    g.setColour (shadowLight().withAlpha (0.15f * s));
    g.drawHorizontalLine (static_cast<int>(bounds.getBottom() - 1.0f),
                          bounds.getX() + cornerRadius, bounds.getRight() - cornerRadius);

    // Right highlight
    g.drawVerticalLine (static_cast<int>(bounds.getRight() - 1.0f),
                        bounds.getY() + cornerRadius, bounds.getBottom() - cornerRadius);
}

void MultiverseFlatTheme::drawGlow (juce::Graphics& g, juce::Rectangle<float> bounds,
                                      juce::Colour colour, float radius, float alpha)
{
    const float intensity = glowIntensity();
    if (intensity <= 0.0f)
        return;

    const float a = alpha * intensity;
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float rx = radius > 0.0f ? radius : bounds.getWidth() * 0.5f;
    const float ry = radius > 0.0f ? radius : bounds.getHeight() * 0.5f;

    // Multi-pass glow: wide outer, medium mid, tight inner
    for (int i = 3; i >= 1; --i)
    {
        const float scale = static_cast<float>(i);
        const float passAlpha = a * (0.15f / scale);
        g.setColour (colour.withAlpha (passAlpha));
        g.fillRoundedRectangle (bounds.expanded (rx * scale * 0.25f, ry * scale * 0.25f),
                                bounds.getHeight() * 0.5f + scale * 3.0f);
    }
}

void MultiverseFlatTheme::draw3DSeparator (juce::Graphics& g, float y, float x1, float x2)
{
    const float s = bevelStrength();

    // Light line above (lit from top)
    g.setColour (shadowLight().withAlpha (0.2f * s));
    g.drawHorizontalLine (static_cast<int>(y - 1.0f), x1, x2);

    // Dark line (the separator itself)
    g.setColour (borderLight().withAlpha (Metrics::dividerAlpha));
    g.drawHorizontalLine (static_cast<int>(y), x1, x2);
}

void MultiverseFlatTheme::drawGradientFill (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto gradient = juce::ColourGradient (panelGradient1(), bounds.getX(), bounds.getY(),
                                           panelGradient2(), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRect (bounds);
}

void MultiverseFlatTheme::drawHeaderBackground (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const Skin& s = skin();

    // 3D gradient header with bottom glow line
    auto gradient = juce::ColourGradient (s.bgRaised.brighter (0.15f), bounds.getX(), bounds.getY(),
                                           s.bgDeep.darker (0.1f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRect (bounds);

    // Bottom accent line
    g.setColour (s.accent1.withAlpha (0.4f * glowIntensity()));
    g.drawHorizontalLine (static_cast<int>(bounds.getBottom() - 1.0f), bounds.getX(), bounds.getRight());

    // Subtle inner shadow at top edge
    g.setColour (shadowDark().withAlpha (0.3f));
    g.drawHorizontalLine (static_cast<int>(bounds.getY()), bounds.getX(), bounds.getRight());
}

void MultiverseFlatTheme::drawTabBarBackground (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const Skin& s = skin();

    // 3D gradient for tab bar
    auto gradient = juce::ColourGradient (s.tabPrimaryBg.brighter (0.05f), bounds.getX(), bounds.getY(),
                                           s.tabPrimaryBg.darker (0.08f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (gradient);
    g.fillRect (bounds);
}

void MultiverseFlatTheme::drawOverlayBackdrop (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const Skin& s = skin();
    g.setColour (s.bgVoid.withAlpha (0.85f));
    g.fillRect (bounds);

    const float radius = juce::jmax (bounds.getWidth(), bounds.getHeight()) * 0.7f;
    juce::Point<float> centre (bounds.getCentreX(), bounds.getCentreY());
    juce::ColourGradient vignette (juce::Colours::transparentBlack, centre,
                                    s.bgVoid.withAlpha (0.4f), centre.translated (radius, 0.0f), true);
    g.setGradientFill (vignette);
    g.fillRect (bounds);
}

//==============================================================================
// Two-tier tab buttons — now bigger and more vivid

void MultiverseFlatTheme::drawPrimaryTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                                  const juce::String& text, bool isActive, bool isHover)
{
    const Skin& s = skin();

    // 3D gradient background
    if (isActive)
    {
        auto gradient = juce::ColourGradient (s.tabActiveBg.brighter (0.08f), bounds.getX(), bounds.getY(),
                                               s.tabActiveBg, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRect (bounds);

        // Glow band
        drawGlow (g, bounds.reduced (0, bounds.getHeight() * 0.5f), s.accent1, bounds.getWidth() * 0.3f, 0.08f);

        // Solid accent line
        g.setColour (s.accent1);
        g.fillRect (bounds.getX(), bounds.getBottom() - 2.0f, bounds.getWidth(), 2.0f);
    }
    else
    {
        auto bg = isHover ? s.bgHover : s.tabPrimaryBg;
        auto gradient = juce::ColourGradient (bg.brighter (0.03f), bounds.getX(), bounds.getY(),
                                               bg.darker (0.03f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRect (bounds);

        // Subtle bevel at top
        g.setColour (shadowLight().withAlpha (0.12f));
        g.drawHorizontalLine (static_cast<int>(bounds.getY()), bounds.getX(), bounds.getRight());
    }

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (14.0f).withStyle ("Bold")));
    g.setColour (isActive ? s.accent1
                          : (isHover ? s.textPrimary : s.textSecondary));
    g.drawFittedText (text, bounds.toNearestInt().reduced (8, 0),
                      juce::Justification::centred, 1);
}

void MultiverseFlatTheme::drawSecondaryTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                                    const juce::String& text, bool isActive, bool isHover)
{
    const Skin& s = skin();

    if (isActive)
    {
        auto gradient = juce::ColourGradient (s.tabActiveBg.brighter (0.06f), bounds.getX(), bounds.getY(),
                                               s.tabActiveBg.darker (0.04f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRect (bounds);

        // Accent underline with glow
        drawGlow (g, bounds.withTop (bounds.getBottom() - 6.0f), s.accent1, bounds.getWidth() * 0.25f, 0.06f);
        g.setColour (s.accent1.withAlpha (0.2f));
        g.fillRect (bounds.getX(), bounds.getBottom() - 4.0f, bounds.getWidth(), 4.0f);
        g.setColour (s.accent1.withAlpha (0.7f));
        g.fillRect (bounds.getX(), bounds.getBottom() - 1.5f, bounds.getWidth(), 1.5f);
    }
    else
    {
        auto bg = isHover ? s.bgHover : s.tabSecondaryBg;
        auto gradient = juce::ColourGradient (bg.brighter (0.02f), bounds.getX(), bounds.getY(),
                                               bg.darker (0.02f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRect (bounds);
    }

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f).withStyle ("Bold")));
    g.setColour (isActive ? s.accent1
                          : (isHover ? s.textPrimary : s.textMuted));
    g.drawFittedText (text, bounds.toNearestInt().reduced (4, 0),
                      juce::Justification::centred, 1);
}

void MultiverseFlatTheme::drawSubTabButton (juce::Graphics& g, juce::Rectangle<float> bounds,
                                              const juce::String& text, bool isActive, bool isHover)
{
    const Skin& s = skin();
    const float corner = bounds.getHeight() / 2.0f;

    if (isActive)
    {
        // Glow behind pill
        drawGlow (g, bounds, s.accent1, bounds.getHeight() * 0.4f, 0.06f);

        // 3D raised pill
        auto gradient = juce::ColourGradient (s.accent1.withAlpha (0.18f), bounds.getX(), bounds.getY(),
                                               s.accent1.withAlpha (0.08f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (bounds, corner);

        // Accent border
        g.setColour (s.accent1.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else if (isHover)
    {
        auto gradient = juce::ColourGradient (s.bgHover.brighter (0.03f), bounds.getX(), bounds.getY(),
                                               s.bgHover.darker (0.03f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        auto gradient = juce::ColourGradient (s.bgRaised.brighter (0.02f), bounds.getX(), bounds.getY(),
                                               s.bgRaised.darker (0.03f), bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (gradient);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (11.0f).withStyle ("Bold")));
    g.setColour (isActive ? s.accent1 : s.textSecondary);
    g.drawFittedText (text, bounds.toNearestInt().reduced (8, 0),
                      juce::Justification::centred, 1);
}

//==============================================================================
// Font getters
// Font getters
juce::Font MultiverseFlatTheme::headerFont()
{
    return juce::Font (juce::FontOptions{}.withHeight (Metrics::fontHeader).withStyle ("Bold"));
}

juce::Font MultiverseFlatTheme::labelFont()
{
    return juce::Font (juce::FontOptions{}.withHeight (Metrics::fontLabel));
}

juce::Font MultiverseFlatTheme::valueFont()
{
    return juce::Font (juce::FontOptions{}.withHeight (Metrics::fontValue)
                        .withName (juce::Font::getDefaultMonospacedFontName()));
}

juce::Font MultiverseFlatTheme::titleFont()
{
    return juce::Font (juce::FontOptions{}.withHeight (Metrics::fontTitle).withStyle ("Bold"));
}