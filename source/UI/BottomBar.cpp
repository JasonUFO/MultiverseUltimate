#include "BottomBar.h"
#include "../PluginProcessor.h"
#include "../Synth/LFOShapeEditor.h"

//==============================================================================
// PitchWheel / ModWheel (unchanged)

void PitchWheel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(MultiverseFlatTheme::bgDeep);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(MultiverseFlatTheme::borderLight.withAlpha(0.4f));
    g.drawRoundedRectangle(bounds, 4.0f, 0.8f);
    const float cy = bounds.getCentreY();
    g.setColour(MultiverseFlatTheme::textMuted.withAlpha(0.3f));
    g.drawHorizontalLine(static_cast<int>(cy), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);
    const float thumbH = 14.0f;
    const float thumbY = cy - value * (bounds.getHeight() / 2.0f - thumbH / 2.0f) - thumbH / 2.0f;
    auto thumb = juce::Rectangle<float>(bounds.getX() + 3.0f, thumbY, bounds.getWidth() - 6.0f, thumbH);
    g.setColour(value != 0.0f ? MultiverseFlatTheme::accentCyan : MultiverseFlatTheme::bgRaised);
    g.fillRoundedRectangle(thumb, 3.0f);
    g.setColour(value != 0.0f ? MultiverseFlatTheme::accentCyan.withAlpha(0.6f) : MultiverseFlatTheme::borderLight);
    g.drawRoundedRectangle(thumb, 3.0f, 0.8f);
}

void PitchWheel::mouseDown(const juce::MouseEvent& e) { lastY = e.y; }
void PitchWheel::mouseDrag(const juce::MouseEvent& e)
{
    const float range = static_cast<float>(getHeight()) / 2.0f;
    value = juce::jlimit(-1.0f, 1.0f, -static_cast<float>(e.y - lastY) / range + value);
    lastY = e.y;
    repaint();
    if (onValueChange) onValueChange(value);
}
void PitchWheel::mouseUp(const juce::MouseEvent&)
{
    value = 0.0f;
    repaint();
    if (onValueChange) onValueChange(value);
}

void ModWheel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(MultiverseFlatTheme::bgDeep);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(MultiverseFlatTheme::borderLight.withAlpha(0.4f));
    g.drawRoundedRectangle(bounds, 4.0f, 0.8f);
    const float fillH = value * (bounds.getHeight() - 12.0f);
    const float fillY = bounds.getBottom() - 6.0f - fillH;
    if (value > 0.001f)
    {
        auto fill = juce::Rectangle<float>(bounds.getX() + 3.0f, fillY, bounds.getWidth() - 6.0f, fillH);
        g.setColour(MultiverseFlatTheme::accentCyan.withAlpha(0.25f));
        g.fillRoundedRectangle(fill, 2.0f);
    }
    const float thumbH = 14.0f;
    const float thumbY = bounds.getBottom() - 6.0f - value * (bounds.getHeight() - 12.0f) - thumbH / 2.0f;
    auto thumb = juce::Rectangle<float>(bounds.getX() + 3.0f, thumbY, bounds.getWidth() - 6.0f, thumbH);
    g.setColour(value > 0.001f ? MultiverseFlatTheme::accentCyan : MultiverseFlatTheme::bgRaised);
    g.fillRoundedRectangle(thumb, 3.0f);
    g.setColour(value > 0.001f ? MultiverseFlatTheme::accentCyan.withAlpha(0.6f) : MultiverseFlatTheme::borderLight);
    g.drawRoundedRectangle(thumb, 3.0f, 0.8f);
}

void ModWheel::mouseDown(const juce::MouseEvent& e)
{
    const float boundsH = static_cast<float>(getHeight() - 12);
    value = juce::jlimit(0.0f, 1.0f, (boundsH / 2.0f + 6.0f - static_cast<float>(e.y)) / boundsH);
    repaint();
    if (onValueChange) onValueChange(value);
}
void ModWheel::mouseDrag(const juce::MouseEvent& e)
{
    const float boundsH = static_cast<float>(getHeight() - 12);
    value = juce::jlimit(0.0f, 1.0f, (boundsH / 2.0f + 6.0f - static_cast<float>(e.y)) / boundsH);
    repaint();
    if (onValueChange) onValueChange(value);
}

