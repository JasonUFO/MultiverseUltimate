#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MultiverseFlatTheme.h"
#include "DrumSequencer/DrumSequencerPanel.h"
#include "Synth/ModulationMatrixPanel.h"
#include "Synth/SynthPanel.h"
#include "Sampler/SamplerPanel.h"
#include "Sequencer/ProSequencerPanel.h"
#include "Sequencer/ArpeggiatorPanel.h"
#include "Effects/EffectsPanel.h"
#include "Presets/LibrarianPanel.h"
#include "Granular/GranularPanel.h"
#include "Layers/LayersPanel.h"
#include "Performance/PerformancePanel.h"
#include "UI/BottomBar.h"
#include "Routing/RoutingPanel.h"
#include "NeuKnob.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::DragAndDropContainer,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (juce::Button*) override;
    void comboBoxChanged (juce::ComboBox*) override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    MultiverseFlatTheme   mvTheme;
    PluginProcessor& processorRef;
    DrumSequencerPanel    drumSequencerPanel;
    ModulationMatrixPanel modulationMatrixPanel;
    SamplerPanel          samplerPanel;
    ProSequencerPanel     proSequencerPanel;
    ArpeggiatorPanel     arpeggiatorPanel;
    SynthPanel            synthPanel;
    EffectsPanel          effectsPanel;
    GranularPanel         granularPanel;
    LayersPanel           layersPanel;
    PerformancePanel      performancePanel;
    RoutingPanel          routingPanel;
    juce::TabbedComponent tabs;

    // Librarian panel (PRE tab)
    LibrarianPanel librarianPanel;

    // Bottom modulation bar (ENV/LFO/MACRO/QFX/KEY sub-tabs)
    ModBar                modBar;

    // Preset navigation (header)
    juce::TextButton   prevPresetButton  { "<" };
    juce::Label        presetNameLabel;
    juce::TextButton   nextPresetButton  { ">" };
    juce::TextButton   favoriteButton    { "\xe2\x98\x85" };  // ★
    juce::TextButton   backButton        { "\xe2\x97\x80" };  // ◀
    juce::TextButton   forwardButton     { "\xe2\x96\xb6" };  // ▶

    void updatePresetNameLabel();
    void updateFavoriteButtonColor();
    void navigatePresetPrev();
    void navigatePresetNext();
    void cycleFavorite();

    // Tooltips
    juce::TooltipWindow tooltipWindow { this, 700 };
    bool tooltipsEnabled = true;

    // Menu button (☰)
    juce::TextButton menuButton { "\xe2\x98\xb0" };
    void showMainMenu();

    // Quick randomize
    juce::TextButton randomizeButton { "RAND" };
    void showRandomizeMenu();
    void randomizeParams(const juce::StringArray& prefixes, bool filterBoring = true);

    // Hidden controls (kept for APVTS attachments + menu access)
    juce::ComboBox scaleCombo;
    juce::ComboBox qualCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualAttachment;
    juce::TextButton fxModeButton { "FX" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fxModeAttachment;

    // MIDI Learn (via menu + CallOutBox)
    bool midiLearnActive = false;
    juce::Component::SafePointer<juce::ComboBox> midiLearnCallout;
    void showMidiLearnCallout();

    // Mod-drag overlay — draws connection line during modulation drag
    class ModDragOverlay : public juce::Component, private juce::Timer
    {
    public:
        ModDragOverlay() { startTimerHz(30); }
        ~ModDragOverlay() override { stopTimer(); }
        void paint(juce::Graphics& g) override
        {
            if (lineStart.isOrigin() && lineEnd.isOrigin()) return;
            const juce::Colour srcCol = NeuKnob::isModDragActive()
                ? NeuKnob::getModSourceColour(NeuKnob::getModDragSource())
                : MultiverseFlatTheme::accentCyan;
            // Glow line
            g.setColour(srcCol.withAlpha(0.15f));
            g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 6.0f);
            // Core line
            g.setColour(srcCol.withAlpha(0.8f));
            g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 2.0f);
            // End dot
            g.fillEllipse(lineEnd.x - 4.0f, lineEnd.y - 4.0f, 8.0f, 8.0f);
        }
        void timerCallback() override
        {
            const bool wasActive = wasDragActive;
            wasDragActive = NeuKnob::isModDragActive();

            if (!wasDragActive)
            {
                if (!NeuKnob::isModDragActive())
                {
                    if (!(lineStart.isOrigin() && lineEnd.isOrigin()))
                    {
                        lineStart = lineEnd = juce::Point<float>();
                        repaint();
                    }
                    return;
                }
                // Drag just started — capture start position from mouse
                auto mouseSrc = juce::Desktop::getInstance().getMainMouseSource();
                auto screenPos = mouseSrc.getScreenPosition();
                lineStart = getLocalPoint(nullptr, screenPos).toFloat();
            }

            if (NeuKnob::isModDragActive())
            {
                auto mouseSrc = juce::Desktop::getInstance().getMainMouseSource();
                auto screenPos = mouseSrc.getScreenPosition();
                lineEnd = getLocalPoint(nullptr, screenPos).toFloat();
                repaint();
            }
            else
            {
                lineStart = lineEnd = juce::Point<float>();
                repaint();
            }
        }
        juce::Point<float> lineStart, lineEnd;
        bool wasDragActive = false;
    };
    ModDragOverlay modDragOverlay;

    void setupTabs();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};