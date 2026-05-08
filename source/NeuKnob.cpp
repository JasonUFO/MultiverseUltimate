#include "NeuKnob.h"
#include "MultiverseFlatTheme.h"
#include "Synth/ModulationMatrix.h"
#include "PluginProcessor.h"

NeuKnob::NeuKnob()
{
    arcTimer.startTimerHz (10);
}

NeuKnob::~NeuKnob()
{
    arcTimer.stopTimer();
}

void NeuKnob::arcTimerTick()
{
    const bool assigned = isMacroAssigned();
    if (assigned == lastMacroState) return;

    lastMacroState = assigned;
    if (assigned)
        setColour (juce::Slider::rotarySliderFillColourId, MultiverseFlatTheme::accentAmber);
    else
        removeColour (juce::Slider::rotarySliderFillColourId);
}

void NeuKnob::paint (juce::Graphics& g)
{
    MidiLearnSlider::paint (g);

    const auto style = getSliderStyle();
    const bool isRotary = (style == juce::Slider::Rotary ||
                           style == juce::Slider::RotaryHorizontalDrag ||
                           style == juce::Slider::RotaryVerticalDrag ||
                           style == juce::Slider::RotaryHorizontalVerticalDrag);

    if (!isRotary) return;

    // Modulation depth arc — pink outer arc showing total mod depth
    if (getParamID().isNotEmpty() && getProcessor() != nullptr)
    {
        auto mapping = paramIDToModTarget(getParamID());
        if (mapping.has_value())
        {
            auto& matrix = getProcessor()->getModulationMatrix();
            auto connections = matrix.getActiveConnectionsForTarget(mapping->target, mapping->targetIndex);
            float totalDepth = 0.0f;
            for (const auto& c : connections)
                totalDepth += std::abs(c.amount);

            if (totalDepth > 0.001f)
            {
                const float cx = getWidth() * 0.5f;
                const float cy = getHeight() * 0.5f;
                const float radius = std::min(getWidth(), getHeight()) * 0.42f;
                const float lineW = 3.0f;

                const float startAngle = juce::MathConstants<float>::pi * 1.25f;
                const float endAngle   = juce::MathConstants<float>::pi * 2.75f;
                const float range      = endAngle - startAngle;
                const float modEnd     = startAngle + range * juce::jmin(totalDepth, 1.0f);

                g.setColour(MultiverseFlatTheme::accentPink.withAlpha(0.6f));
                g.drawEllipse(cx - radius - lineW, cy - radius - lineW,
                              (radius + lineW) * 2.0f, (radius + lineW) * 2.0f, lineW);

                juce::Path modArc;
                modArc.addArc(cx - radius - lineW, cy - radius - lineW,
                              (radius + lineW) * 2.0f, (radius + lineW) * 2.0f,
                              startAngle, modEnd, true);
                g.strokePath(modArc, juce::PathStrokeType(lineW));
            }
        }
    }

    // Drag-over highlight ring
    if (isDragOver)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float radius = std::min(getWidth(), getHeight()) * 0.46f;
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.5f));
        g.drawRoundedRectangle(juce::Rectangle<float>(cx - radius, cy - radius, radius * 2, radius * 2), radius * 0.3f, 2.5f);
    }

    if (!(isMouseOver() || isMouseButtonDown())) return;

    const juce::String text = getTextFromValue (getValue());
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (9.0f)));
    const float pillW = std::max (40.0f, (float) text.length() * 5.8f + 14.0f);
    const float pillH = 15.0f;
    const float pillX = (getWidth()  - pillW) * 0.5f;
    const float pillY = 2.0f;

    auto pill = juce::Rectangle<float> (pillX, pillY, pillW, pillH);
    g.setColour (MultiverseFlatTheme::bgDeep);
    g.fillRoundedRectangle (pill, pillH * 0.5f);
    g.setColour (MultiverseFlatTheme::accentBlue.withAlpha (0.5f));
    g.drawRoundedRectangle (pill, pillH * 0.5f, 1.0f);
    g.setColour (MultiverseFlatTheme::textPrimary);
    g.drawText (text, pill.toNearestInt(), juce::Justification::centred, false);
}

bool NeuKnob::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    return details.description.toString().startsWith("modsrc:");
}

void NeuKnob::itemDragEnter(const juce::DragAndDropTarget::SourceDetails&)
{
    isDragOver = true;
    repaint();
}

void NeuKnob::itemDragExit(const juce::DragAndDropTarget::SourceDetails&)
{
    isDragOver = false;
    repaint();
}

void NeuKnob::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    isDragOver = false;
    repaint();

    auto desc = details.description.toString();
    if (!desc.startsWith("modsrc:")) return;
    int sourceInt = desc.substring(7).getIntValue();
    auto source = static_cast<ModSourceType>(sourceInt);

    if (getParamID().isEmpty() || getProcessor() == nullptr) return;
    auto mapping = paramIDToModTarget(getParamID());
    if (!mapping.has_value()) return;

    getProcessor()->getModulationMatrix().addConnection(source, mapping->target, 0.5f);
}
