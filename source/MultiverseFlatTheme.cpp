#include "MultiverseFlatTheme.h"

//==============================================================================
// Palette
const juce::Colour MultiverseFlatTheme::bgVoid      { 0xFF0F1014 };
const juce::Colour MultiverseFlatTheme::bgBase      { 0xFF15171C };
const juce::Colour MultiverseFlatTheme::bgRaised    { 0xFF262930 };
const juce::Colour MultiverseFlatTheme::bgDeep      { 0xFF111318 };
const juce::Colour MultiverseFlatTheme::bgHover     { 0xFF323540 };

const juce::Colour MultiverseFlatTheme::accentCyan  { 0xFF00D4FF };
const juce::Colour MultiverseFlatTheme::accentPink  { 0xFFFF2A6D };
const juce::Colour MultiverseFlatTheme::accentPurple{ 0xFF9B6DFF };
const juce::Colour MultiverseFlatTheme::accentGreen { 0xFF00FF87 };
const juce::Colour MultiverseFlatTheme::accentAmber { 0xFFFFB800 };

const juce::Colour MultiverseFlatTheme::accentBlue   { accentCyan };
const juce::Colour MultiverseFlatTheme::neonCyan     { accentCyan };
const juce::Colour MultiverseFlatTheme::neonPink     { accentPink };
const juce::Colour MultiverseFlatTheme::neonPurple   { accentPurple };
const juce::Colour MultiverseFlatTheme::neonGreen    { accentGreen };

const juce::Colour MultiverseFlatTheme::borderLight { 0xFF404558 };
const juce::Colour MultiverseFlatTheme::borderActive{ accentCyan };

const juce::Colour MultiverseFlatTheme::textPrimary   { 0xFFF0F0F8 };
const juce::Colour MultiverseFlatTheme::textSecondary { 0xFF9098A8 };
const juce::Colour MultiverseFlatTheme::textMuted     { 0xFF556070 };
const juce::Colour MultiverseFlatTheme::textLabel     { 0xFF7080A0 };

