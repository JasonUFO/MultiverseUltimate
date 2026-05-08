#include "LFOShapeEditor.h"
#include "../MultiverseFlatTheme.h"
#include <cmath>
#include <algorithm>

LFOShapeEditor::LFOShapeEditor()
{
    fillSine();

    penBtn.setClickingTogglesState(true);
    penBtn.setToggleState(true, juce::dontSendNotification);
    penBtn.onClick = [this] {
        currentTool = 0;
        penBtn.setToggleState(true,  juce::dontSendNotification);
        lineBtn.setToggleState(false, juce::dontSendNotification);
    };
    addAndMakeVisible(penBtn);

    lineBtn.setClickingTogglesState(true);
    lineBtn.onClick = [this] {
        currentTool = 1;
        penBtn.setToggleState(false, juce::dontSendNotification);
        lineBtn.setToggleState(true,  juce::dontSendNotification);
    };
    addAndMakeVisible(lineBtn);

    sinBtn.onClick  = [this] { fillSine();   notifyChanged(); repaint(); };
    sawBtn.onClick  = [this] { fillSaw();    notifyChanged(); repaint(); };
    sqrBtn.onClick  = [this] { fillSquare(); notifyChanged(); repaint(); };
    triBtn.onClick  = [this] { fillTri();    notifyChanged(); repaint(); };
    normBtn.onClick = [this] { normalize();  notifyChanged(); repaint(); };

    addAndMakeVisible(sinBtn);
    addAndMakeVisible(sawBtn);
    addAndMakeVisible(sqrBtn);
    addAndMakeVisible(triBtn);
    addAndMakeVisible(normBtn);
}

void LFOShapeEditor::setTable(const std::array<float, 256>& t)
{
    table = t;
    repaint();
}

void LFOShapeEditor::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);

    auto ca = canvasArea.toFloat();

    g.setColour(MultiverseFlatTheme::bgVoid);
    g.fillRect(ca);

    // Center line (zero crossing)
    float midY = ca.getY() + ca.getHeight() * 0.5f;
    g.setColour(MultiverseFlatTheme::textMuted.withAlpha(0.35f));
    g.drawHorizontalLine(juce::roundToInt(midY), ca.getX(), ca.getRight());
    g.drawVerticalLine(juce::roundToInt(ca.getCentreX()), ca.getY(), ca.getBottom());

    // Waveform path
    juce::Path path;
    for (int i = 0; i < 256; ++i)
    {
        float x = ca.getX() + (float)i / 255.0f * ca.getWidth();
        float y = ca.getY() + (1.0f - (table[i] * 0.5f + 0.5f)) * ca.getHeight();
        y = juce::jlimit(ca.getY(), ca.getBottom(), y);
        if (i == 0) path.startNewSubPath(x, y);
        else        path.lineTo(x, y);
    }

    // Fill
    juce::Path fill = path;
    fill.lineTo(ca.getRight(), midY);
    fill.lineTo(ca.getX(),     midY);
    fill.closeSubPath();
    g.setColour(MultiverseFlatTheme::accentBlue.withAlpha(0.15f));
    g.fillPath(fill);

    // Stroke
    g.setColour(MultiverseFlatTheme::accentBlue);
    g.strokePath(path, juce::PathStrokeType(1.5f));

    // Border
    g.setColour(MultiverseFlatTheme::bgRaised);
    g.drawRect(canvasArea, 1);
}

void LFOShapeEditor::resized()
{
    auto b = getLocalBounds().reduced(4);

    auto toolbar = b.removeFromTop(22);
    penBtn .setBounds(toolbar.removeFromLeft(36));
    toolbar.removeFromLeft(2);
    lineBtn.setBounds(toolbar.removeFromLeft(40));
    toolbar.removeFromLeft(8);
    sinBtn .setBounds(toolbar.removeFromLeft(32));
    toolbar.removeFromLeft(2);
    sawBtn .setBounds(toolbar.removeFromLeft(32));
    toolbar.removeFromLeft(2);
    sqrBtn .setBounds(toolbar.removeFromLeft(32));
    toolbar.removeFromLeft(2);
    triBtn .setBounds(toolbar.removeFromLeft(32));
    toolbar.removeFromLeft(8);
    normBtn.setBounds(toolbar.removeFromLeft(42));

    b.removeFromTop(4);
    canvasArea = b;
}

