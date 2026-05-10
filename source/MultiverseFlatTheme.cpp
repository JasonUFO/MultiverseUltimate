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
// Flat card helper

void MultiverseFlatTheme::drawCard (juce::Graphics& g,
                                     juce::Rectangle<float> bounds,
                                     float cornerRadius, bool isActive,
                                     juce::Colour fillColor)
{
    const Skin& s = skin();
    juce::Colour fill = fillColor.isTransparent() ? s.bgRaised : fillColor;

    // Neon glow behind active cards
    if (isActive)
    {
        g.setColour (s.accent1.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds.expanded (4.0f), cornerRadius + 2.0f);
    }

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, cornerRadius);

    // Border with neon glow for active cards
    if (isActive)
    {
        g.setColour (s.accent1.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds.expanded (1.0f).reduced (0.5f), cornerRadius + 1.0f, 1.0f);
    }
    g.setColour (isActive ? s.borderActive : s.borderLight);
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, 1.5f);
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

        g.setColour (arcColour.withAlpha (0.25f));
        g.fillEllipse (tipX - 7.0f, tipY - 7.0f, 14.0f, 14.0f);
        g.setColour (arcColour.withAlpha (0.12f));
        g.fillEllipse (tipX - 11.0f, tipY - 11.0f, 22.0f, 22.0f);
        g.setColour (arcColour);
        g.fillEllipse (tipX - 3.5f, tipY - 3.5f, 7.0f, 7.0f);
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

    g.setColour (on ? s.bgDeep : s.bgRaised);
    g.fillRoundedRectangle (bounds, corner);

    if (on)
    {
        g.setColour (s.accent1.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.2f));
        g.drawRoundedRectangle (bounds.expanded (1.5f), corner + 1.5f, 1.5f);
    }
    g.setColour (on ? s.accent1.withAlpha (0.6f) : s.borderLight);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

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

    if (active)
    {
        g.setColour (s.accent1.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds.expanded (3.0f), corner + 2.0f);

        g.setColour (s.bgDeep);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.1f));
        g.fillRoundedRectangle (bounds, corner);

        g.setColour (s.accent1.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else if (highlight)
    {
        g.setColour (s.accent1.withAlpha (0.04f));
        g.fillRoundedRectangle (bounds.expanded (2.0f), corner + 1.0f);

        g.setColour (s.bgHover);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        g.setColour (s.bgRaised);
        g.fillRoundedRectangle (bounds, corner);
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

    if (hasFocus)
    {
        g.setColour (s.accent1.withAlpha (0.1f));
        g.fillRoundedRectangle (bounds.expanded (3.0f), corner + 2.0f);
    }

    g.setColour (isButtonDown ? s.bgDeep : s.bgRaised);
    g.fillRoundedRectangle (bounds, corner);
    g.setColour (hasFocus ? s.accent1 : s.borderLight);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

    const float arrowX = buttonX + buttonW / 2.0f;
    const float arrowY = buttonY + height / 2.0f;
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
// Section divider line
void MultiverseFlatTheme::drawDivider (juce::Graphics& g, float y, float x1, float x2)
{
    g.setColour (borderLight().withAlpha (Metrics::dividerAlpha));
    g.drawHorizontalLine (static_cast<int>(y), x1, x2);
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

    if (isActive)
    {
        // Active: bright background + vivid glow line at bottom
        g.setColour (s.tabActiveBg);
        g.fillRect (bounds);

        // Glow band
        g.setColour (s.tabActiveGlow.withAlpha (0.35f));
        g.fillRect (bounds.getX(), bounds.getBottom() - 6.0f, bounds.getWidth(), 6.0f);

        // Solid accent line
        g.setColour (s.accent1);
        g.fillRect (bounds.getX(), bounds.getBottom() - 2.0f, bounds.getWidth(), 2.0f);
    }
    else
    {
        g.setColour (isHover ? s.bgHover : s.tabPrimaryBg);
        g.fillRect (bounds);
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
        g.setColour (s.tabActiveBg);
        g.fillRect (bounds);
        g.setColour (s.accent1.withAlpha (0.2f));
        g.fillRect (bounds.getX(), bounds.getBottom() - 4.0f, bounds.getWidth(), 4.0f);
        g.setColour (s.accent1.withAlpha (0.7f));
        g.fillRect (bounds.getX(), bounds.getBottom() - 1.5f, bounds.getWidth(), 1.5f);
    }
    else
    {
        g.setColour (isHover ? s.bgHover : s.tabSecondaryBg);
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
        g.setColour (s.accent1.withAlpha (0.06f));
        g.fillRoundedRectangle (bounds.expanded (3.0f), corner + 2.0f);
        g.setColour (s.accent1.withAlpha (0.15f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.accent1.withAlpha (0.5f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else if (isHover)
    {
        g.setColour (s.bgHover);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (s.borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        g.setColour (s.bgRaised);
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