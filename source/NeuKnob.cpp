#include "NeuKnob.h"
#include "MultiverseFlatTheme.h"
#include "Synth/ModulationMatrix.h"
#include "PluginProcessor.h"

bool NeuKnob::modDragActive = false;
ModSourceType NeuKnob::modDragSource = ModSourceType::LFO1;

juce::Colour NeuKnob::getModSourceColour(ModSourceType type)
{
    // Palette rebuilt each call to pick up skin changes
    const juce::Colour palette[] = {
        MultiverseFlatTheme::accentCyan(),    // LFO1
        MultiverseFlatTheme::accentPink(),    // LFO2
        MultiverseFlatTheme::accentPurple(),  // LFO3
        MultiverseFlatTheme::accentGreen(),   // LFO4
        MultiverseFlatTheme::accentAmber(),   // Envelope
        MultiverseFlatTheme::accentCyan(),    // Velocity
        MultiverseFlatTheme::accentPurple(),  // NoteNumber
        MultiverseFlatTheme::accentPink(),    // Aftertouch
        MultiverseFlatTheme::accentGreen(),   // ModWheel
        MultiverseFlatTheme::accentAmber(),   // PitchBend
        MultiverseFlatTheme::accentPurple(),  // Random
        MultiverseFlatTheme::accentCyan(),    // EnvelopeFollower
        MultiverseFlatTheme::accentGreen(),   // MPEPressure
        MultiverseFlatTheme::accentAmber(),   // MPESlide
        MultiverseFlatTheme::accentPink(),    // SequencerStep
        MultiverseFlatTheme::accentAmber(),   // LFO5
        MultiverseFlatTheme::accentCyan(),    // LFO6
        MultiverseFlatTheme::accentPink(),    // LFO7
        MultiverseFlatTheme::accentPurple(),  // LFO8
        MultiverseFlatTheme::accentGreen(),   // Envelope2
        MultiverseFlatTheme::accentAmber()    // Envelope3
    };
    const int idx = static_cast<int>(type);
    if (idx >= 0 && idx < 21)
        return palette[idx];
    return MultiverseFlatTheme::accentCyan();
}

NeuKnob::NeuKnob()
{
    arcTimer.startTimerHz(10);
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
        setColour(juce::Slider::rotarySliderFillColourId, MultiverseFlatTheme::accentAmber());
    else
        removeColour(juce::Slider::rotarySliderFillColourId);
}

