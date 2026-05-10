#include "PerformancePanel.h"
#include "../MultiverseFlatTheme.h"
#include "../PluginProcessor.h"

//==============================================================================
// XYPad
//==============================================================================
PerformancePanel::XYPad::XYPad(PluginProcessor& p) : proc(p)
{
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}

void PerformancePanel::XYPad::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Card background
    MultiverseFlatTheme::drawCard (g, b, 10.0f, false, MultiverseFlatTheme::bgDeep());

    // Title
    g.setColour(MultiverseFlatTheme::textLabel());
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText("XY PAD  (MACRO 1 \xc3\x97 MACRO 2)",
               (int)b.getX() + 10, (int)b.getY() + 6, 200, 14,
               juce::Justification::centredLeft);

    auto inner = b.reduced(12.0f).withTrimmedTop(16.0f);

    // Grid
    g.setColour(MultiverseFlatTheme::accentBlue().withAlpha(0.10f));
    for (int i = 1; i < 5; ++i)
    {
        float x = inner.getX() + inner.getWidth()  * (float)i / 5.0f;
        float y = inner.getY() + inner.getHeight() * (float)i / 5.0f;
        g.drawLine(x, inner.getY(), x, inner.getBottom(), 0.5f);
        g.drawLine(inner.getX(), y, inner.getRight(), y, 0.5f);
    }

    // Centre crosshair
    g.setColour(MultiverseFlatTheme::accentBlue().withAlpha(0.18f));
    float cx = inner.getCentreX();
    float cy = inner.getCentreY();
    g.drawLine(cx, inner.getY(), cx, inner.getBottom(), 1.0f);
    g.drawLine(inner.getX(), cy, inner.getRight(), cy, 1.0f);

    // Axis labels
    g.setColour(MultiverseFlatTheme::textLabel());
    g.setFont(MultiverseFlatTheme::labelFont());
    g.drawText("M1 \xe2\x86\x92", (int)inner.getX(), (int)inner.getBottom() + 1, 40, 12,
               juce::Justification::centredLeft);
    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(
        -juce::MathConstants<float>::halfPi,
        inner.getX() - 2.0f, inner.getCentreY()));
    g.drawText("M2 \xe2\x86\x92", (int)(inner.getX() - 2.0f) - 20, (int)inner.getCentreY() - 6, 40, 12,
               juce::Justification::centred);
    g.restoreState();

    // Dot
    float dotPX = inner.getX() + dotX * inner.getWidth();
    float dotPY = inner.getY() + (1.0f - dotY) * inner.getHeight();

    // Crosshair through dot
    g.setColour(MultiverseFlatTheme::accentBlue().withAlpha(0.35f));
    g.drawLine(inner.getX(), dotPY, inner.getRight(), dotPY, 1.0f);
    g.drawLine(dotPX, inner.getY(), dotPX, inner.getBottom(), 1.0f);

    // Glow
    const float glowR = 18.0f;
    juce::ColourGradient glow(
        MultiverseFlatTheme::accentBlue().withAlpha(0.35f), dotPX, dotPY,
        MultiverseFlatTheme::accentBlue().withAlpha(0.0f),  dotPX + glowR, dotPY,
        true);
    g.setGradientFill(glow);
    g.fillEllipse(dotPX - glowR, dotPY - glowR, glowR * 2.0f, glowR * 2.0f);

    // Outer ring
    g.setColour(MultiverseFlatTheme::accentBlue());
    g.drawEllipse(dotPX - 8.0f, dotPY - 8.0f, 16.0f, 16.0f, 1.5f);

    // Inner fill
    g.setColour(MultiverseFlatTheme::accentBlue().withAlpha(0.7f));
    g.fillEllipse(dotPX - 5.0f, dotPY - 5.0f, 10.0f, 10.0f);

    // Bright centre
    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.fillEllipse(dotPX - 2.5f, dotPY - 2.5f, 5.0f, 5.0f);
}

void PerformancePanel::XYPad::mouseDown(const juce::MouseEvent& e)
{
    updateFromMouse(e.position);
}

void PerformancePanel::XYPad::mouseDrag(const juce::MouseEvent& e)
{
    updateFromMouse(e.position);
}

void PerformancePanel::XYPad::updateFromMouse(juce::Point<float> pos)
{
    auto inner = getLocalBounds().toFloat().reduced(12.0f).withTrimmedTop(16.0f);
    float nx = juce::jlimit(0.0f, 1.0f, (pos.x - inner.getX()) / inner.getWidth());
    float ny = juce::jlimit(0.0f, 1.0f, 1.0f - (pos.y - inner.getY()) / inner.getHeight());

    if (auto* p1 = proc.apvts.getParameter("macro1"))
        p1->setValueNotifyingHost(nx);
    if (auto* p2 = proc.apvts.getParameter("macro2"))
        p2->setValueNotifyingHost(ny);

    dotX = nx;
    dotY = ny;
    repaint();
}

