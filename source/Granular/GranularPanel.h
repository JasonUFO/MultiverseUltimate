#pragma once
#include <JuceHeader.h>
#include "../MidiLearnSlider.h"
#include "GranularEngine.h"

class PluginProcessor;

class GranularPanel : public juce::Component
{
public:
    explicit GranularPanel (PluginProcessor& p);
    ~GranularPanel() override = default;

    void paint   (juce::Graphics& g) override;
    void resized () override;

private:
    PluginProcessor& proc;

    // Source file section
    juce::TextButton  loadButton   { "LOAD GR" };
    juce::Label       fileLabel;

    // Grain controls
    MidiLearnSlider positionSlider;
    MidiLearnSlider grainSizeSlider;
    MidiLearnSlider spraySlider;
    MidiLearnSlider densitySlider;
    MidiLearnSlider pitchScatterSlider;
    MidiLearnSlider stereoSpreadSlider;

    juce::ComboBox  envShapeCombo;
    juce::ToggleButton reverseButton { "Reverse" };

    // Voice envelope
    MidiLearnSlider attackSlider;
    MidiLearnSlider decaySlider;
    MidiLearnSlider sustainSlider;
    MidiLearnSlider releaseSlider;

    // Labels
    juce::Label positionLabel    { {}, "Position"  };
    juce::Label grainSizeLabel   { {}, "Grain Size" };
    juce::Label sprayLabel       { {}, "Spray"      };
    juce::Label densityLabel     { {}, "Density"    };
    juce::Label pitchScatLabel   { {}, "Pitch Scat" };
    juce::Label stereoSpreadLabel{ {}, "Spread"     };
    juce::Label envShapeLabel    { {}, "Env Shape"  };
    juce::Label attackLabel      { {}, "Attack"     };
    juce::Label decayLabel       { {}, "Decay"      };
    juce::Label sustainLabel     { {}, "Sustain"    };
    juce::Label releaseLabel     { {}, "Release"    };
    juce::Label envelopeHeader   { {}, "— Voice Envelope —" };

    // APVTS attachments
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttach  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BoolAttach   = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttach> positionAttach;
    std::unique_ptr<SliderAttach> grainSizeAttach;
    std::unique_ptr<SliderAttach> sprayAttach;
    std::unique_ptr<SliderAttach> densityAttach;
    std::unique_ptr<SliderAttach> pitchScatAttach;
    std::unique_ptr<SliderAttach> stereoSpreadAttach;
    std::unique_ptr<ComboAttach>  envShapeAttach;
    std::unique_ptr<BoolAttach>   reverseAttach;
    std::unique_ptr<SliderAttach> attackAttach;
    std::unique_ptr<SliderAttach> decayAttach;
    std::unique_ptr<SliderAttach> sustainAttach;
    std::unique_ptr<SliderAttach> releaseAttach;

    void setupSlider (MidiLearnSlider& s, const juce::String& paramID,
                      const juce::String& tooltip);
    void setupLabel  (juce::Label& l);
    void loadSourceFile();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularPanel)
};
