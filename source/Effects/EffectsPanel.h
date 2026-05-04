#pragma once
#include <JuceHeader.h>
#include "../NeuKnob.h"

class PluginProcessor;

// Drag-to-reorder strip showing the 6-effect chain order
class EffectChainStrip : public juce::Component
{
public:
    explicit EffectChainStrip(PluginProcessor& p);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp  (const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

private:
    PluginProcessor& proc;
    int dragSource = -1;
    int dragOver   = -1;
    bool dragging  = false;

    juce::Rectangle<int> tileRect(int i) const;
    int slotAt(int x) const;
    static const char* effectName(int id);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectChainStrip)
};

//==============================================================================
class EffectsPanel : public juce::Component
{
public:
    explicit EffectsPanel(PluginProcessor& p);
    ~EffectsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    EffectChainStrip chainStrip;

    // --- Chorus ---
    juce::Label chorusSectionLabel;
    NeuKnob chorusRateSlider, chorusDepthSlider, chorusMixSlider;
    juce::Label chorusRateLabel, chorusDepthLabel, chorusMixLabel;

    // --- Distortion ---
    juce::Label distortionSectionLabel;
    NeuKnob distDriveSlider, distToneSlider, distMixSlider;
    juce::Label distDriveLabel, distToneLabel, distMixLabel;

    // --- EQ ---
    juce::Label eqSectionLabel;
    NeuKnob eqLowSlider, eqMidSlider, eqHighSlider;
    juce::Label eqLowLabel, eqMidLabel, eqHighLabel;

    // --- Compressor ---
    juce::Label compressorSectionLabel;
    NeuKnob compThreshSlider, compRatioSlider, compAttackSlider, compReleaseSlider, compMakeupSlider;
    juce::Label compThreshLabel, compRatioLabel, compAttackLabel, compReleaseLabel, compMakeupLabel;

    // --- Delay ---
    juce::Label delaySectionLabel;
    NeuKnob delayTimeSlider, delayFeedbackSlider, delayMixSlider;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayMixLabel;

    // --- Reverb ---
    juce::Label reverbSectionLabel;
    NeuKnob reverbRoomSlider, reverbDampSlider, reverbWetSlider;
    juce::Label reverbRoomLabel, reverbDampLabel, reverbWetLabel;

    NeuKnob reverbPreDelaySlider, reverbLFDampSlider, reverbWidthSlider;
    juce::Label reverbPreDelayLabel, reverbLFDampLabel, reverbWidthLabel;
    juce::ToggleButton reverbFreezeButton;
    juce::Label reverbFreezeLabel;

    // --- Aux Sends ---
    juce::Label sendsSectionLabel;
    NeuKnob auxSendDelaySlider, auxSendReverbSlider;
    juce::Label auxSendDelayLabel, auxSendReverbLabel;

    // Section card bounds (filled in resized())
    juce::Rectangle<int> chorusSectionBounds, distortionSectionBounds, eqSectionBounds;
    juce::Rectangle<int> compressorSectionBounds, delaySectionBounds, reverbSectionBounds;
    juce::Rectangle<int> sendsSectionBounds;

    void setupSlider(juce::Slider& s, double min, double max, double value, double skew = 1.0);
    void setupLabel (juce::Label& l, const juce::String& text);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> chorusRateAttach, chorusDepthAttach, chorusMixAttach;
    std::unique_ptr<SliderAttachment> distDriveAttach, distToneAttach, distMixAttach;
    std::unique_ptr<SliderAttachment> eqLowAttach, eqMidAttach, eqHighAttach;
    std::unique_ptr<SliderAttachment> compThreshAttach, compRatioAttach, compAttackAttach, compReleaseAttach, compMakeupAttach;
    std::unique_ptr<SliderAttachment> delayTimeAttach, delayFeedbackAttach, delayMixAttach;
    std::unique_ptr<SliderAttachment> reverbRoomAttach, reverbDampAttach, reverbWetAttach;
    std::unique_ptr<SliderAttachment> reverbPreDelayAttach, reverbLFDampAttach, reverbWidthAttach;
    std::unique_ptr<ButtonAttachment> reverbFreezeAttach;
    std::unique_ptr<SliderAttachment> auxSendDelayAttach, auxSendReverbAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};