//==============================================================================
MultiverseFlatTheme::MultiverseFlatTheme()
{
    // Window / panel backgrounds
    setColour (juce::ResizableWindow::backgroundColourId, bgBase);
    setColour (juce::DocumentWindow::backgroundColourId,  bgBase);

    // Labels
    setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId,       textPrimary);
    setColour (juce::Label::outlineColourId,    juce::Colours::transparentBlack);
    setColour (juce::Label::textWhenEditingColourId, textPrimary);

    // Sliders
    setColour (juce::Slider::backgroundColourId,         bgDeep);
    setColour (juce::Slider::thumbColourId,              bgRaised);
    setColour (juce::Slider::trackColourId,              accentCyan);
    setColour (juce::Slider::rotarySliderFillColourId,   accentCyan);
    setColour (juce::Slider::rotarySliderOutlineColourId, borderLight);
    setColour (juce::Slider::textBoxTextColourId,        textSecondary);
    setColour (juce::Slider::textBoxBackgroundColourId,  bgDeep);
    setColour (juce::Slider::textBoxOutlineColourId,     juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,   accentCyan.withAlpha (0.3f));

    // TextButton
    setColour (juce::TextButton::buttonColourId,   bgRaised);
    setColour (juce::TextButton::buttonOnColourId, bgDeep);
    setColour (juce::TextButton::textColourOffId,  textSecondary);
    setColour (juce::TextButton::textColourOnId,   accentCyan);

    // ToggleButton
    setColour (juce::ToggleButton::textColourId,        textSecondary);
    setColour (juce::ToggleButton::tickColourId,        accentCyan);
    setColour (juce::ToggleButton::tickDisabledColourId, textMuted);

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,    bgRaised);
    setColour (juce::ComboBox::textColourId,          textPrimary);
    setColour (juce::ComboBox::outlineColourId,       juce::Colours::transparentBlack);
    setColour (juce::ComboBox::buttonColourId,        bgRaised);
    setColour (juce::ComboBox::arrowColourId,         textSecondary);
    setColour (juce::ComboBox::focusedOutlineColourId, accentCyan);

    // PopupMenu
    setColour (juce::PopupMenu::backgroundColourId,           bgRaised);
    setColour (juce::PopupMenu::textColourId,                 textPrimary);
    setColour (juce::PopupMenu::headerTextColourId,           textSecondary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accentCyan.withAlpha (0.15f));
    setColour (juce::PopupMenu::highlightedTextColourId,       textPrimary);

    // Tabs
    setColour (juce::TabbedButtonBar::tabTextColourId,    textSecondary);
    setColour (juce::TabbedButtonBar::frontTextColourId,  accentCyan);
    setColour (juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::TabbedComponent::backgroundColourId, bgBase);
    setColour (juce::TabbedComponent::outlineColourId,    juce::Colours::transparentBlack);

    // TextEditor
    setColour (juce::TextEditor::backgroundColourId,      bgDeep);
    setColour (juce::TextEditor::textColourId,            textPrimary);
    setColour (juce::TextEditor::highlightColourId,       accentCyan.withAlpha (0.3f));
    setColour (juce::TextEditor::highlightedTextColourId,  textPrimary);
    setColour (juce::TextEditor::outlineColourId,         juce::Colours::transparentBlack);
    setColour (juce::TextEditor::focusedOutlineColourId,  accentCyan.withAlpha (0.5f));

    // ListBox
    setColour (juce::ListBox::backgroundColourId, bgDeep);
    setColour (juce::ListBox::textColourId,       textPrimary);
    setColour (juce::ListBox::outlineColourId,    juce::Colours::transparentBlack);

    // ScrollBar
    setColour (juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId,      accentCyan.withAlpha (0.4f));
    setColour (juce::ScrollBar::trackColourId,      bgDeep);

    // GroupComponent
    setColour (juce::GroupComponent::textColourId,    textLabel);
    setColour (juce::GroupComponent::outlineColourId, borderLight);

    // Tooltip
    setColour (juce::TooltipWindow::backgroundColourId, bgRaised);
    setColour (juce::TooltipWindow::textColourId,       textPrimary);
    setColour (juce::TooltipWindow::outlineColourId,    borderLight);

    // TableListBox / headers
    setColour (juce::TableHeaderComponent::backgroundColourId,      bgRaised);
    setColour (juce::TableHeaderComponent::textColourId,            textSecondary);
    setColour (juce::TableHeaderComponent::outlineColourId,         borderLight);
    setColour (juce::TableHeaderComponent::highlightColourId,       accentCyan.withAlpha (0.12f));
}

//==============================================================================
// Flat card helper

void MultiverseFlatTheme::drawCard (juce::Graphics& g,
                                     juce::Rectangle<float> bounds,
                                     float cornerRadius, bool isActive)
{
    g.setColour (bgRaised);
    g.fillRoundedRectangle (bounds, cornerRadius);
    g.setColour (isActive ? borderActive : borderLight);
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerRadius, 1.5f);
}