void PerformancePanel::XYPad::updateFromAPVTS()
{
    auto* r1 = proc.apvts.getRawParameterValue("macro1");
    auto* r2 = proc.apvts.getRawParameterValue("macro2");
    if (!r1 || !r2) return;

    float nx = r1->load();
    float ny = r2->load();
    if (nx != dotX || ny != dotY)
    {
        dotX = nx;
        dotY = ny;
        repaint();
    }
}

//==============================================================================
// PerformancePanel
//==============================================================================
PerformancePanel::PerformancePanel(PluginProcessor& p)
    : proc(p), xyPad(p)
{
    auto& apvts = p.apvts;
    auto& mgr   = p.getMacroManager();

    titleLabel.setText("PERFORMANCE VIEW", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textLabel());
    titleLabel.setFont(MultiverseFlatTheme::headerFont());
    addAndMakeVisible(titleLabel);

    bpmLabel.setJustificationType(juce::Justification::centredRight);
    bpmLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::accentBlue());
    bpmLabel.setFont(MultiverseFlatTheme::titleFont());
    addAndMakeVisible(bpmLabel);

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        auto& grp = macros[i];

        grp.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        grp.knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        grp.knob.setTooltip("Macro " + juce::String(i + 1)
            + " — right-click any knob to assign it here");
        addAndMakeVisible(grp.knob);

        grp.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "macro" + juce::String(i + 1), grp.knob);

        grp.nameLabel.setText(mgr.getName(i), juce::dontSendNotification);
        grp.nameLabel.setJustificationType(juce::Justification::centred);
        grp.nameLabel.setFont(MultiverseFlatTheme::titleFont());
        grp.nameLabel.setEditable(false, true);
        grp.nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        grp.nameLabel.setTooltip("Double-click to rename this macro");
        grp.nameLabel.onTextChange = [this, i]
        {
            proc.getMacroManager().setName(i, macros[i].nameLabel.getText());
        };
        addAndMakeVisible(grp.nameLabel);
    }

    addAndMakeVisible(xyPad);

    startTimerHz(30);
}

PerformancePanel::~PerformancePanel()
{
    stopTimer();
}

void PerformancePanel::paint(juce::Graphics& g)
{
    MultiverseFlatTheme::drawContentBackground(g, getLocalBounds().toFloat());

    // Info strip card
    if (infoCardRect.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, infoCardRect.toFloat(), 8.0f);
    }

    // Macro area card
    if (macroCardRect.getHeight() > 0)
    {
        MultiverseFlatTheme::drawCard (g, macroCardRect.toFloat(), 10.0f);
        g.setColour(MultiverseFlatTheme::textLabel());
        g.setFont(MultiverseFlatTheme::headerFont());
        g.drawText("MACROS",
                   macroCardRect.getX() + 10, macroCardRect.getY() + 6, 100, 13,
                   juce::Justification::centredLeft);
    }
}

void PerformancePanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin);

    // Info strip
    infoCardRect = area.removeFromTop(34);
    area.removeFromTop(6);
    {
        auto row = infoCardRect.reduced(8, 0);
        titleLabel.setBounds(row.removeFromLeft(row.getWidth() / 2));
        bpmLabel.setBounds(row);
    }

    // XY Pad at bottom
    const int xyH = juce::jmin(220, area.getHeight() * 2 / 5);
    auto xyArea = area.removeFromBottom(xyH);
    area.removeFromBottom(6);
    xyPad.setBounds(xyArea);

    // Macro knob grid
    macroCardRect = area;
    auto macroInner = macroCardRect.reduced(8);
    macroInner.removeFromTop(18); // section title

    const int cols  = 4;
    const int cellW = macroInner.getWidth()  / cols;
    const int cellH = macroInner.getHeight() / 2;

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        auto cell = juce::Rectangle<int>(
            macroInner.getX() + col * cellW,
            macroInner.getY() + row * cellH,
            cellW, cellH).reduced(12);

        const int lblH = 24;
        auto& grp = macros[i];
        grp.nameLabel.setBounds(cell.removeFromBottom(lblH));
        grp.knob.setBounds(cell);
    }
}

void PerformancePanel::timerCallback()
{
    xyPad.updateFromAPVTS();

    // Sync macro names from manager
    auto& mgr = proc.getMacroManager();
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const juce::String name = mgr.getName(i);
        if (macros[i].nameLabel.getText() != name)
            macros[i].nameLabel.setText(name, juce::dontSendNotification);
    }

    // BPM from DAW playhead
    if (auto* ph = proc.getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())
            {
                bpmLabel.setText("BPM: " + juce::String(*bpm, 1),
                                 juce::dontSendNotification);
                return;
            }
        }
    }
    bpmLabel.setText("BPM: \xe2\x80\x94", juce::dontSendNotification);
}