//==============================================================================
// EnvSubPanel

EnvSubPanel::EnvSubPanel(PluginProcessor& p, int idx)
    : proc(p), envIndex(idx),
      envelopeDisplay()
{
    auto& apvts = p.apvts;

    // Select parameter IDs based on envelope index
    juce::String prefix = (idx == 0) ? "" : (idx == 1) ? "modEnv2" : "modEnv3";
    juce::String aID = (idx == 0) ? "attack"  : prefix + "Attack";
    juce::String dID = (idx == 0) ? "decay"   : prefix + "Decay";
    juce::String sID = (idx == 0) ? "sustain" : prefix + "Sustain";
    juce::String rID = (idx == 0) ? "release" : prefix + "Release";

    auto setupKnob = [&](KnobGroup& g, const juce::String& paramID, const juce::String& name)
    {
        g.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        g.knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(g.knob);
        g.attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, g.knob);
        g.knob.init(p, paramID);
        g.label.setText(name, juce::dontSendNotification);
        g.label.setFont(MultiverseFlatTheme::labelFont());
        g.label.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
        g.label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(g.label);
    };

    setupKnob(attack,  aID, "A");
    setupKnob(decay,   dID, "D");
    setupKnob(sustain, sID, "S");
    setupKnob(release, rID, "R");

    envelopeDisplay.setSliders(&attack.knob, &decay.knob, &sustain.knob, &release.knob);
    addAndMakeVisible(envelopeDisplay);
}

void EnvSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);
    // Section label
    const juce::String labels[] = { "AMP ENV", "ENV 2", "ENV 3" };
    g.setColour(MultiverseFlatTheme::accentCyan);
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText(labels[envIndex], getLocalBounds().removeFromTop(18).toFloat(),
              juce::Justification::centred);
}

void EnvSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    area.removeFromTop(18); // label

    // Display on left, knobs on right
    auto displayArea = area.removeFromLeft(area.getWidth() * 2 / 5);
    envelopeDisplay.setBounds(displayArea.reduced(2));

    // 4 knobs in a row on the right
    auto knobArea = area;
    const int knobW = knobArea.getWidth() / 4;
    auto setupKnobBounds = [&](KnobGroup& g, juce::Rectangle<int> col)
    {
        g.knob.setBounds(col.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
        col.removeFromTop(2);
        g.label.setBounds(col.removeFromTop(14));
    };

    setupKnobBounds(attack,  knobArea.removeFromLeft(knobW));
    setupKnobBounds(decay,   knobArea.removeFromLeft(knobW));
    setupKnobBounds(sustain, knobArea.removeFromLeft(knobW));
    setupKnobBounds(release, knobArea);
}

//==============================================================================
// LFOSubPanel

LFOSubPanel::LFOSubPanel(PluginProcessor& p)
    : proc(p)
{
    auto& apvts = p.apvts;

    // Bank selector buttons
    for (int i = 0; i < 8; ++i)
    {
        bankButtons[i].setButtonText("LFO" + juce::String(i + 1));
        bankButtons[i].setClickingTogglesState(false);
        bankButtons[i].setRadioGroupId(1001);
        bankButtons[i].addListener(this);
        addAndMakeVisible(bankButtons[i]);
    }
    bankButtons[0].setToggleState(true, juce::dontSendNotification);

    // Controls
    rateKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    rateKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(rateKnob);

    shapeCombo.addItemList({"Sine", "Triangle", "Saw", "Square", "S&H", "Custom"}, 1);
    addAndMakeVisible(shapeCombo);

    syncButton.setClickingTogglesState(true);
    addAndMakeVisible(syncButton);

    syncDivCombo.addItemList({"1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1"}, 1);
    addAndMakeVisible(syncDivCombo);

    drawButton.setTooltip("Open LFO shape editor");
    addAndMakeVisible(drawButton);

    rateLabel.setFont(MultiverseFlatTheme::labelFont());
    rateLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    shapeLabel.setFont(MultiverseFlatTheme::labelFont());
    shapeLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    shapeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(shapeLabel);

    drawButton.onClick = [this] { showShapeEditor(); };

    switchBank(0);
    startTimerHz(15);
}

void LFOSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);

    // LFO label
    g.setColour(MultiverseFlatTheme::accentCyan);
    g.setFont(MultiverseFlatTheme::headerFont());
    auto topArea = getLocalBounds().removeFromTop(18).toFloat();
    g.drawText("LFO " + juce::String(activeBank + 1), topArea, juce::Justification::centredLeft);

    // Waveform preview area
    if (wavePathDirty)
    {
        rebuildWavePath();
        wavePathDirty = false;
    }
    auto previewArea = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    previewArea.removeFromTop(22 + 26 + 4); // bank buttons + gap
    previewArea = previewArea.removeFromLeft(previewArea.getWidth() * 2 / 5);

    g.setColour(MultiverseFlatTheme::bgBase);
    g.fillRoundedRectangle(previewArea.toFloat().reduced(2), 4.0f);
    g.setColour(MultiverseFlatTheme::borderLight.withAlpha(0.4f));
    g.drawRoundedRectangle(previewArea.toFloat().reduced(2), 4.0f, 0.8f);

    auto previewInner = previewArea.reduced(4);
    g.setColour(MultiverseFlatTheme::accentCyan);
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));
}

void LFOSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);

    // Label row
    area.removeFromTop(18);

    // Bank selector row
    auto bankRow = area.removeFromTop(26);
    const int btnW = bankRow.getWidth() / 8;
    for (int i = 0; i < 8; ++i)
        bankButtons[i].setBounds(bankRow.removeFromLeft(btnW).reduced(1, 2));

    area.removeFromTop(4);

    // Left: waveform preview, Right: controls
    auto leftArea = area.removeFromLeft(area.getWidth() * 2 / 5);
    auto rightArea = area;

    // Controls layout (vertical: Rate knob, Shape combo, Sync row, Draw button)
    auto controlArea = rightArea;

    // Rate knob
    auto rateRow = controlArea.removeFromTop(KNOB_SZ + 16);
    rateKnob.setBounds(rateRow.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
    rateLabel.setBounds(rateRow.removeFromTop(14));

    controlArea.removeFromTop(4);

    // Shape combo
    auto shapeRow = controlArea.removeFromTop(22);
    shapeLabel.setBounds(shapeRow.removeFromLeft(40));
    shapeCombo.setBounds(shapeRow);

    controlArea.removeFromTop(4);

    // Sync row
    auto syncRow = controlArea.removeFromTop(22);
    syncButton.setBounds(syncRow.removeFromLeft(50));
    syncDivCombo.setBounds(syncRow);

    controlArea.removeFromTop(4);

    // Draw button
    drawButton.setBounds(controlArea.removeFromTop(24));
}

void LFOSubPanel::buttonClicked(juce::Button* btn)
{
    for (int i = 0; i < 8; ++i)
    {
        if (btn == &bankButtons[i])
        {
            switchBank(i);
            return;
        }
    }
}

void LFOSubPanel::switchBank(int newBank)
{
    activeBank = newBank;
    auto& apvts = proc.apvts;
    const juce::String idx = juce::String(newBank + 1);

    // Rebuild attachments
    rateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "lfo" + idx + "Rate", rateKnob);
    rateKnob.init(proc, "lfo" + idx + "Rate");

    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "lfo" + idx + "Shape", shapeCombo);

    syncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "lfo" + idx + "Sync", syncButton);

    syncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "lfo" + idx + "SyncDiv", syncDivCombo);

    // Update button colors
    for (int i = 0; i < 8; ++i)
    {
        const bool active = (i == newBank);
        bankButtons[i].setColour(juce::TextButton::buttonColourId,
            active ? MultiverseFlatTheme::accentCyan.withAlpha(0.2f) : MultiverseFlatTheme::bgRaised);
        bankButtons[i].setColour(juce::TextButton::textColourOffId,
            active ? MultiverseFlatTheme::accentCyan : MultiverseFlatTheme::textSecondary);
    }

    wavePathDirty = true;
    repaint();
}