//==============================================================================
void MultiverseFlatTheme::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int w, int h,
                                             float sliderPos,
                                             float startAngle, float endAngle,
                                             juce::Slider& slider)
{
    const float radius = juce::jmin (w / 2.0f, h / 2.0f) - 4.0f;
    if (radius < 4.0f) return;

    const float cx = x + w / 2.0f;
    const float cy = y + h / 2.0f;

    // Subtle outer ring outline
    g.setColour (bgDeep.withAlpha (0.6f));
    g.drawEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 0.5f);

    // Arc groove (track)
    const float arcRadius  = radius * 0.72f;
    const float trackWidth = 2.0f;
    {
        juce::Path groove;
        groove.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f,
                               startAngle, endAngle, true);
        g.setColour (bgDeep);
        g.strokePath (groove, juce::PathStrokeType (trackWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Fill arc + tip dot
    const juce::Colour arcColour = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);
    if (sliderPos > 0.001f)
    {
        juce::Path fillArc;
        fillArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f,
                                startAngle, currentAngle, true);
        g.setColour (arcColour);
        g.strokePath (fillArc, juce::PathStrokeType (trackWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float tipX = cx + arcRadius * std::sin (currentAngle);
        const float tipY = cy - arcRadius * std::cos (currentAngle);
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
    const bool isHoriz = (style == juce::Slider::LinearHorizontal ||
                           style == juce::Slider::LinearBar);
    const bool isVert  = (style == juce::Slider::LinearVertical);

    if (isHoriz)
    {
        const float trackH = 4.0f;
        const float trackY = y + h / 2.0f - trackH / 2.0f;
        auto track = juce::Rectangle<float> ((float)x, trackY, (float)w, trackH);

        // Track
        g.setColour (bgDeep);
        g.fillRoundedRectangle (track, trackH / 2.0f);

        // Fill
        if (sliderPos > minPos + 1.0f)
        {
            auto fill = track.withRight (sliderPos);
            g.setColour (accentCyan);
            g.fillRoundedRectangle (fill, trackH / 2.0f);
        }

        // Thumb (flat)
        const float thumbW = 10.0f;
        const float thumbH = 16.0f;
        auto thumb = juce::Rectangle<float> (sliderPos - thumbW / 2.0f,
                                              y + h / 2.0f - thumbH / 2.0f,
                                              thumbW, thumbH);
        g.setColour (bgRaised);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (borderLight);
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else if (isVert)
    {
        const float trackW = 4.0f;
        const float trackX = x + w / 2.0f - trackW / 2.0f;
        auto track = juce::Rectangle<float> (trackX, (float)y, trackW, (float)h);

        g.setColour (bgDeep);
        g.fillRoundedRectangle (track, trackW / 2.0f);

        // Fill from thumb down to minimum
        if (sliderPos < minPos - 1.0f)
        {
            auto fill = track.withTop (sliderPos).withBottom (minPos);
            g.setColour (accentCyan);
            g.fillRoundedRectangle (fill, trackW / 2.0f);
        }

        const float thumbW = 16.0f;
        const float thumbH = 10.0f;
        auto thumb = juce::Rectangle<float> (x + w / 2.0f - thumbW / 2.0f,
                                              sliderPos - thumbH / 2.0f,
                                              thumbW, thumbH);
        g.setColour (bgRaised);
        g.fillRoundedRectangle (thumb, 3.0f);
        g.setColour (borderLight);
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
    const bool on = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    const float corner = bounds.getHeight() / 2.0f;

    // Background pill (flat)
    g.setColour (on ? bgDeep : bgRaised);
    g.fillRoundedRectangle (bounds, corner);

    if (on)
    {
        g.setColour (accentCyan.withAlpha (0.12f));
        g.fillRoundedRectangle (bounds, corner);
    }

    // Border
    g.setColour (on ? accentCyan.withAlpha (0.4f) : borderLight);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

    // Indicator dot
    const float dotSize = 6.0f;
    const float dotX = bounds.getX() + 8.0f;
    const float dotY = bounds.getCentreY() - dotSize / 2.0f;
    g.setColour (on ? accentCyan : textMuted);
    g.fillEllipse (dotX, dotY, dotSize, dotSize);

    // Label text
    g.setColour (on ? textPrimary : textSecondary);
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
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const float corner = 4.0f;
    const bool active = isDown || button.getToggleState();

    if (active)
    {
        g.setColour (bgDeep);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentCyan.withAlpha (0.1f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentCyan.withAlpha (0.4f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        g.setColour (highlight ? bgHover : bgRaised);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (highlight ? borderLight.withAlpha (0.6f) : borderLight);
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
}

void MultiverseFlatTheme::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                           bool highlight, bool isDown)
{
    const bool active = isDown || button.getToggleState();
    g.setColour (active ? accentCyan
                        : (highlight ? textPrimary : textSecondary));
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (11.0f)));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseFlatTheme::drawComboBox (juce::Graphics& g, int width, int height,
                                         bool isButtonDown, int buttonX, int buttonY,
                                         int buttonW, int /*buttonH*/, juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float)width, (float)height).reduced (1.0f);
    const float corner = 4.0f;

    g.setColour (isButtonDown ? bgDeep : bgRaised);
    g.fillRoundedRectangle (bounds, corner);
    g.setColour (borderLight);
    g.drawRoundedRectangle (bounds, corner, 1.0f);

    // Dropdown arrow
    const float arrowX = buttonX + buttonW / 2.0f;
    const float arrowY = buttonY + height / 2.0f;
    juce::Path arrow;
    arrow.addTriangle (arrowX - 4.0f, arrowY - 2.0f,
                       arrowX + 4.0f, arrowY - 2.0f,
                       arrowX,        arrowY + 3.0f);
    g.setColour (textSecondary);
    g.fillPath (arrow);
}

//==============================================================================
void MultiverseFlatTheme::drawTabAreaBehindFrontButton (juce::TabbedButtonBar& /*bar*/,
                                                          juce::Graphics& g, int w, int h)
{
    g.setColour (bgDeep);
    g.fillRect (0, 0, w, h);
    // Bottom separator
    g.setColour (borderLight);
    g.drawLine (0.0f, (float)h - 0.5f, (float)w, (float)h - 0.5f, 1.0f);
}

void MultiverseFlatTheme::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                           bool mouseOver, bool /*mouseDown*/)
{
    const bool front = button.isFrontTab();
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f, 2.0f);

    if (front)
    {
        // Active tab: raised background + accent bottom line
        g.setColour (bgBase);
        g.fillRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f);
        // Bottom accent line
        g.setColour (accentCyan);
        g.fillRect (bounds.getX() + 4.0f, bounds.getBottom() - 2.0f,
                    bounds.getWidth() - 8.0f, 2.0f);
    }
    else
    {
        // Inactive tab: clearly visible against dark bar
        g.setColour (mouseOver ? bgHover : bgRaised.withAlpha (0.9f));
        g.fillRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f);
        // Border for separation
        g.setColour (borderLight);
        g.drawRoundedRectangle (bounds.reduced (2.0f, 0.0f), 4.0f, 0.8f);
    }

    g.setFont (juce::Font (juce::FontOptions{}.withHeight (11.0f)));
    g.setColour (front  ? accentCyan
               : mouseOver ? textPrimary
                           : textSecondary);
    g.drawFittedText (button.getButtonText(), bounds.toNearestInt().reduced (4, 0),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseFlatTheme::drawLabel (juce::Graphics& g, juce::Label& label)
{
    if (label.isBeingEdited())
    {
        g.setColour (accentCyan.withAlpha (0.35f));
        g.drawRoundedRectangle (label.getLocalBounds().toFloat().reduced (1.0f), 3.0f, 1.0f);
        return;
    }

    if (label.isEnabled())
    {
        g.setColour (label.findColour (juce::Label::textColourId));
    }
    else
    {
        g.setColour (textMuted);
    }

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
    g.setColour (bgRaised);
    g.fillRoundedRectangle (0.0f, 0.0f, (float)width, (float)height, 4.0f);
    g.setColour (borderLight);
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
    if (isSeparator)
    {
        g.setColour (borderLight);
        g.drawLine ((float)area.getX() + 6, area.getCentreY(),
                    (float)area.getRight() - 6, area.getCentreY(), 1.0f);
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour (accentCyan.withAlpha (0.12f));
        g.fillRoundedRectangle (area.toFloat().reduced (2.0f, 1.0f), 3.0f);
    }

    const juce::Colour col = textColour ? *textColour
                                        : (isActive ? textPrimary : textMuted);
    g.setColour (col);
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (12.0f)));

    auto textArea = area.toFloat().reduced (8.0f, 0.0f);

    if (isTicked)
    {
        g.setColour (accentCyan);
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
        g.setColour (textSecondary);
        g.fillPath (arrow);
    }

    if (!shortcut.isEmpty())
    {
        g.setColour (textMuted);
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
    // Track
    g.setColour (bgDeep.withAlpha (0.5f));
    g.fillRoundedRectangle ((float)x, (float)y, (float)w, (float)h, 2.0f);

    if (thumbSize <= 0) return;

    juce::Rectangle<float> thumb;
    if (isVertical)
        thumb = { (float)x + 1.0f, (float)(y + thumbPos), (float)w - 2.0f, (float)thumbSize };
    else
        thumb = { (float)(x + thumbPos), (float)y + 1.0f, (float)thumbSize, (float)h - 2.0f };

    g.setColour (accentCyan.withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.fillRoundedRectangle (thumb, 2.0f);
}

//==============================================================================
// Section divider line
void MultiverseFlatTheme::drawDivider (juce::Graphics& g, float y, float x1, float x2)
{
    g.setColour (borderLight.withAlpha (Metrics::dividerAlpha));
    g.drawHorizontalLine (static_cast<int>(y), x1, x2);
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