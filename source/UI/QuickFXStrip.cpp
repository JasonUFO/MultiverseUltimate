#include "QuickFXStrip.h"
#include "../PluginProcessor.h"

QuickFXStrip::QuickFXStrip(PluginProcessor& p) : proc(p)
{
    auto& apvts = p.apvts;

    // Enable toggles
    auto setupToggle = [&](juce::ToggleButton& btn)
    {
        btn.setToggleState(false, juce::dontSendNotification);
        btn.setColour(juce::ToggleButton::tickColourId, MultiverseFlatTheme::accentCyan);
        btn.setColour(juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary);
        addAndMakeVisible(btn);
    };
    setupToggle(filterModEnable);
    setupToggle(ampModEnable);
    setupToggle(mainFilterEnable);

    filterModEnableAttach = std::make_unique<ButtonAttach>(apvts, "filterModEnabled", filterModEnable);
    ampModEnableAttach    = std::make_unique<ButtonAttach>(apvts, "ampModEnabled", ampModEnable);
    mainFilterEnableAttach = std::make_unique<ButtonAttach>(apvts, "mainFilterEnabled", mainFilterEnable);

    // Filter Mod knobs
    setupKnob(fmCutoff,    "filterModCutoff",    "CUT");
    setupKnob(fmResonance, "filterModResonance", "RES");
    setupKnob(fmEnvDepth,  "filterModEnvDepth",  "ENV");

    // Amp Mod knobs
    setupKnob(amVolume, "ampModVolume", "VOL");
    setupKnob(amPan,    "ampModPan",    "PAN");

    // Delay knobs
    setupKnob(delayMix,      "delayMix",      "MIX");
    setupKnob(delayTime,     "delayTime",     "TIM");
    setupKnob(delayFeedback, "delayFeedback",  "FDB");

    // Reverb knobs
    setupKnob(reverbWet,  "reverbWet",  "WET");
    setupKnob(reverbRoom, "reverbRoom", "ROM");
    setupKnob(reverbDamp, "reverbDamp", "DMP");

    // Main Filter knobs
    setupKnob(mfCutoff,    "mainFilterCutoff",    "CUT");
    setupKnob(mfResonance, "mainFilterResonance", "RES");

    mfTypeCombo.addItemList({"LP", "HP", "BP", "Notch"}, 1);
    mfTypeCombo.setTooltip("Main Filter Type");
    addAndMakeVisible(mfTypeCombo);
    mfTypeAttach = std::make_unique<ComboAttach>(apvts, "mainFilterType", mfTypeCombo);

    // Section headers
    const auto hdrFont = juce::Font(juce::FontOptions{}.withHeight(9.0f));
    auto setupHeader = [&](juce::Label& lbl)
    {
        lbl.setFont(hdrFont);
        lbl.setColour(juce::Label::textColourId, MultiverseFlatTheme::accentCyan);
        lbl.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(lbl);
    };
    setupHeader(filterModLabel);
    setupHeader(ampModLabel);
    setupHeader(delayLabel);
    setupHeader(reverbLabel);
    setupHeader(mainFilterLabel);
}

void QuickFXStrip::setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& shortName)
{
    g.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    g.knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(g.knob);

    g.attach = std::make_unique<SliderAttach>(proc.apvts, paramID, g.knob);
    g.knob.init(proc, paramID);

    g.label.setText(shortName, juce::dontSendNotification);
    g.label.setFont(juce::Font(juce::FontOptions{}.withHeight(8.0f)));
    g.label.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    g.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(g.label);
}

void QuickFXStrip::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);

    // Draw section cards
    for (const auto& rect : { filterModRect, ampModRect, delayRect, reverbRect, mainFilterRect })
        MultiverseFlatTheme::drawCard(g, rect.toFloat(), 6.0f);
}

