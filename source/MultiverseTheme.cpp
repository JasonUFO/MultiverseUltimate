#include "MultiverseTheme.h"

//==============================================================================
// Palette
const juce::Colour MultiverseTheme::bgBase      { 0xff171720 };
const juce::Colour MultiverseTheme::bgRaised    { 0xff1e1e2c };
const juce::Colour MultiverseTheme::bgDeep      { 0xff111119 };
const juce::Colour MultiverseTheme::shadowDark  { 0xff0b0b12 };
const juce::Colour MultiverseTheme::shadowLight { 0xff252535 };
const juce::Colour MultiverseTheme::accentBlue  { 0xff5b8def };
const juce::Colour MultiverseTheme::accentPurple{ 0xffc084fc };
const juce::Colour MultiverseTheme::accentGreen { 0xff34d399 };
const juce::Colour MultiverseTheme::accentAmber { 0xfffbbf24 };
const juce::Colour MultiverseTheme::textPrimary  { 0xffe4e8f0 };
const juce::Colour MultiverseTheme::textSecondary{ 0xff7a8499 };
const juce::Colour MultiverseTheme::textMuted    { 0xff3d4358 };
const juce::Colour MultiverseTheme::textLabel    { 0xff5a6280 };

//==============================================================================
MultiverseTheme::MultiverseTheme()
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
    setColour (juce::Slider::trackColourId,              accentBlue);
    setColour (juce::Slider::rotarySliderFillColourId,   accentBlue);
    setColour (juce::Slider::rotarySliderOutlineColourId,bgDeep);
    setColour (juce::Slider::textBoxTextColourId,        textSecondary);
    setColour (juce::Slider::textBoxBackgroundColourId,  bgDeep);
    setColour (juce::Slider::textBoxOutlineColourId,     juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxHighlightColourId,   accentBlue.withAlpha (0.3f));

    // TextButton
    setColour (juce::TextButton::buttonColourId,   bgRaised);
    setColour (juce::TextButton::buttonOnColourId, bgDeep);
    setColour (juce::TextButton::textColourOffId,  textSecondary);
    setColour (juce::TextButton::textColourOnId,   accentBlue);

    // ToggleButton
    setColour (juce::ToggleButton::textColourId,        textSecondary);
    setColour (juce::ToggleButton::tickColourId,        accentBlue);
    setColour (juce::ToggleButton::tickDisabledColourId,textMuted);

    // ComboBox
    setColour (juce::ComboBox::backgroundColourId,    bgRaised);
    setColour (juce::ComboBox::textColourId,          textPrimary);
    setColour (juce::ComboBox::outlineColourId,       juce::Colours::transparentBlack);
    setColour (juce::ComboBox::buttonColourId,        bgRaised);
    setColour (juce::ComboBox::arrowColourId,         textSecondary);
    setColour (juce::ComboBox::focusedOutlineColourId,accentBlue);

    // PopupMenu
    setColour (juce::PopupMenu::backgroundColourId,           bgRaised);
    setColour (juce::PopupMenu::textColourId,                 textPrimary);
    setColour (juce::PopupMenu::headerTextColourId,           textSecondary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId,accentBlue.withAlpha (0.2f));
    setColour (juce::PopupMenu::highlightedTextColourId,      textPrimary);

    // Tabs
    setColour (juce::TabbedButtonBar::tabTextColourId,    textSecondary);
    setColour (juce::TabbedButtonBar::frontTextColourId,  accentBlue);
    setColour (juce::TabbedButtonBar::tabOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::TabbedComponent::backgroundColourId, bgBase);
    setColour (juce::TabbedComponent::outlineColourId,    juce::Colours::transparentBlack);

    // TextEditor
    setColour (juce::TextEditor::backgroundColourId,      bgDeep);
    setColour (juce::TextEditor::textColourId,            textPrimary);
    setColour (juce::TextEditor::highlightColourId,       accentBlue.withAlpha (0.3f));
    setColour (juce::TextEditor::highlightedTextColourId, textPrimary);
    setColour (juce::TextEditor::outlineColourId,         juce::Colours::transparentBlack);
    setColour (juce::TextEditor::focusedOutlineColourId,  accentBlue.withAlpha (0.5f));

    // ListBox
    setColour (juce::ListBox::backgroundColourId, bgDeep);
    setColour (juce::ListBox::textColourId,       textPrimary);
    setColour (juce::ListBox::outlineColourId,    juce::Colours::transparentBlack);

    // ScrollBar
    setColour (juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ScrollBar::thumbColourId,      accentBlue.withAlpha (0.5f));
    setColour (juce::ScrollBar::trackColourId,      bgDeep);

    // GroupComponent
    setColour (juce::GroupComponent::textColourId,    textLabel);
    setColour (juce::GroupComponent::outlineColourId, shadowLight);

    // Tooltip
    setColour (juce::TooltipWindow::backgroundColourId, bgRaised);
    setColour (juce::TooltipWindow::textColourId,       textPrimary);
    setColour (juce::TooltipWindow::outlineColourId,    accentBlue.withAlpha (0.3f));

    // TableListBox / headers
    setColour (juce::TableHeaderComponent::backgroundColourId,      bgRaised);
    setColour (juce::TableHeaderComponent::textColourId,            textSecondary);
    setColour (juce::TableHeaderComponent::outlineColourId,         shadowDark);
    setColour (juce::TableHeaderComponent::highlightColourId,       accentBlue.withAlpha (0.15f));
}

