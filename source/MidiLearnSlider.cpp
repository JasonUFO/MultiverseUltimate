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

    if (mapped)
    {
        // Orange "L" badge, top-right corner of the knob bounds
        const auto b = getLocalBounds().reduced (3).toFloat();
        const float sz = 14.0f;
        auto badge = juce::Rectangle<float> (b.getRight() - sz, b.getY(), sz, sz);

        g.setColour (juce::Colours::orange.withAlpha (0.9f));
        g.fillRoundedRectangle (badge, 3.0f);
        g.setColour (juce::Colours::black);
        g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.drawText ("L", badge.toNearestInt(), juce::Justification::centred, false);
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
        // Show what's mapped and offer to remove it
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

        menu.addItem (1, "Mapped: " + mappingDesc, false); // non-selectable info
        menu.addSeparator();
        menu.addItem (2, "Unlearn");
    }
    else
    {
        menu.addItem (3, "Learn MIDI CC...");
    }

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
        [this] (int result)
        {
            if (result == 2) // Unlearn
            {
                auto& mappings = proc->midiMappings;
                mappings.erase (
                    std::remove_if (mappings.begin(), mappings.end(),
                        [this](const PluginProcessor::MidiMapping& m) {
                            return m.paramID == paramID;
                        }),
                    mappings.end());
            }
            else if (result == 3) // Learn
            {
                // Find parameter index and start learning
                const int idx = proc->getParameterIndexFromID (paramID);
                if (idx >= 0)
                {
                    proc->startMidiLearnForParameter (idx);
                    // The editor's MIDI learn button state won't auto-update here;
                    // the user can also use the header button for full learn flow.
                }
            }
        });
}
