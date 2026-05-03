#include "NeuKnob.h"

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
        setColour (juce::Slider::rotarySliderFillColourId, MultiverseTheme::accentAmber);
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
    if (!(isMouseOver() || isMouseButtonDown())) return;

    const juce::String text = getTextFromValue (getValue());
    g.setFont (juce::Font (juce::FontOptions{}.withHeight (9.0f)));
    const float pillW = std::max (40.0f, (float) text.length() * 5.8f + 14.0f);
    const float pillH = 15.0f;
    const float pillX = (getWidth()  - pillW) * 0.5f;
    const float pillY = 2.0f;

    auto pill = juce::Rectangle<float> (pillX, pillY, pillW, pillH);
    g.setColour (MultiverseTheme::bgDeep);
    g.fillRoundedRectangle (pill, pillH * 0.5f);
    g.setColour (MultiverseTheme::accentBlue.withAlpha (0.5f));
    g.drawRoundedRectangle (pill, pillH * 0.5f, 1.0f);
    g.setColour (MultiverseTheme::textPrimary);
    g.drawText (text, pill.toNearestInt(), juce::Justification::centred, false);
}