//==============================================================================
// Neumorphic shadow helpers

void MultiverseTheme::drawNeumorphicEllipse (juce::Graphics& g,
                                              juce::Rectangle<float> b, float offset)
{
    juce::Path p;
    p.addEllipse (b);

    const int blur = juce::roundToInt (offset * 2.0f);
    {
        juce::DropShadow ds { shadowDark.withAlpha (0.7f), blur, { (int)offset, (int)offset } };
        ds.drawForPath (g, p);
    }
    {
        juce::DropShadow ds { shadowLight.withAlpha (0.5f), blur, { -(int)offset, -(int)offset } };
        ds.drawForPath (g, p);
    }
}

void MultiverseTheme::drawNeumorphicRect (juce::Graphics& g,
                                           juce::Rectangle<float> b,
                                           float cornerRadius, float offset)
{
    juce::Path p;
    p.addRoundedRectangle (b, cornerRadius);

    const int blur = juce::roundToInt (offset * 2.0f);
    {
        juce::DropShadow ds { shadowDark.withAlpha (0.65f), blur, { (int)offset, (int)offset } };
        ds.drawForPath (g, p);
    }
    {
        juce::DropShadow ds { shadowLight.withAlpha (0.45f), blur, { -(int)offset, -(int)offset } };
        ds.drawForPath (g, p);
    }
}

