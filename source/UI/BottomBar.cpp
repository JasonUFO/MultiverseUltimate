#include "BottomBar.h"
#include "../Assets/AssetManager.h"
#include "../PluginProcessor.h"
#include "../Synth/LFOShapeEditor.h"

//==============================================================================
// PitchWheel — wheel sprite with skin-aware accent overlay

void PitchWheel::paint(juce::Graphics& g)
{
    const Skin& s = MultiverseFlatTheme::skin();
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Track background with gradient
    juce::ColourGradient trackGrad(s.bgDeep.contrasting(0.02f), bounds.getTopLeft(),
                                    s.bgDeep, bounds.getBottomLeft(), false);
    g.setGradientFill(trackGrad);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Track border
    g.setColour(s.borderLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Center line
    const float cy = bounds.getCentreY();
    g.setColour(s.textMuted.withAlpha(0.25f));
    g.drawHorizontalLine(static_cast<int>(cy), bounds.getX() + 4.0f, bounds.getRight() - 4.0f);

    // Tick marks
    g.setColour(s.textMuted.withAlpha(0.15f));
    for (int i = 1; i < 8; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * i / 8.0f;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
    }

    // Wheel sprite thumb indicator
    const float wheelSize = juce::jmin(bounds.getWidth() - 4.0f, 30.0f);
    const float thumbY = cy - value * (bounds.getHeight() / 2.0f - wheelSize / 2.0f) - wheelSize / 2.0f;
    auto wheelRect = juce::Rectangle<float>(
        bounds.getCentreX() - wheelSize / 2.0f,
        thumbY,
        wheelSize,
        wheelSize
    );

    AssetManager::instance().drawWheelFrame(g, wheelRect, value, true);

    // Accent glow and fill overlay
    if (value != 0.0f)
    {
        // Fill from center to thumb
        auto fillRect = juce::Rectangle<float>(bounds.getX() + 4.0f,
            juce::jmin(cy, thumbY + wheelSize / 2.0f),
            bounds.getWidth() - 8.0f,
            std::abs(thumbY + wheelSize / 2.0f - cy));

        juce::ColourGradient grad(s.accent1.withAlpha(0.0f), 0, fillRect.getY(),
                                   s.accent1.withAlpha(0.4f), 0, fillRect.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(fillRect, 3.0f);

        // Glow around wheel thumb
        g.setColour(s.accent1.withAlpha(0.25f));
        g.fillRoundedRectangle(wheelRect.expanded(2.0f), 5.0f);
    }
    else
    {
        // Neutral glow at center
        g.setColour(s.bgRaised.withAlpha(0.3f));
        g.fillRoundedRectangle(wheelRect.expanded(1.0f), 4.0f);
    }
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

//==============================================================================
// ModWheel — wheel sprite with skin-aware accent overlay, stays where released

void ModWheel::paint(juce::Graphics& g)
{
    const Skin& s = MultiverseFlatTheme::skin();
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Track background with gradient (matches PitchWheel)
    juce::ColourGradient trackGrad(s.bgDeep.contrasting(0.02f), bounds.getTopLeft(),
                                    s.bgDeep, bounds.getBottomLeft(), false);
    g.setGradientFill(trackGrad);
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(s.borderLight.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    // Tick marks
    g.setColour(s.textMuted.withAlpha(0.15f));
    for (int i = 1; i < 8; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * i / 8.0f;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX() + 6.0f, bounds.getRight() - 6.0f);
    }

    // Wheel sprite thumb indicator
    const float wheelSize = juce::jmin(bounds.getWidth() - 4.0f, 30.0f);
    const float trackH = bounds.getHeight() - 16.0f;
    const float thumbY = bounds.getBottom() - 8.0f - value * trackH - wheelSize / 2.0f;
    auto wheelRect = juce::Rectangle<float>(
        bounds.getCentreX() - wheelSize / 2.0f,
        thumbY,
        wheelSize,
        wheelSize
    );

    AssetManager::instance().drawWheelFrame(g, wheelRect, value, false);

    // Fill from bottom and glow overlay
    if (value > 0.001f)
    {
        const float fillH = value * trackH;
        const float fillY = bounds.getBottom() - 8.0f - fillH;
        auto fill = juce::Rectangle<float>(bounds.getX() + 4.0f, fillY, bounds.getWidth() - 8.0f, fillH);

        juce::ColourGradient grad(s.accent1.withAlpha(0.05f), 0, fill.getY(),
                                   s.accent1.withAlpha(0.4f), 0, fill.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(fill, 3.0f);

        // Glow around wheel thumb
        g.setColour(s.accent1.withAlpha(0.25f));
        g.fillRoundedRectangle(wheelRect.expanded(2.0f), 5.0f);
    }
    else
    {
        // Neutral glow at bottom
        g.setColour(s.bgRaised.withAlpha(0.3f));
        g.fillRoundedRectangle(wheelRect.expanded(1.0f), 4.0f);
    }
}

void ModWheel::mouseDown(const juce::MouseEvent& e)
{
    const float boundsH = static_cast<float>(getHeight() - 16);
    value = juce::jlimit(0.0f, 1.0f, (boundsH / 2.0f + 8.0f - static_cast<float>(e.y)) / boundsH);
    repaint();
    if (onValueChange) onValueChange(value);
}
void ModWheel::mouseDrag(const juce::MouseEvent& e)
{
    const float boundsH = static_cast<float>(getHeight() - 16);
    value = juce::jlimit(0.0f, 1.0f, (boundsH / 2.0f + 8.0f - static_cast<float>(e.y)) / boundsH);
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
        g.label.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
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
    g.fillAll(MultiverseFlatTheme::bgDeep());
    const juce::String labels[] = { "AMP ENV", "ENV 2", "ENV 3" };
    g.setColour(MultiverseFlatTheme::accent1());
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText(labels[envIndex], getLocalBounds().removeFromTop(18).toFloat(),
              juce::Justification::centred);
}

void EnvSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    area.removeFromTop(18);

    auto displayArea = area.removeFromLeft(area.getWidth() * 2 / 5);
    envelopeDisplay.setBounds(displayArea.reduced(2));

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

    for (int i = 0; i < 8; ++i)
    {
        bankButtons[i].onClick = [this, i] { switchBank(i); };
        addAndMakeVisible(bankButtons[i]);
    }
    bankButtons[0].isActive = true;

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
    rateLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    rateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(rateLabel);

    shapeLabel.setFont(MultiverseFlatTheme::labelFont());
    shapeLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    shapeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(shapeLabel);

    drawButton.onClick = [this] { showShapeEditor(); };

    switchBank(0);
    startTimerHz(15);
}

void LFOSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep());

    g.setColour(MultiverseFlatTheme::accent1());
    g.setFont(MultiverseFlatTheme::headerFont());
    auto topArea = getLocalBounds().removeFromTop(18).toFloat();
    g.drawText("LFO " + juce::String(activeBank + 1), topArea, juce::Justification::centredLeft);

    if (wavePathDirty)
    {
        rebuildWavePath();
        wavePathDirty = false;
    }
    auto previewArea = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);
    previewArea.removeFromTop(22 + 26 + 4);
    previewArea = previewArea.removeFromLeft(previewArea.getWidth() * 2 / 5);

    g.setColour(MultiverseFlatTheme::bgBase());
    g.fillRoundedRectangle(previewArea.toFloat().reduced(2), 4.0f);
    g.setColour(MultiverseFlatTheme::borderLight().withAlpha(0.4f));
    g.drawRoundedRectangle(previewArea.toFloat().reduced(2), 4.0f, 0.8f);

    g.setColour(MultiverseFlatTheme::accent1());
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));
}

void LFOSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);

    dragArea = area.removeFromTop(18);

    auto bankRow = area.removeFromTop(26);
    const int btnW = bankRow.getWidth() / 8;
    for (int i = 0; i < 8; ++i)
        bankButtons[i].setBounds(bankRow.removeFromLeft(btnW).reduced(1, 2));

    area.removeFromTop(4);

    auto leftArea = area.removeFromLeft(area.getWidth() * 2 / 5);
    auto rightArea = area;

    auto controlArea = rightArea;

    auto rateRow = controlArea.removeFromTop(KNOB_SZ + 16);
    rateKnob.setBounds(rateRow.removeFromTop(KNOB_SZ).withSizeKeepingCentre(KNOB_SZ, KNOB_SZ));
    rateLabel.setBounds(rateRow.removeFromTop(14));

    controlArea.removeFromTop(4);

    auto shapeRow = controlArea.removeFromTop(26);
    shapeLabel.setBounds(shapeRow.removeFromLeft(40));
    shapeCombo.setBounds(shapeRow);

    controlArea.removeFromTop(4);

    auto syncRow = controlArea.removeFromTop(26);
    syncButton.setBounds(syncRow.removeFromLeft(50));
    syncDivCombo.setBounds(syncRow);

    controlArea.removeFromTop(4);

    drawButton.setBounds(controlArea.removeFromTop(24));
}

void LFOSubPanel::mouseDrag(const juce::MouseEvent& e)
{
    if (!dragArea.contains(e.getMouseDownPosition()))
        return;

    if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
    {
        static const int lfoSourceMap[] = { 0, 1, 2, 3, 15, 16, 17, 18 };
        int sourceInt = (activeBank >= 0 && activeBank < 8) ? lfoSourceMap[activeBank] : 0;
        auto dragDescription = juce::String("modsrc:") + juce::String(sourceInt);
        container->startDragging(dragDescription, this);
    }
}