void NeuKnob::paint(juce::Graphics& g)
{
    MidiLearnSlider::paint(g);

    const auto style = getSliderStyle();
    const bool isRotary = (style == juce::Slider::Rotary ||
                           style == juce::Slider::RotaryHorizontalDrag ||
                           style == juce::Slider::RotaryVerticalDrag ||
                           style == juce::Slider::RotaryHorizontalVerticalDrag);

    if (!isRotary) return;

    // Per-source modulation arcs — each source gets its own coloured arc
    if (getParamID().isNotEmpty() && getProcessor() != nullptr)
    {
        auto mapping = paramIDToModTarget(getParamID());
        if (mapping.has_value())
        {
            auto& matrix = getProcessor()->getModulationMatrix();
            auto connections = matrix.getActiveConnectionsForTarget(mapping->target, mapping->targetIndex);

            if (!connections.empty())
            {
                const float cx = getWidth() * 0.5f;
                const float cy = getHeight() * 0.5f;
                const float startAngle = juce::MathConstants<float>::pi * 1.25f;
                const float endAngle   = juce::MathConstants<float>::pi * 2.75f;
                const float range      = endAngle - startAngle;

                // Draw each connection as a coloured arc at increasing radii
                const float baseRadius = std::min(getWidth(), getHeight()) * 0.42f;
                const float arcSpacing = 3.0f;
                const float lineWidth  = 2.5f;

                for (size_t i = 0; i < connections.size(); ++i)
                {
                    const auto& conn = connections[static_cast<int>(i)];
                    const float depth = std::abs(conn.amount);
                    if (depth < 0.001f) continue;

                    const float radius = baseRadius + i * arcSpacing;
                    const float modEnd = startAngle + range * juce::jmin(depth, 1.0f);
                    const juce::Colour srcColour = getModSourceColour(conn.source);

                    // Background track arc (dimmed)
                    g.setColour(srcColour.withAlpha(0.15f));
                    g.drawEllipse(cx - radius - lineWidth * 0.5f,
                                  cy - radius - lineWidth * 0.5f,
                                  (radius + lineWidth * 0.5f) * 2.0f,
                                  (radius + lineWidth * 0.5f) * 2.0f,
                                  lineWidth);

                    // Outer glow (bloom)
                    juce::Path glowArc;
                    glowArc.addArc(cx - radius - lineWidth * 0.5f,
                                   cy - radius - lineWidth * 0.5f,
                                   (radius + lineWidth * 0.5f) * 2.0f,
                                   (radius + lineWidth * 0.5f) * 2.0f,
                                   startAngle, modEnd, true);
                    g.setColour(srcColour.withAlpha(0.15f));
                    g.strokePath(glowArc, juce::PathStrokeType(lineWidth + 4.0f));

                    // Active modulation arc
                    juce::Path modArc;
                    modArc.addArc(cx - radius - lineWidth * 0.5f,
                                  cy - radius - lineWidth * 0.5f,
                                  (radius + lineWidth * 0.5f) * 2.0f,
                                  (radius + lineWidth * 0.5f) * 2.0f,
                                  startAngle, modEnd, true);
                    g.setColour(srcColour.withAlpha(0.85f));
                    g.strokePath(modArc, juce::PathStrokeType(lineWidth));
                }
            }
        }
    }

    // Drag-over highlight ring — Avenger-style glow with source colour
    if (isDragOver)
    {
        const float cx = getWidth() * 0.5f;
        const float cy = getHeight() * 0.5f;
        const float radius = std::min(getWidth(), getHeight()) * 0.46f;
        const juce::Colour hlColour = dragSourceColour.isTransparent()
            ? MultiverseFlatTheme::accentCyan() : dragSourceColour;
        auto bounds = juce::Rectangle<float>(cx - radius, cy - radius, radius * 2, radius * 2);

        // Outer bloom
        MultiverseFlatTheme::drawGlow(g, bounds.expanded(4.0f), hlColour, radius * 0.3f + 4.0f, 0.25f);
        // Inner glow ring
        g.setColour(hlColour.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, radius * 0.3f, 2.5f);
        // Specular ring
        g.setColour(hlColour.brighter(0.5f).withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.reduced(2.0f), radius * 0.3f - 2.0f, 1.0f);
    }
    else if (modDragActive && getParamID().isNotEmpty() && getProcessor() != nullptr)
    {
        // Global mod-drag active: highlight all valid drop targets with a subtle glow
        auto mapping = paramIDToModTarget(getParamID());
        if (mapping.has_value())
        {
            const float cx = getWidth() * 0.5f;
            const float cy = getHeight() * 0.5f;
            const float radius = std::min(getWidth(), getHeight()) * 0.46f;
            const juce::Colour srcColour = getModSourceColour(modDragSource);
            // Subtle pulse glow
            MultiverseFlatTheme::drawGlow(g,
                juce::Rectangle<float>(cx - radius, cy - radius, radius * 2, radius * 2),
                srcColour, radius * 0.3f, 0.1f);
            g.setColour(srcColour.withAlpha(0.3f));
            g.drawRoundedRectangle(juce::Rectangle<float>(cx - radius, cy - radius, radius * 2, radius * 2), radius * 0.3f, 1.5f);
        }
    }

    if (!(isMouseOver() || isMouseButtonDown())) return;

    const juce::String text = getTextFromValue(getValue());
    g.setFont(juce::Font(juce::FontOptions{}.withHeight(9.0f)));
    const float pillW = std::max(40.0f, (float) text.length() * 5.8f + 14.0f);
    const float pillH = 15.0f;
    const float pillX = (getWidth()  - pillW) * 0.5f;
    const float pillY = 2.0f;

    auto pill = juce::Rectangle<float>(pillX, pillY, pillW, pillH);

    // 3D metallic pill: inset background + bevel + accent glow
    MultiverseFlatTheme::drawInset(g, pill, pillH * 0.5f);
    MultiverseFlatTheme::drawGradientFill(g, pill.reduced(0.5f));
    g.setColour(MultiverseFlatTheme::accentBlue().withAlpha(0.4f));
    g.drawRoundedRectangle(pill, pillH * 0.5f, 1.0f);
    // Specular highlight along top edge
    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawHorizontalLine(static_cast<int>(pillY + 2.0f),
                          pillX + 6.0f, pillX + pillW - 6.0f);
    g.setColour(MultiverseFlatTheme::textPrimary());
    g.drawText(text, pill.toNearestInt(), juce::Justification::centred, false);
}

bool NeuKnob::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    return details.description.toString().startsWith("modsrc:");
}

void NeuKnob::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& details)
{
    isDragOver = true;
    // Set global mod-drag state so all knobs can show highlight
    auto desc = details.description.toString();
    if (desc.startsWith("modsrc:"))
    {
        int sourceInt = desc.substring(7).upToFirstOccurrenceOf(":", false, false).getIntValue();
        auto source = static_cast<ModSourceType>(sourceInt);
        modDragActive = true;
        modDragSource = source;
        dragSourceColour = getModSourceColour(source);
    }
    repaint();
}

void NeuKnob::itemDragExit(const juce::DragAndDropTarget::SourceDetails&)
{
    isDragOver = false;
    dragSourceColour = juce::Colour();
    modDragActive = false;
    repaint();
}

void NeuKnob::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    isDragOver = false;
    dragSourceColour = juce::Colour();
    modDragActive = false;
    repaint();

    auto desc = details.description.toString();
    if (!desc.startsWith("modsrc:")) return;
    int sourceInt = desc.substring(7).upToFirstOccurrenceOf(":", false, false).getIntValue();
    if (sourceInt == 0 && desc.substring(7) != "0") sourceInt = desc.substring(7).getIntValue();
    auto source = static_cast<ModSourceType>(sourceInt);

    if (getParamID().isEmpty() || getProcessor() == nullptr) return;
    auto mapping = paramIDToModTarget(getParamID());
    if (!mapping.has_value()) return;

    getProcessor()->getModulationMatrix().addConnection(source, mapping->target, 0.5f);
}