//==============================================================================
void MultiverseTheme::drawRotarySlider (juce::Graphics& g,
                                         int x, int y, int w, int h,
                                         float sliderPos,
                                         float startAngle, float endAngle,
                                         juce::Slider& slider)
{
    const float radius = juce::jmin (w / 2.0f, h / 2.0f) - 6.0f;
    if (radius < 4.0f) return;

    const float cx = x + w / 2.0f;
    const float cy = y + h / 2.0f;
    auto discBounds = juce::Rectangle<float> (cx - radius, cy - radius,
                                               radius * 2.0f, radius * 2.0f);

    // --- Neumorphic shadows ---
    drawNeumorphicEllipse (g, discBounds, 4.0f);

    // --- Main disc (radial gradient for depth) ---
    {
        juce::ColourGradient grad (
            bgRaised.brighter (0.08f), cx - radius * 0.3f, cy - radius * 0.3f,
            bgRaised.darker  (0.12f), cx + radius * 0.5f, cy + radius * 0.5f,
            true);
        g.setGradientFill (grad);
        g.fillEllipse (discBounds);
    }

    // Rim highlight (top-left edge)
    g.setColour (shadowLight.withAlpha (0.55f));
    g.drawEllipse (discBounds.reduced (0.5f), 1.0f);

    // --- Arc groove ---
    const float arcRadius  = radius * 0.70f;
    const float trackWidth = 3.0f;
    {
        juce::Path groove;
        groove.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f,
                               startAngle, endAngle, true);
        // Outer dark edge for depth
        g.setColour (shadowDark.withAlpha (0.8f));
        g.strokePath (groove, juce::PathStrokeType (trackWidth + 2.0f,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // Groove fill
        g.setColour (bgDeep);
        g.strokePath (groove, juce::PathStrokeType (trackWidth,
            juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // --- Fill arc ---
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

        // Glowing tip dot
        const float tipX = cx + arcRadius * std::sin (currentAngle);
        const float tipY = cy - arcRadius * std::cos (currentAngle);
        g.setColour (arcColour.withAlpha (0.25f));
        g.fillEllipse (tipX - 5.0f, tipY - 5.0f, 10.0f, 10.0f);
        g.setColour (arcColour.withAlpha (0.7f));
        g.fillEllipse (tipX - 3.0f, tipY - 3.0f,  6.0f,  6.0f);
    }

    // --- Pointer line ---
    {
        const float lineLen = arcRadius - trackWidth - 1.0f;
        const float px = cx + lineLen * std::sin (currentAngle);
        const float py = cy - lineLen * std::cos (currentAngle);
        g.setColour (textPrimary.withAlpha (0.55f));
        g.drawLine (cx, cy, px, py, 1.5f);
    }

    // --- Center cap ---
    const float capR = radius * 0.20f;
    auto capBounds = juce::Rectangle<float> (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    {
        juce::ColourGradient grad (
            bgRaised.brighter (0.18f), cx - capR * 0.4f, cy - capR * 0.4f,
            bgRaised,                  cx + capR * 0.4f, cy + capR * 0.4f,
            true);
        g.setGradientFill (grad);
        g.fillEllipse (capBounds);
    }

    // LED dot in centre
    const float dotR = 2.5f;
    if (sliderPos > 0.001f)
    {
        g.setColour (arcColour.withAlpha (0.35f));
        g.fillEllipse (cx - dotR * 1.8f, cy - dotR * 1.8f, dotR * 3.6f, dotR * 3.6f);
    }
    g.setColour (sliderPos > 0.001f ? arcColour : juce::Colour (0xff1e2030));
    g.fillEllipse (cx - dotR, cy - dotR, dotR * 2.0f, dotR * 2.0f);
}

//==============================================================================
void MultiverseTheme::drawLinearSlider (juce::Graphics& g,
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
        const float trackH = 5.0f;
        const float trackY = y + h / 2.0f - trackH / 2.0f;
        auto track = juce::Rectangle<float> ((float)x, trackY, (float)w, trackH);

        // Inset track
        g.setColour (shadowDark.withAlpha (0.6f));
        g.fillRoundedRectangle (track.expanded (1.0f), trackH / 2.0f + 1.0f);
        g.setColour (bgDeep);
        g.fillRoundedRectangle (track, trackH / 2.0f);

        // Fill
        if (sliderPos > minPos + 1.0f)
        {
            auto fill = track.withRight (sliderPos);
            juce::ColourGradient grad (accentBlue.darker (0.3f), minPos, 0.0f,
                                       accentBlue,               sliderPos, 0.0f, false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (fill, trackH / 2.0f);
        }

        // Thumb
        const float thumbW = 11.0f;
        const float thumbH = 20.0f;
        auto thumb = juce::Rectangle<float> (sliderPos - thumbW / 2.0f,
                                              y + h / 2.0f - thumbH / 2.0f,
                                              thumbW, thumbH);
        drawNeumorphicRect (g, thumb, 3.0f, 2.0f);
        {
            juce::ColourGradient grad (bgRaised.brighter (0.1f),
                                       thumb.getX(), thumb.getY(),
                                       bgRaised.darker (0.1f),
                                       thumb.getRight(), thumb.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (thumb, 3.0f);
        }
        g.setColour (shadowLight.withAlpha (0.4f));
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else if (isVert)
    {
        const float trackW = 5.0f;
        const float trackX = x + w / 2.0f - trackW / 2.0f;
        auto track = juce::Rectangle<float> (trackX, (float)y, trackW, (float)h);

        g.setColour (shadowDark.withAlpha (0.6f));
        g.fillRoundedRectangle (track.expanded (1.0f), trackW / 2.0f + 1.0f);
        g.setColour (bgDeep);
        g.fillRoundedRectangle (track, trackW / 2.0f);

        // Fill from thumb down to minimum
        if (sliderPos < minPos - 1.0f)
        {
            auto fill = track.withTop (sliderPos).withBottom (minPos);
            juce::ColourGradient grad (accentBlue, 0.0f, minPos,
                                       accentBlue.darker (0.3f), 0.0f, sliderPos, false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (fill, trackW / 2.0f);
        }

        const float thumbW = 20.0f;
        const float thumbH = 11.0f;
        auto thumb = juce::Rectangle<float> (x + w / 2.0f - thumbW / 2.0f,
                                              sliderPos - thumbH / 2.0f,
                                              thumbW, thumbH);
        drawNeumorphicRect (g, thumb, 3.0f, 2.0f);
        {
            juce::ColourGradient grad (bgRaised.brighter (0.1f),
                                       thumb.getX(), thumb.getY(),
                                       bgRaised.darker (0.1f),
                                       thumb.getRight(), thumb.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (thumb, 3.0f);
        }
        g.setColour (shadowLight.withAlpha (0.4f));
        g.drawRoundedRectangle (thumb, 3.0f, 1.0f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, slider);
    }
}

//==============================================================================
void MultiverseTheme::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                         bool /*highlight*/, bool /*down*/)
{
    const bool on = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);
    const float corner = bounds.getHeight() / 2.0f;

    // Background pill
    drawNeumorphicRect (g, bounds, corner, 2.5f);
    g.setColour (on ? bgDeep : bgRaised);
    g.fillRoundedRectangle (bounds, corner);

    // Active tint
    if (on)
    {
        g.setColour (accentBlue.withAlpha (0.18f));
        g.fillRoundedRectangle (bounds, corner);
    }

    // LED dot
    const float ledSize = 7.0f;
    const float ledX = bounds.getX() + 7.0f;
    const float ledY = bounds.getCentreY() - ledSize / 2.0f;

    if (on)
    {
        g.setColour (accentBlue.withAlpha (0.35f));
        g.fillEllipse (ledX - 2.5f, ledY - 2.5f, ledSize + 5.0f, ledSize + 5.0f);
    }
    g.setColour (on ? accentBlue : juce::Colour (0xff1e2030));
    g.fillEllipse (ledX, ledY, ledSize, ledSize);

    // Label text
    g.setColour (on ? textPrimary : textSecondary);
    g.setFont (juce::Font (11.0f, juce::Font::plain));
    const int textX = (int)(ledX + ledSize + 5.0f);
    g.drawFittedText (button.getButtonText(),
                      textX, 0, (int)(bounds.getRight()) - textX - 4, (int)bounds.getHeight(),
                      juce::Justification::centredLeft, 1);
}

//==============================================================================
void MultiverseTheme::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& /*bgColour*/,
                                             bool highlight, bool isDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const float corner = 6.0f;
    const bool active = isDown || button.getToggleState();

    if (active)
    {
        g.setColour (bgDeep);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentBlue.withAlpha (0.15f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentBlue.withAlpha (0.35f));
        g.drawRoundedRectangle (bounds, corner, 1.0f);
    }
    else
    {
        drawNeumorphicRect (g, bounds, corner, 3.0f);
        g.setColour (highlight ? bgRaised.brighter (0.05f) : bgRaised);
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (shadowLight.withAlpha (0.35f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);
    }
}

void MultiverseTheme::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                       bool highlight, bool isDown)
{
    const bool active = isDown || button.getToggleState();
    g.setColour (active ? accentBlue
                        : (highlight ? textPrimary : textSecondary));
    g.setFont (juce::Font (11.0f, juce::Font::plain));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseTheme::drawComboBox (juce::Graphics& g, int width, int height,
                                     bool isButtonDown, int buttonX, int buttonY,
                                     int buttonW, int /*buttonH*/, juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float)width, (float)height).reduced (1.0f);
    const float corner = 6.0f;

    drawNeumorphicRect (g, bounds, corner, 2.5f);
    g.setColour (isButtonDown ? bgDeep : bgRaised);
    g.fillRoundedRectangle (bounds, corner);
    g.setColour (shadowLight.withAlpha (0.3f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);

    // Dropdown arrow
    const float arrowX = buttonX + buttonW / 2.0f;
    const float arrowY = buttonY + height / 2.0f;
    juce::Path arrow;
    arrow.addTriangle (arrowX - 4.5f, arrowY - 2.0f,
                       arrowX + 4.5f, arrowY - 2.0f,
                       arrowX,        arrowY + 3.5f);
    g.setColour (textSecondary);
    g.fillPath (arrow);
}

//==============================================================================
void MultiverseTheme::drawTabAreaBehindFrontButton (juce::TabbedButtonBar& /*bar*/,
                                                     juce::Graphics& g, int w, int h)
{
    // Dark tray behind all tabs
    g.setColour (bgBase.darker (0.08f));
    g.fillRect (0, 0, w, h);
    // Bottom edge line
    g.setColour (shadowLight.withAlpha (0.4f));
    g.drawLine (0.0f, (float)h - 1.0f, (float)w, (float)h - 1.0f, 1.0f);
}

void MultiverseTheme::drawTabButton (juce::TabBarButton& button, juce::Graphics& g,
                                      bool mouseOver, bool /*mouseDown*/)
{
    const bool front = button.isFrontTab();
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f, 3.0f);
    const float corner = 6.0f;

    if (front)
    {
        // Active tab: accent tint + underline
        g.setColour (accentBlue.withAlpha (0.13f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentBlue);
        g.fillRect (bounds.getX() + 4.0f, bounds.getBottom() - 2.0f,
                    bounds.getWidth() - 8.0f, 2.0f);
    }
    else if (mouseOver)
    {
        g.setColour (shadowLight.withAlpha (0.2f));
        g.fillRoundedRectangle (bounds, corner);
    }

    g.setFont (juce::Font (11.0f, juce::Font::plain));
    g.setColour (front ? accentBlue
                       : (mouseOver ? textSecondary.brighter (0.15f) : textSecondary));
    g.drawFittedText (button.getButtonText(), bounds.toNearestInt(),
                      juce::Justification::centred, 1);
}

//==============================================================================
void MultiverseTheme::drawLabel (juce::Graphics& g, juce::Label& label)
{
    if (label.isBeingEdited())
    {
        // Editing border only — TextEditor handles its own content
        g.setColour (accentBlue.withAlpha (0.45f));
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
void MultiverseTheme::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.setColour (bgRaised);
    g.fillRoundedRectangle (0.0f, 0.0f, (float)width, (float)height, 6.0f);
    g.setColour (shadowLight.withAlpha (0.35f));
    g.drawRoundedRectangle (0.5f, 0.5f, (float)width - 1.0f, (float)height - 1.0f, 6.0f, 1.0f);
}

void MultiverseTheme::drawPopupMenuItem (juce::Graphics& g,
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
        g.setColour (shadowLight.withAlpha (0.35f));
        g.drawLine ((float)area.getX() + 6, area.getCentreY(),
                    (float)area.getRight() - 6, area.getCentreY(), 1.0f);
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour (accentBlue.withAlpha (0.18f));
        g.fillRoundedRectangle (area.toFloat().reduced (2.0f, 1.0f), 4.0f);
    }

    const juce::Colour col = textColour ? *textColour
                                        : (isActive ? textPrimary : textMuted);
    g.setColour (col);
    g.setFont (juce::Font (12.0f, juce::Font::plain));

    auto textArea = area.toFloat().reduced (8.0f, 0.0f);

    if (isTicked)
    {
        g.setColour (accentBlue);
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
        g.setFont (juce::Font (10.0f));
        g.drawText (shortcut, area.reduced (8, 0), juce::Justification::centredRight, true);
    }
}

//==============================================================================
void MultiverseTheme::drawScrollbar (juce::Graphics& g, juce::ScrollBar& /*bar*/,
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

    g.setColour (accentBlue.withAlpha (isMouseOver ? 0.65f : 0.40f));
    g.fillRoundedRectangle (thumb, 2.0f);
}