void QuickFXStrip::resized()
{
    auto area = getLocalBounds().reduced(6, 6);
    const int w = area.getWidth();
    const int knobRowH = KNOB_SZ + LABEL_H + 2;
    const int knobColW = (w - 4) / 2;

    // Helper: lay out 2 knobs side-by-side
    auto layoutKnobPair = [&](KnobGroup& left, KnobGroup& right, juce::Rectangle<int> row)
    {
        auto lCol = row.removeFromLeft(knobColW);
        auto rCol = row;

        left.knob.setBounds(lCol.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
        left.label.setBounds(lCol.removeFromTop(LABEL_H));

        right.knob.setBounds(rCol.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
        right.label.setBounds(rCol.removeFromTop(LABEL_H));
    };

    // Helper: lay out a single centered knob
    auto layoutKnobSingle = [&](KnobGroup& k, juce::Rectangle<int> row)
    {
        k.knob.setBounds(row.withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
        k.label.setBounds(row.withTop(row.getBottom() - LABEL_H).withSizeKeepingCentre(30, LABEL_H));
    };

    // Helper: section header row
    auto layoutHeader = [&](juce::Label& lbl, juce::ToggleButton* enable, juce::Rectangle<int> row)
    {
        auto left = row.removeFromLeft(w - 44);
        lbl.setBounds(left.removeFromTop(16));
        if (enable)
            enable->setBounds(row.removeFromRight(36).withSizeKeepingCentre(28, TOGGLE_H));
    };

    // ── FILTER MOD ──
    {
        filterModRect = area.removeFromTop(16 + TOGGLE_H + knobRowH * 2 + 6);
        auto sec = filterModRect.reduced(4, 2);
        layoutHeader(filterModLabel, &filterModEnable, sec.removeFromTop(16));
        layoutKnobPair(fmCutoff, fmResonance, sec.removeFromTop(knobRowH));
        layoutKnobSingle(fmEnvDepth, sec.removeFromTop(knobRowH));
    }

    area.removeFromTop(SECTION_GAP);

    // ── AMP MOD ──
    {
        ampModRect = area.removeFromTop(16 + TOGGLE_H + knobRowH + 2);
        auto sec = ampModRect.reduced(4, 2);
        layoutHeader(ampModLabel, &ampModEnable, sec.removeFromTop(16));
        layoutKnobPair(amVolume, amPan, sec.removeFromTop(knobRowH));
    }

    area.removeFromTop(SECTION_GAP);

    // ── DELAY ──
    {
        delayRect = area.removeFromTop(16 + knobRowH * 2 + 4);
        auto sec = delayRect.reduced(4, 2);
        layoutHeader(delayLabel, nullptr, sec.removeFromTop(16));
        layoutKnobPair(delayMix, delayTime, sec.removeFromTop(knobRowH));
        layoutKnobSingle(delayFeedback, sec.removeFromTop(knobRowH));
    }

    area.removeFromTop(SECTION_GAP);

    // ── REVERB ──
    {
        reverbRect = area.removeFromTop(16 + knobRowH * 2 + 4);
        auto sec = reverbRect.reduced(4, 2);
        layoutHeader(reverbLabel, nullptr, sec.removeFromTop(16));
        layoutKnobPair(reverbWet, reverbRoom, sec.removeFromTop(knobRowH));
        layoutKnobSingle(reverbDamp, sec.removeFromTop(knobRowH));
    }

    area.removeFromTop(SECTION_GAP);

    // ── MAIN FILTER ──
    {
        mainFilterRect = area.removeFromTop(16 + TOGGLE_H + COMBO_H + knobRowH + 4);
        auto sec = mainFilterRect.reduced(4, 2);
        layoutHeader(mainFilterLabel, &mainFilterEnable, sec.removeFromTop(16));
        mfTypeCombo.setBounds(sec.removeFromTop(COMBO_H));
        layoutKnobPair(mfCutoff, mfResonance, sec.removeFromTop(knobRowH));
    }
}