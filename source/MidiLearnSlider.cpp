#include "MidiLearnSlider.h"
#include "PluginProcessor.h"

MidiLearnSlider::MidiLearnSlider()
{
}

MidiLearnSlider::~MidiLearnSlider()
{
    stopTimer();
}

void MidiLearnSlider::init (PluginProcessor& p, const juce::String& pid)
{
    proc    = &p;
    paramID = pid;
    startTimerHz (10); // poll at 10 Hz — cheap, just a vector scan
}

//==============================================================================
void MidiLearnSlider::paint (juce::Graphics& g)
{
    juce::Slider::paint (g); // draw the knob normally first

    const auto b  = getLocalBounds().reduced (3).toFloat();
    const float sz = 14.0f;

    if (mapped)
    {
        // Orange "L" badge, top-right corner
        auto badge = juce::Rectangle<float> (b.getRight() - sz, b.getY(), sz, sz);
        g.setColour (juce::Colours::orange.withAlpha (0.9f));
        g.fillRoundedRectangle (badge, 3.0f);
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.drawText ("L", badge.toNearestInt(), juce::Justification::centred, false);
    }

    if (macroAssigned)
    {
        // Cyan "M" badge, top-left corner
        auto badge = juce::Rectangle<float> (b.getX(), b.getY(), sz, sz);
        g.setColour (juce::Colours::cyan.withAlpha (0.9f));
        g.fillRoundedRectangle (badge, 3.0f);
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.drawText ("M", badge.toNearestInt(), juce::Justification::centred, false);
    }
}

void MidiLearnSlider::mouseDown (const juce::MouseEvent& e)
{
    if (e.mods.isRightButtonDown() && proc != nullptr)
    {
        showContextMenu();
        return;
    }
    juce::Slider::mouseDown (e);
}

//==============================================================================
void MidiLearnSlider::timerCallback()
{
    const bool nowMapped = checkHasMapping();
    if (nowMapped != mapped)
    {
        mapped = nowMapped;
        repaint();
    }

    const bool nowMacro = checkHasMacro();
    if (nowMacro != macroAssigned)
    {
        macroAssigned = nowMacro;
        repaint();
    }
}

bool MidiLearnSlider::checkHasMacro() const
{
    if (proc == nullptr || paramID.isEmpty())
        return false;
    return proc->getMacroManager().getMacroIndexForParam(paramID) >= 0;
}

bool MidiLearnSlider::checkHasMapping() const
{
    if (proc == nullptr || paramID.isEmpty())
        return false;
    for (const auto& m : proc->midiMappings)
        if (m.paramID == paramID)
            return true;
    return false;
}

void MidiLearnSlider::showContextMenu()
{
    juce::PopupMenu menu;

    if (mapped)
    {
        juce::String mappingDesc;
        for (const auto& m : proc->midiMappings)
        {
            if (m.paramID == paramID)
            {
                switch (m.type)
                {
                    case PluginProcessor::MidiMapping::CC:
                        mappingDesc = "CC " + juce::String (m.controllerNumber);
                        break;
                    case PluginProcessor::MidiMapping::PitchWheel:
                        mappingDesc = "Pitch Wheel";
                        break;
                    case PluginProcessor::MidiMapping::ChannelPressure:
                        mappingDesc = "Channel Pressure";
                        break;
                    default:
                        mappingDesc = "Unknown";
                        break;
                }
                if (m.midiChannel > 0)
                    mappingDesc += " (Ch " + juce::String (m.midiChannel) + ")";
                break;
            }
        }
        menu.addItem (1, "Mapped: " + mappingDesc, false);
        menu.addSeparator();
        menu.addItem (2, "Unlearn");
    }
    else
    {
        menu.addItem (3, "Learn MIDI CC...");
    }

    // Macro assignment submenu
    auto& mgr = proc->getMacroManager();
    const int assignedMacro = mgr.getMacroIndexForParam (paramID);

    juce::PopupMenu macroSub;
    for (int i = 0; i < MacroManager::NUM_MACROS; ++i)
        macroSub.addItem (100 + i, mgr.getName (i), true, assignedMacro == i);

    menu.addSeparator();
    menu.addSubMenu ("Assign to Macro", macroSub);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
        [this, assignedMacro] (int result)
        {
            if (result == 2)
            {
                auto& mappings = proc->midiMappings;
                mappings.erase (
                    std::remove_if (mappings.begin(), mappings.end(),
                        [this](const PluginProcessor::MidiMapping& m) {
                            return m.paramID == paramID;
                        }),
                    mappings.end());
            }
            else if (result == 3)
            {
                const int idx = proc->getParameterIndexFromID (paramID);
                if (idx >= 0)
                    proc->startMidiLearnForParameter (idx);
            }
            else if (result >= 100 && result < 100 + MacroManager::NUM_MACROS)
            {
                const int macroIdx = result - 100;
                if (assignedMacro == macroIdx)
                    proc->getMacroManager().removeAssignment (macroIdx, paramID);
                else
                    proc->getMacroManager().addAssignment (macroIdx, paramID, 0.0f, 1.0f);
            }
        });
}