void LFOSubPanel::rebuildWavePath()
{
    wavePath.clear();
    auto previewArea = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    previewArea.removeFromTop(22 + 26 + 4);
    previewArea = previewArea.removeFromLeft(previewArea.getWidth() * 2 / 5);
    auto inner = previewArea.reduced(6);

    if (inner.getWidth() <= 0 || inner.getHeight() <= 0) return;

    const float w = static_cast<float>(inner.getWidth());
    const float h = static_cast<float>(inner.getHeight());
    const float cy = inner.getY() + h / 2.0f;

    // Read LFO shape from APVTS
    auto* shapeParam = proc.apvts.getRawParameterValue("lfo" + juce::String(activeBank + 1) + "Shape");
    int shape = shapeParam ? static_cast<int>(shapeParam->load()) : 0;

    wavePath.startNewSubPath(inner.getX(), cy);

    for (int i = 0; i <= 100; ++i)
    {
        const float phase = static_cast<float>(i) / 100.0f * 2.0f * juce::MathConstants<float>::pi;
        float val = 0.0f;
        switch (shape)
        {
            case 0: val = std::sin(phase); break; // Sine
            case 1: val = 1.0f - 2.0f * std::abs(phase / juce::MathConstants<float>::pi - 1.0f); break; // Triangle
            case 2: val = phase / juce::MathConstants<float>::pi - 1.0f; break; // Saw
            case 3: val = phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f; break; // Square
            case 4: val = std::sin(phase + static_cast<float>(activeBank) * 0.7f); break; // S&H (approx)
            case 5: // Custom - read from table if available
            {
                auto* tableParam = proc.apvts.getRawParameterValue("lfo" + juce::String(activeBank + 1) + "Shape");
                val = std::sin(phase); // fallback for now
                break;
            }
            default: val = std::sin(phase); break;
        }
        const float x = inner.getX() + static_cast<float>(i) / 100.0f * w;
        const float y = cy - val * h / 2.0f;
        if (i == 0) wavePath.startNewSubPath(x, y);
        else wavePath.lineTo(x, y);
    }
}

void LFOSubPanel::timerCallback()
{
    wavePathDirty = true;
    repaint();
}

void LFOSubPanel::showShapeEditor()
{
    // Get the wavetable oscillator for LFO shape editing
    auto* shapeEditor = new LFOShapeEditor();
    auto* tableRaw = proc.apvts.getRawParameterValue("lfo" + juce::String(activeBank + 1) + "CustomTable");
    // For now, just launch the editor with default settings
    shapeEditor->setSize(310, 160);
    juce::CallOutBox::launchAsynchronously(
        std::unique_ptr<juce::Component>(shapeEditor),
        drawButton.getScreenBounds(), nullptr);
}

//==============================================================================
// MacroSubPanel

MacroSubPanel::MacroSubPanel(PluginProcessor& p) : proc(p)
{
    auto& apvts = p.apvts;
    auto& mgr   = p.getMacroManager();

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        auto& g = macros[i];
        g.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        g.knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        g.knob.setTooltip("Macro " + juce::String(i + 1)
            + " — right-click any parameter to assign it here");
        addAndMakeVisible(g.knob);

        g.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "macro" + juce::String(i + 1), g.knob);

        g.nameLabel.setText(mgr.getName(i), juce::dontSendNotification);
        g.nameLabel.setJustificationType(juce::Justification::centred);
        g.nameLabel.setFont(MultiverseFlatTheme::headerFont());
        g.nameLabel.setEditable(false, true);
        g.nameLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
        g.nameLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        g.nameLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        g.nameLabel.setTooltip("Double-click to rename this macro");
        g.nameLabel.onTextChange = [this, i]
        {
            proc.getMacroManager().setName(i, macros[i].nameLabel.getText());
        };
        addAndMakeVisible(g.nameLabel);

        g.valueLabel.setJustificationType(juce::Justification::centred);
        g.valueLabel.setFont(MultiverseFlatTheme::valueFont());
        g.valueLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textMuted);
        g.valueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        g.valueLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(g.valueLabel);
    }

    startTimerHz(30);
}

void MacroSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);
}

void MacroSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    const int cellW = area.getWidth() / MacroManager::NUM_MACROS;

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        auto cell = area.removeFromLeft(cellW);
        auto& g = macros[i];
        g.knob.setBounds(cell.removeFromTop(KNOB_SIZE).withSizeKeepingCentre(KNOB_SIZE, KNOB_SIZE));
        cell.removeFromTop(1);
        g.nameLabel.setBounds(cell.removeFromTop(14));
        g.valueLabel.setBounds(cell.removeFromTop(12));
    }
}

