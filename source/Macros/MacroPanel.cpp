#include "MacroPanel.h"
#include "../PluginProcessor.h"

MacroPanel::MacroPanel(PluginProcessor& p) : proc(p)
{
    auto& apvts = p.apvts;
    auto& mgr   = p.getMacroManager();

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        auto& g = macros[i];

        g.knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        g.knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
        g.knob.setTooltip("Macro " + juce::String(i + 1)
            + " — right-click any parameter to assign it here");
        addAndMakeVisible(g.knob);

        g.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, "macro" + juce::String(i + 1), g.knob);

        g.nameLabel.setText(mgr.getName(i), juce::dontSendNotification);
        g.nameLabel.setJustificationType(juce::Justification::centred);
        g.nameLabel.setFont(juce::Font(12.0f, juce::Font::bold));
        g.nameLabel.setEditable(false, true);
        g.nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        g.nameLabel.setTooltip("Double-click to rename this macro");
        g.nameLabel.onTextChange = [this, i]
        {
            proc.getMacroManager().setName(i, macros[i].nameLabel.getText());
        };
        addAndMakeVisible(g.nameLabel);

        g.assignButton.setButtonText("0 params");
        g.assignButton.setColour(juce::TextButton::textColourOffId, juce::Colours::orange);
        g.assignButton.setColour(juce::TextButton::buttonColourId,   juce::Colour(0xff2a2a2a));
        g.assignButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff3a3a3a));
        g.assignButton.setTooltip("Click to view or remove parameter assignments");
        g.assignButton.onClick = [this, i] { showAssignmentList(i); };
        addAndMakeVisible(g.assignButton);
    }

    startTimerHz(30);
}

MacroPanel::~MacroPanel()
{
    stopTimer();
}

void MacroPanel::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseTheme::bgBase);

    const float cr = 8.0f;
    if (macroSectionRect.getHeight() > 0)
    {
        MultiverseTheme::drawNeumorphicRect(g, macroSectionRect.toFloat(), cr, 3.0f);
        g.setColour(MultiverseTheme::bgRaised);
        g.fillRoundedRectangle(macroSectionRect.toFloat(), cr);
        g.setColour(MultiverseTheme::shadowLight.withAlpha(0.3f));
        g.drawRoundedRectangle(macroSectionRect.toFloat().reduced(0.5f), cr, 1.0f);

        // Section title
        g.setColour(MultiverseTheme::textLabel);
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("MACRO CONTROLS", macroSectionRect.getX() + 8, macroSectionRect.getY() + 5, 200, 14, juce::Justification::centredLeft);
    }

    // Instruction text
    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("right-click any knob/slider to assign it to a macro",
               macroSectionRect.removeFromTop(28).withTrimmedLeft(8), juce::Justification::centredLeft, false);
}

void MacroPanel::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(30);

    // Macro section card wraps all macro knobs
    macroSectionRect = area;

    const int cols  = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / 2;

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const int col = i % cols;
        const int row = i / cols;
        auto cell = juce::Rectangle<int>(
            area.getX() + col * cellW,
            area.getY() + row * cellH,
            cellW, cellH).reduced(12);

        const int btnH  = 24;
        const int lblH  = 22;
        const int knobH = cell.getHeight() - btnH - lblH;

        auto& g = macros[i];
        g.knob.setBounds(cell.removeFromTop(knobH));
        g.nameLabel.setBounds(cell.removeFromTop(lblH));
        g.assignButton.setBounds(cell.removeFromTop(btnH));
    }
}

void MacroPanel::timerCallback()
{
    auto& mgr   = proc.getMacroManager();
    auto& apvts = proc.apvts;

    // Push macro values to assigned target params (message thread, 30 Hz)
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const auto targets = mgr.getTargets(i);
        if (targets.empty())
            continue;

        const auto* rawMacro = apvts.getRawParameterValue("macro" + juce::String(i + 1));
        if (rawMacro == nullptr)
            continue;
        const float macroVal = rawMacro->load();

        for (const auto& t : targets)
        {
            auto* targetParam = apvts.getParameter(t.paramID);
            if (targetParam == nullptr)
                continue;
            const float norm = juce::jlimit(0.0f, 1.0f,
                t.minVal + macroVal * (t.maxVal - t.minVal));
            targetParam->setValueNotifyingHost(norm);
        }
    }

    // Update assignment count buttons and name labels
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const int count = mgr.getAssignmentCount(i);
        const juce::String countText = (count == 1) ? "1 param"
                                                     : juce::String(count) + " params";
        if (macros[i].assignButton.getButtonText() != countText)
            macros[i].assignButton.setButtonText(countText);

        const juce::String name = mgr.getName(i);
        if (macros[i].nameLabel.getText() != name)
            macros[i].nameLabel.setText(name, juce::dontSendNotification);
    }
}

void MacroPanel::showAssignmentList(int macroIdx)
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
        juce::PopupMenu::Options().withTargetComponent(&macros[macroIdx].assignButton),
        [this, macroIdx, targets](int result)
        {
            if (result >= 1000 && result < 1000 + static_cast<int>(targets.size()))
            {
                const int idx = result - 1000;
                proc.getMacroManager().removeAssignment(macroIdx, targets[idx].paramID);
            }
        });
}