void LFOSubPanel::switchBank(int newBank)
{
    activeBank = newBank;
    auto& apvts = proc.apvts;
    const juce::String idx = juce::String(newBank + 1);

    rateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "lfo" + idx + "Rate", rateKnob);
    rateKnob.init(proc, "lfo" + idx + "Rate");

    shapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "lfo" + idx + "Shape", shapeCombo);

    syncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "lfo" + idx + "Sync", syncButton);

    syncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "lfo" + idx + "SyncDiv", syncDivCombo);

    for (int i = 0; i < 8; ++i)
    {
        bankButtons[i].isActive = (i == newBank);
        bankButtons[i].repaint();
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

    auto* shapeParam = proc.apvts.getRawParameterValue("lfo" + juce::String(activeBank + 1) + "Shape");
    int shape = shapeParam ? static_cast<int>(shapeParam->load()) : 0;

    wavePath.startNewSubPath(inner.getX(), cy);

    for (int i = 0; i <= 100; ++i)
    {
        const float phase = static_cast<float>(i) / 100.0f * 2.0f * juce::MathConstants<float>::pi;
        float val = 0.0f;
        switch (shape)
        {
            case 0: val = std::sin(phase); break;
            case 1: val = 1.0f - 2.0f * std::abs(phase / juce::MathConstants<float>::pi - 1.0f); break;
            case 2: val = phase / juce::MathConstants<float>::pi - 1.0f; break;
            case 3: val = phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f; break;
            case 4: val = std::sin(phase + static_cast<float>(activeBank) * 0.7f); break;
            case 5: val = std::sin(phase); break;
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
    auto* shapeEditor = new LFOShapeEditor();
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
        g.nameLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
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
        g.valueLabel.setColour(juce::Label::textColourId, MultiverseFlatTheme::textMuted());
        g.valueLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        g.valueLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(g.valueLabel);
    }

    startTimerHz(30);
}

void MacroSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep());
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
        btn.setColour(juce::ToggleButton::tickColourId, MultiverseFlatTheme::accent1());
        btn.setColour(juce::ToggleButton::textColourId, MultiverseFlatTheme::textSecondary());
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
        lbl.setColour(juce::Label::textColourId, MultiverseFlatTheme::accent1());
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
    g.label.setColour(juce::Label::textColourId, MultiverseFlatTheme::textSecondary());
    g.label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(g.label);
}

void QuickFXSubPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgDeep());
}

void QuickFXSubPanel::resized()
{
    auto area = getLocalBounds().reduced(MultiverseFlatTheme::Metrics::outerMargin, 2);

    const int numSections = 5;
    const int sectionW = area.getWidth() / numSections;

    auto layoutSection = [&](juce::Label& header, juce::ToggleButton* enable,
                             std::initializer_list<std::pair<KnobGroup*, KnobGroup*>> pairs,
                             juce::Rectangle<int> sec)
    {
        auto topRow = sec.removeFromTop(16);
        header.setBounds(topRow.removeFromLeft(sectionW - 44));
        if (enable)
            enable->setBounds(topRow.removeFromRight(36).withSizeKeepingCentre(28, TOGGLE_H));

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

    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(filterModLabel, &filterModEnable,
            { {&fmCutoff, &fmResonance}, {&fmEnvDepth, nullptr} }, sec);
    }
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(ampModLabel, &ampModEnable,
            { {&amVolume, &amPan} }, sec);
    }
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(delayLabel, nullptr,
            { {&delayMix, &delayTime}, {&delayFeedback, nullptr} }, sec);
    }
    {
        auto sec = area.removeFromLeft(sectionW);
        layoutSection(reverbLabel, nullptr,
            { {&reverbWet, &reverbRoom}, {&reverbDamp, nullptr} }, sec);
    }
    {
        auto sec = area;
        layoutSection(mainFilterLabel, &mainFilterEnable,
            { {&mfCutoff, &mfResonance} }, sec);
        mfTypeCombo.setBounds(sec.removeFromTop(COMBO_H).reduced(2));
    }
}

//==============================================================================
// KeyboardStrip — always-visible bottom strip with wheels + keyboard

KeyboardStrip::KeyboardStrip(PluginProcessor& p)
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
    keyboard.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, MultiverseFlatTheme::keyboardWhite());
    keyboard.setColour(juce::MidiKeyboardComponent::blackNoteColourId, MultiverseFlatTheme::keyboardBlack());
    addAndMakeVisible(keyboard);
}

