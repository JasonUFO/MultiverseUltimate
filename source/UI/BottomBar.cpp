#include "BottomBar.h"
#include "../PluginProcessor.h"

BottomBar::BottomBar(PluginProcessor& p)
    : proc(p),
      keyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
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
        g.nameLabel.setFont(juce::Font(9.0f, juce::Font::bold));
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
    }

    keyboard.setOctaveForMiddleC(5);
    keyboard.setAvailableRange(24, 108);
    keyboard.setLowestVisibleKey(36);
    addAndMakeVisible(keyboard);

    startTimerHz(30);
}

BottomBar::~BottomBar()
{
    stopTimer();
}

void BottomBar::paint(juce::Graphics& g)
{
    g.fillAll(MultiverseFlatTheme::bgBase);

    // Separator line at top
    g.setColour(MultiverseFlatTheme::borderLight);
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(getWidth()));
}

void BottomBar::resized()
{
    auto area = getLocalBounds();

    // Keyboard at bottom-right
    auto kbArea = area.removeFromBottom(KEYBOARD_H);
    keyboard.setBounds(kbArea);

    // Macros in the left portion of the remaining space
    const int macroAreaW = MacroManager::NUM_MACROS * 36;
    auto macroArea = area.removeFromLeft(macroAreaW).reduced(4, 2);

    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        auto cell = macroArea.removeFromLeft(36);
        auto& g = macros[i];
        g.knob.setBounds(cell.removeFromTop(KNOB_SIZE).withSizeKeepingCentre(KNOB_SIZE, KNOB_SIZE));
        g.nameLabel.setBounds(cell.removeFromTop(14));
    }
}

void BottomBar::timerCallback()
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

    // Update name labels
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
    {
        const juce::String name = mgr.getName(i);
        if (macros[i].nameLabel.getText() != name)
            macros[i].nameLabel.setText(name, juce::dontSendNotification);
    }
}

void BottomBar::showAssignmentList(int macroIdx)
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