void LFOShapeEditor::mouseDown(const juce::MouseEvent& e)
{
    if (!canvasArea.contains(e.getPosition())) return;

    float nx = juce::jlimit(0.0f, 1.0f, (float)(e.x - canvasArea.getX()) / (float)canvasArea.getWidth());
    float ny = juce::jlimit(0.0f, 1.0f, (float)(e.y - canvasArea.getY()) / (float)canvasArea.getHeight());

    if (currentTool == 0)
    {
        writeAtX(nx, ny);
        notifyChanged();
        repaint();
    }
    else
    {
        lineStartNorm = { nx, ny };
        lineDrawing = true;
    }
}

void LFOShapeEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (currentTool != 0) return;
    if (!canvasArea.contains(e.getPosition()) && !e.mouseWasDraggedSinceMouseDown()) return;

    float nx = juce::jlimit(0.0f, 1.0f, (float)(e.x - canvasArea.getX()) / (float)canvasArea.getWidth());
    float ny = juce::jlimit(0.0f, 1.0f, (float)(e.y - canvasArea.getY()) / (float)canvasArea.getHeight());
    writeAtX(nx, ny);
    notifyChanged();
    repaint();
}

void LFOShapeEditor::mouseUp(const juce::MouseEvent& e)
{
    if (currentTool == 1 && lineDrawing)
    {
        float nx = juce::jlimit(0.0f, 1.0f, (float)(e.x - canvasArea.getX()) / (float)canvasArea.getWidth());
        float ny = juce::jlimit(0.0f, 1.0f, (float)(e.y - canvasArea.getY()) / (float)canvasArea.getHeight());
        writeLine(lineStartNorm.x, lineStartNorm.y, nx, ny);
        notifyChanged();
        repaint();
        lineDrawing = false;
    }
}

int LFOShapeEditor::xToIndex(float normX) const
{
    return juce::jlimit(0, 255, (int)(normX * 255.0f + 0.5f));
}

float LFOShapeEditor::yToValue(float normY) const
{
    // normY: 0=top=+1, 1=bottom=-1
    return 1.0f - normY * 2.0f;
}

void LFOShapeEditor::writeAtX(float normX, float normY)
{
    table[xToIndex(normX)] = yToValue(normY);
}

void LFOShapeEditor::writeLine(float x0, float y0, float x1, float y1)
{
    int i0 = xToIndex(x0);
    int i1 = xToIndex(x1);
    if (i0 > i1) { std::swap(i0, i1); std::swap(y0, y1); }
    if (i0 == i1) { table[i0] = yToValue(y0); return; }
    for (int i = i0; i <= i1; ++i)
    {
        float t = (float)(i - i0) / (float)(i1 - i0);
        table[i] = yToValue(y0 + t * (y1 - y0));
    }
}

void LFOShapeEditor::fillSine()
{
    for (int i = 0; i < 256; ++i)
        table[i] = std::sin(2.0f * 3.14159265f * (float)i / 256.0f);
}

void LFOShapeEditor::fillSaw()
{
    for (int i = 0; i < 256; ++i)
        table[i] = (float)i / 128.0f - 1.0f;
}

void LFOShapeEditor::fillSquare()
{
    for (int i = 0; i < 256; ++i)
        table[i] = (i < 128) ? 1.0f : -1.0f;
}

void LFOShapeEditor::fillTri()
{
    for (int i = 0; i < 256; ++i)
        table[i] = (i < 128) ? (float)i / 64.0f - 1.0f
                              : 3.0f - (float)i / 64.0f;
}

void LFOShapeEditor::normalize()
{
    float maxAbs = 0.0f;
    for (auto v : table) maxAbs = std::max(maxAbs, std::abs(v));
    if (maxAbs < 0.0001f) return;
    for (auto& v : table) v /= maxAbs;
}

void LFOShapeEditor::notifyChanged()
{
    if (onTableChanged) onTableChanged(table);
}