void KeyboardStrip::paint(juce::Graphics& g)
{
    const Skin& s = MultiverseFlatTheme::skin();
    g.fillAll(s.bgDeep);

    // Top border with accent glow
    g.setColour(s.borderLight);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    // Glow line along top
    g.setColour(s.accent1.withAlpha(0.15f));
    g.drawHorizontalLine(1, 0.0f, static_cast<float>(getWidth()));
}

void KeyboardStrip::resized()
{
    const Skin& s = MultiverseFlatTheme::skin();
    const int wheelW = static_cast<int>(s.wheelWidth);
    const int kbH = static_cast<int>(s.keyboardHeight * 0.65f); // keyboard portion of strip
    const int wheelGap = 4;
    const int margin = 4;

    auto area = getLocalBounds().reduced(margin, 2);

    // Wheels on the left
    auto wheelArea = area.removeFromLeft(wheelW * 2 + wheelGap);
    pitchWheel.setBounds(wheelArea.removeFromLeft(wheelW).reduced(2, 6));
    wheelArea.removeFromLeft(wheelGap);
    modWheel.setBounds(wheelArea.removeFromLeft(wheelW).reduced(2, 6));

    // Keyboard fills the rest
    auto kbArea = area.removeFromBottom(kbH);
    keyboard.setBounds(kbArea);
}

void KeyboardStrip::sendPitchBend(float value)
{
    const int midiValue = static_cast<int>(8192 + value * 8191);
    juce::MidiMessage msg = juce::MidiMessage::pitchWheel(1, midiValue);
    juce::ScopedLock sl(proc.uiMidiLock);
    proc.uiMidiBuffer.addEvent(msg, 0);
}

void KeyboardStrip::sendModWheel(float value)
{
    const int midiValue = static_cast<int>(value * 127);
    juce::MidiMessage msg = juce::MidiMessage::controllerEvent(1, 1, midiValue);
    juce::ScopedLock sl(proc.uiMidiLock);
    proc.uiMidiBuffer.addEvent(msg, 0);
}

//==============================================================================
// ModBar — sub-tabs without KEY (keyboard is now always visible)

ModBar::ModBar(PluginProcessor& p)
    : proc(p),
      env1Panel(p, 0),
      env2Panel(p, 1),
      env3Panel(p, 2),
      lfoPanel(p),
      macroPanel(p),
      quickFXPanel(p)
{
    subPanels[kEnv1]  = &env1Panel;
    subPanels[kEnv2]  = &env2Panel;
    subPanels[kEnv3]  = &env3Panel;
    subPanels[kLfo]    = &lfoPanel;
    subPanels[kMacro]  = &macroPanel;
    subPanels[kQfx]    = &quickFXPanel;

    for (int i = 0; i < kNumSubTabs; ++i)
    {
        subTabButtons[i].onClick = [this, i] { switchSubTab(i); };
        addAndMakeVisible(subTabButtons[i]);
    }
    subTabButtons[kMacro].isActive = true;

    for (int i = 0; i < kNumSubTabs; ++i)
        addAndMakeVisible(*subPanels[i]);

    switchSubTab(kMacro);
    startTimerHz(15);
}

ModBar::~ModBar()
{
    stopTimer();
}

void ModBar::paint(juce::Graphics& g)
{
    const Skin& s = MultiverseFlatTheme::skin();
    auto bounds = getLocalBounds().toFloat();

    // 3D procedural background: inset panel with gradient
    MultiverseFlatTheme::drawInset(g, bounds, 4.0f);
    MultiverseFlatTheme::drawGradientFill(g, bounds.reduced(1.0f));

    // Top border line
    g.setColour(s.borderLight);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));

    // Sub-tab row background
    g.setColour(s.bgBase);
    g.fillRect(0, 0, getWidth(), SUB_TAB_H);
}

void ModBar::resized()
{
    auto area = getLocalBounds();

    auto tabRow = area.removeFromTop(SUB_TAB_H);
    const int tabW = tabRow.getWidth() / kNumSubTabs;
    for (int i = 0; i < kNumSubTabs; ++i)
        subTabButtons[i].setBounds(tabRow.removeFromLeft(tabW));

    for (int i = 0; i < kNumSubTabs; ++i)
        subPanels[i]->setBounds(area);
}

void ModBar::switchSubTab(int newIndex)
{
    activeSubTab = newIndex;
    for (int i = 0; i < kNumSubTabs; ++i)
    {
        subPanels[i]->setVisible(i == newIndex);
        subTabButtons[i].isActive = (i == newIndex);
        subTabButtons[i].repaint();
    }
    repaint();
    resized();
}

void ModBar::timerCallback()
{
    // Nothing extra needed
}