void MacroSubPanel::timerCallback()
{
    auto& mgr   = proc.getMacroManager();
    auto& apvts = proc.apvts;

    // Push macro values to assigned target params
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const auto targets = mgr.getTargets(i);
        if (targets.empty()) continue;

        const auto* rawMacro = apvts.getRawParameterValue("macro" + juce::String(i + 1));
        if (rawMacro == nullptr) continue;
        const float macroVal = rawMacro->load();

        for (const auto& t : targets)
        {
            auto* targetParam = apvts.getParameter(t.paramID);
            if (targetParam == nullptr) continue;
            const float norm = juce::jlimit(0.0f, 1.0f,
                t.minVal + macroVal * (t.maxVal - t.minVal));
            targetParam->setValueNotifyingHost(norm);
        }
    }

    // Update name and value labels
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const juce::String name = mgr.getName(i);
        if (macros[i].nameLabel.getText() != name)
            macros[i].nameLabel.setText(name, juce::dontSendNotification);

        const auto* rawMacro = proc.apvts.getRawParameterValue("macro" + juce::String(i + 1));
        if (rawMacro)
        {
            const int pct = juce::roundToInt(rawMacro->load() * 100.0f);
            macros[i].valueLabel.setText(juce::String(pct) + "%", juce::dontSendNotification);
        }
    }
}

void MacroSubPanel::showAssignmentList(int macroIdx)
{
    auto& mgr         = proc.getMacroManager();
    const auto targets = mgr.getTargets(macroIdx);

    juce::PopupMenu menu;
    menu.addSectionHeader(mgr.getName(macroIdx) + " assignments");

    if (targets.empty())
    {
        menu.addItem(1, "(no parameters assigned)", false);
    }
    else
    {
        for (int idx = 0; idx < static_cast<int>(targets.size()); ++idx)
        {
            juce::PopupMenu sub;
            sub.addItem(1000 + idx, "Remove");
            menu.addSubMenu(targets[idx].paramID, sub);
        }
    }

    menu.showMenuAsync(
        juce::PopupMenu::Options().withTargetComponent(&macros[macroIdx].knob),
        [this, macroIdx, targets](int result)
        {
            if (result >= 1000 && result < 1000 + static_cast<int>(targets.size()))
            {
                const int idx = result - 1000;
                proc.getMacroManager().removeAssignment(macroIdx, targets[idx].paramID);
            }
        });
}

//==============================================================================
// QuickFXSubPanel

QuickFXSubPanel::QuickFXSubPanel(PluginProcessor& p) : proc(p)
{
    auto& apvts = p.apvts;

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

    filterModEnableAttach  = std::make_unique<ButtonAttach>(apvts, "filterModEnabled", filterModEnable);
    ampModEnableAttach     = std::make_unique<ButtonAttach>(apvts, "ampModEnabled", ampModEnable);
    mainFilterEnableAttach = std::make_unique<ButtonAttach>(apvts, "mainFilterEnabled", mainFilterEnable);

    setupKnob(fmCutoff,    "filterModCutoff",    "CUT");
    setupKnob(fmResonance, "filterModResonance", "RES");
    setupKnob(fmEnvDepth,  "filterModEnvDepth",  "ENV");
    setupKnob(amVolume,    "ampModVolume",       "VOL");
    setupKnob(amPan,       "ampModPan",           "PAN");
    setupKnob(delayMix,    "delayMix",            "MIX");
    setupKnob(delayTime,   "delayTime",           "TIM");
    setupKnob(delayFeedback, "delayFeedback",     "FDB");
    setupKnob(reverbWet,   "reverbWet",           "WET");
    setupKnob(reverbRoom,  "reverbRoom",          "ROM");
    setupKnob(reverbDamp,  "reverbDamp",          "DMP");
    setupKnob(mfCutoff,    "mainFilterCutoff",    "CUT");
    setupKnob(mfResonance, "mainFilterResonance", "RES");

    mfTypeCombo.addItemList({"LP", "HP", "BP", "Notch"}, 1);
    mfTypeCombo.setTooltip("Main Filter Type");
    addAndMakeVisible(mfTypeCombo);
    mfTypeAttach = std::make_unique<ComboAttach>(apvts, "mainFilterType", mfTypeCombo);

    auto setupHeader = [&](juce::Label& lbl)
    {
        lbl.setFont(MultiverseFlatTheme::headerFont());
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

void QuickFXSubPanel::setupKnob(KnobGroup& g, const juce::String& paramID, const juce::String& shortName)
{
    g.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    g.knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(g.knob);
    g.attach = std::make_unique<SliderAttach>(proc.apvts, paramID, g.knob);
    g.knob.init(proc, paramID);
    g.label.setText(shortName, juce::dontSendNotification);
    g.label.setFont(MultiverseFlatTheme::labelFont());
    g.label.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary);
    g.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(g.label);
}

void QuickFXSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);
}

void QuickFXSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);

    // 5 sections horizontally
    const int numSections = 5;
    const int sectionW = area.getWidth() / numSections;

    // Helper: layout a section with enable toggle, header, and knob rows
    auto layoutSection = [&](juce::Label& header, juce::ToggleButton* enable,
                             std::initializer_list<std::pair<KnobGroup*, KnobGroup*>> pairs,
                             juce::Rectangle<int> sec)
    {
        auto topRow = sec.removeFromTop(16);
        header.setBounds(topRow.removeFromLeft(sectionW - 44));
        if (enable)
            enable->setBounds(topRow.removeFromRight(36).withSizeKeepingCentre(28, TOGGLE_H));

        // Knob rows
        for (auto& [left, right] : pairs)
        {
            auto row = sec.removeFromTop(KNOB_SZ + LABEL_H + 2);
            if (left && right)
            {
                auto lCol = row.removeFromLeft(row.getWidth() / 2);
                auto rCol = row;
                left->knob.setBounds(lCol.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
                lCol.removeFromTop(1);
                left->label.setBounds(lCol.removeFromTop(LABEL_H));
                right->knob.setBounds(rCol.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
                rCol.removeFromTop(1);
                right->label.setBounds(rCol.removeFromTop(LABEL_H));
            }
            else if (left)
            {
                left->knob.setBounds(row.withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
                auto labelRow = sec.removeFromTop(LABEL_H);
                left->label.setBounds(labelRow.withSizeKeepingCentre(40, LABEL_H));
            }
        }
    };

    // FLT MOD section
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(filterModLabel, &filterModEnable,
            { {&fmCutoff, &fmResonance}, {&fmEnvDepth, nullptr} }, sec);
    }

    // AMP MOD section
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(ampModLabel, &ampModEnable,
            { {&amVolume, &amPan} }, sec);
    }

    // DELAY section
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(delayLabel, nullptr,
            { {&delayMix, &delayTime}, {&delayFeedback, nullptr} }, sec);
    }

    // REVERB section
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(reverbLabel, nullptr,
            { {&reverbWet, &reverbRoom}, {&reverbDamp, nullptr} }, sec);
    }

    // MAIN FILTER section
    {
        auto sec = area;
        layoutSection(mainFilterLabel, &mainFilterEnable,
            { {&mfCutoff, &mfResonance} }, sec);
        mfTypeCombo.setBounds(sec.removeFromTop(COMBO_H).reduced(2));
    }
}

//==============================================================================
// KeyboardSubPanel

KeyboardSubPanel::KeyboardSubPanel(PluginProcessor& p)
    : proc(p),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    pitchWheel.onValueChange = [this](float v) { sendPitchBend(v); };
    addAndMakeVisible(pitchWheel);

    modWheel.onValueChange = [this](float v) { sendModWheel(v); };
    addAndMakeVisible(modWheel);

    keyboard.setOctaveForMiddleC(5);
    keyboard.setAvailableRange(24, 108);
    keyboard.setLowestVisibleKey(36);
    addAndMakeVisible(keyboard);
}

void KeyboardSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);
}

void KeyboardSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);

    // Keyboard at the bottom
    auto kbArea = area.removeFromBottom(KEYBOARD_H);
    keyboard.setBounds(kbArea);

    // Wheels on the left side
    auto wheelArea = area.removeFromLeft(WHEEL_W * 2 + 4);
    pitchWheel.setBounds(wheelArea.removeFromLeft(WHEEL_W).reduced(2, 6));
    modWheel.setBounds(wheelArea.removeFromLeft(WHEEL_W).reduced(2, 6));
}

