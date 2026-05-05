#pragma once
#include <JuceHeader.h>
#include <array>
#include <functional>

class LFOShapeEditor : public juce::Component
{
public:
    LFOShapeEditor();

    std::function<void(const std::array<float, 256>&)> onTableChanged;

    void setTable(const std::array<float, 256>& table);
    const std::array<float, 256>& getTable() const { return table; }

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    std::array<float, 256> table;
    int  currentTool   = 0;     // 0=pencil, 1=line
    bool lineDrawing   = false;
    juce::Point<float> lineStartNorm;

    juce::TextButton penBtn  { "PEN" };
    juce::TextButton lineBtn { "LINE" };
    juce::TextButton sinBtn  { "SIN" };
    juce::TextButton sawBtn  { "SAW" };
    juce::TextButton sqrBtn  { "SQR" };
    juce::TextButton triBtn  { "TRI" };
    juce::TextButton normBtn { "NORM" };

    juce::Rectangle<int> canvasArea;

    void writeAtX(float normX, float normY);
    void writeLine(float x0, float y0, float x1, float y1);
    void fillSine();
    void fillSaw();
    void fillSquare();
    void fillTri();
    void normalize();
    void notifyChanged();

    int   xToIndex(float normX) const;
    float yToValue(float normY) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOShapeEditor)
};