void KeyboardSubPanel::sendPitchBend(float value)
{
    const int midiValue = static_cast<int>(8192 + value * 8191);
    juce::MidiMessage msg = juce::MidiMessage::pitchWheel(1, midiValue);
    juce::ScopedLock sl(proc.uiMidiLock);
    proc.uiMidiBuffer.addEvent(msg, 0);
}

void KeyboardSubPanel::sendModWheel(float value)
{
    const int midiValue = static_cast<int>(value * 127);
    juce::MidiMessage msg = juce::MidiMessage::controllerEvent(1, 1, midiValue);
    juce::ScopedLock sl(proc.uiMidiLock);
    proc.uiMidiBuffer.addEvent(msg, 0);
}

//==============================================================================
// ModBar

ModBar::ModBar(PluginProcessor& p)
    : proc(p),
      env1Panel(p, 0),
      env2Panel(p, 1),
      env3Panel(p, 2),
      lfoPanel(p),
      macroPanel(p),
      quickFXPanel(p),
      keyboardPanel(p)
{
    // Initialize sub-panels array
    subPanels[kEnv1]  = &env1Panel;
    subPanels[kEnv2]  = &env2Panel;
    subPanels[kEnv3]  = &env3Panel;
    subPanels[kLfo]    = &lfoPanel;
    subPanels[kMacro]  = &macroPanel;
    subPanels[kQfx]    = &quickFXPanel;
    subPanels[kKey]    = &keyboardPanel;

    // Sub-tab buttons
    const juce::String tabNames[] = {"ENV1", "ENV2", "ENV3", "LFO", "MACRO", "QFX", "KEY"};
    for (int i = 0; i < kNumSubTabs; ++i)
    {
        subTabButtons[i].setButtonText(tabNames[i]);
        subTabButtons[i].setClickingTogglesState(true);
        subTabButtons[i].setRadioGroupId(2001);
        subTabButtons[i].addListener(this);
        addAndMakeVisible(subTabButtons[i]);
    }
    subTabButtons[kMacro].setToggleState(true, juce::dontSendNotification);

    // Sub-panels (all added as children, only active one visible)
    for (int i = 0; i < kNumSubTabs; ++i)
        addAndMakeVisible(*subPanels[i]);

    // Show only the active sub-panel
    switchSubTab(kMacro);

    startTimerHz(15);
}

ModBar::~ModBar()
{
    stopTimer();
}

void ModBar::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep);

    // Top border
    g.setColour(MultiverseFlatTheme::borderLight);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    // Sub-tab strip background
    g.setColour(MultiverseFlatTheme::bgBase);
    g.fillRect(0, 0, getWidth(), SUB_TAB_H);
}

void ModBar::resized()
{
    auto area = getLocalBounds();

    // Sub-tab strip
    auto tabRow = area.removeFromTop(SUB_TAB_H);
    const int tabW = tabRow.getWidth() / kNumSubTabs;
    for (int i = 0; i < kNumSubTabs; ++i)
        subTabButtons[i].setBounds(tabRow.removeFromLeft(tabW));

    // Content area
    for (int i = 0; i < kNumSubTabs; ++i)
        subPanels[i]->setBounds(area);
}

void ModBar::switchSubTab(int newIndex)
{
    activeSubTab = newIndex;
    for (int i = 0; i < kNumSubTabs; ++i)
    {
        subPanels[i]->setVisible(i == newIndex);
        const bool active = (i == newIndex);
        subTabButtons[i].setColour(juce::TextButton::buttonColourId,
            active ? MultiverseFlatTheme::accentCyan.withAlpha(0.2f) : MultiverseFlatTheme::bgBase);
        subTabButtons[i].setColour(juce::TextButton::textColourOffId,
            active ? MultiverseFlatTheme::accentCyan : MultiverseFlatTheme::textSecondary);
        subTabButtons[i].setColour(juce::TextButton::textColourOnId,
            MultiverseFlatTheme::accentCyan);
    }
    repaint();
    resized();
}

void ModBar::timerCallback()
{
    // Nothing extra needed — sub-panels have their own timers
}

void ModBar::buttonClicked(juce::Button* btn)
{
    for (int i = 0; i < kNumSubTabs; ++i)
    {
        if (btn == &subTabButtons[i])
        {
            switchSubTab(i);
            return;
        }
    }
}