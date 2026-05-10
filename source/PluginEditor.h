#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MultiverseFlatTheme.h"
#include "SkinManager.h"
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
#include "Global/GlobalPanel.h"
#include "Presets/PresetOverlay.h"
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
    // Two-tier tab bar
    class TwoTierTabBar : public juce::Component
    {
    public:
        TwoTierTabBar (PluginEditor& editor) : owner (editor) {}

        struct TabDef
        {
            juce::String name;
            int panelIndex;
            bool primary;
        };

        void setTabs (const juce::Array<TabDef>& defs)
        {
            tabs = defs;
            primaryTabs.clear();
            secondaryTabs.clear();
            for (int i = 0; i < tabs.size(); ++i)
            {
                if (tabs[i].primary)
                    primaryTabs.add (i);
                else
                    secondaryTabs.add (i);
            }
            repaint();
        }

        void setActivePanel (int panelIndex)
        {
            activePanel = panelIndex;
            repaint();
        }

        int getActivePanel() const { return activePanel; }

        void paint (juce::Graphics& g) override
        {
            const Skin& s = MultiverseFlatTheme::skin();
            auto area = getLocalBounds();

            // Primary row background
            auto primaryArea = area.removeFromTop (MultiverseFlatTheme::Metrics::primaryTabH);
            g.setColour (s.tabPrimaryBg);
            g.fillRect (primaryArea);

            // Draw primary tabs
            int numPrimary = primaryTabs.size();
            if (numPrimary > 0)
            {
                int tabW = primaryArea.getWidth() / numPrimary;
                for (int i = 0; i < numPrimary; ++i)
                {
                    int tabIdx = primaryTabs[i];
                    bool isActive = (tabs[tabIdx].panelIndex == activePanel);
                    bool isHover = (hoveredTab == tabIdx);
                    auto tabBounds = juce::Rectangle<float> (primaryArea.getX() + i * tabW,
                                                              primaryArea.getY(),
                                                              tabW,
                                                              primaryArea.getHeight());
                    MultiverseFlatTheme::drawPrimaryTabButton (g, tabBounds,
                        tabs[tabIdx].name, isActive, isHover);
                }
            }

            // Secondary row background
            auto secondaryArea = area.removeFromTop (MultiverseFlatTheme::Metrics::secondaryTabH);
            g.setColour (s.tabSecondaryBg);
            g.fillRect (secondaryArea);

            // Bottom border
            g.setColour (s.borderLight);
            g.drawHorizontalLine (getLocalBounds().getBottom() - 1, 0.0f, (float)getWidth());

            // Draw secondary tabs
            int numSecondary = secondaryTabs.size();
            if (numSecondary > 0)
            {
                int tabW = secondaryArea.getWidth() / numSecondary;
                for (int i = 0; i < numSecondary; ++i)
                {
                    int tabIdx = secondaryTabs[i];
                    bool isActive = (tabs[tabIdx].panelIndex == activePanel);
                    bool isHover = (hoveredTab == tabIdx);
                    auto tabBounds = juce::Rectangle<float> (secondaryArea.getX() + i * tabW,
                                                              secondaryArea.getY(),
                                                              tabW,
                                                              secondaryArea.getHeight());
                    MultiverseFlatTheme::drawSecondaryTabButton (g, tabBounds,
                        tabs[tabIdx].name, isActive, isHover);
                }
            }
        }

        void mouseDown (const juce::MouseEvent& e) override
        {
            int idx = getTabAtPosition (e.getPosition());
            if (idx >= 0 && idx < tabs.size())
            {
                owner.switchToPanel (tabs[idx].panelIndex);
            }
        }

        void mouseMove (const juce::MouseEvent& e) override
        {
            int idx = getTabAtPosition (e.getPosition());
            if (idx != hoveredTab)
            {
                hoveredTab = idx;
                repaint();
            }
        }

        void mouseExit (const juce::MouseEvent&) override
        {
            hoveredTab = -1;
            repaint();
        }

    private:
        PluginEditor& owner;
        juce::Array<TabDef> tabs;
        juce::Array<int> primaryTabs;
        juce::Array<int> secondaryTabs;
        int activePanel = 0;
        int hoveredTab = -1;

        int getTabAtPosition (juce::Point<int> pos)
        {
            auto area = getLocalBounds();
            auto primaryArea = area.removeFromTop (MultiverseFlatTheme::Metrics::primaryTabH);
            auto secondaryArea = area.removeFromTop (MultiverseFlatTheme::Metrics::secondaryTabH);

            if (primaryArea.contains (pos))
            {
                int numPrimary = primaryTabs.size();
                if (numPrimary > 0)
                {
                    int col = pos.x / (primaryArea.getWidth() / numPrimary);
                    if (col >= 0 && col < numPrimary)
                        return primaryTabs[col];
                }
            }
            else if (secondaryArea.contains (pos))
            {
                int numSecondary = secondaryTabs.size();
                if (numSecondary > 0)
                {
                    int col = pos.x / (secondaryArea.getWidth() / numSecondary);
                    if (col >= 0 && col < numSecondary)
                        return secondaryTabs[col];
                }
            }
            return -1;
        }
    };

    // Allow TwoTierTabBar to call switchToPanel
    void switchToPanel (int panelIndex);

    MultiverseFlatTheme   mvTheme;
    PluginProcessor& processorRef;

    // All panels (indexed by the tab system)
    juce::Component* panels[11];
    SynthPanel            synthPanel;
    EffectsPanel           effectsPanel;
    ModulationMatrixPanel  modulationMatrixPanel;
    GlobalPanel            globalPanel;
    DrumSequencerPanel     drumSequencerPanel;
    GranularPanel          granularPanel;
    SamplerPanel           samplerPanel;
    LayersPanel            layersPanel;
    ProSequencerPanel      proSequencerPanel;
    ArpeggiatorPanel       arpeggiatorPanel;
    RoutingPanel            routingPanel;
    PerformancePanel       performancePanel;

    // Two-tier tab bar
    TwoTierTabBar tabBar;

    // Scrollable content viewport
    juce::Viewport contentViewport;
    int activePanelIndex = 0;

    // Librarian panel
    LibrarianPanel librarianPanel;

    // Preset overlay
    PresetOverlay presetOverlay;

    // Bottom modulation bar (ENV/LFO/MACRO/QFX sub-tabs — no KEY tab)
    ModBar modBar;

    // Always-visible keyboard strip at bottom
    KeyboardStrip keyboardStrip;

    // Preset navigation (header)
    juce::TextButton   prevPresetButton  { "<" };
    juce::TextButton   presetNameButton;
    juce::TextButton   nextPresetButton  { ">" };
    juce::TextButton   favoriteButton    { "\xe2\x98\x85" };  // ★
    juce::TextButton   backButton        { "\xe2\x97\x80" };  // ◀
    juce::TextButton   forwardButton     { "\xe2\x96\xb6" };  // ▶

    void updatePresetNameLabel();
    void updateFavoriteButtonColor();
    void navigatePresetPrev();
    void navigatePresetNext();
    void cycleFavorite();
    void showPresetOverlay();
    void hidePresetOverlay();

    // Tooltips
    juce::TooltipWindow tooltipWindow { this, 700 };
    bool tooltipsEnabled = true;

    // Menu button
    juce::TextButton menuButton { "\xe2\x98\xb0" };
    void showMainMenu();

    // Quick randomize
    juce::TextButton randomizeButton { "RAND" };
    void showRandomizeMenu();
    void randomizeParams(const juce::StringArray& prefixes, bool filterBoring = true);

    // Header controls
    juce::ComboBox scaleCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;
    juce::ComboBox qualCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> qualAttachment;
    juce::TextButton fxModeButton { "FX" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fxModeAttachment;

    // MIDI Learn
    bool midiLearnActive = false;
    juce::Component::SafePointer<juce::ComboBox> midiLearnCallout;
    void showMidiLearnCallout();

    // Mod-drag overlay
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
                : MultiverseFlatTheme::accent1();
            g.setColour(srcCol.withAlpha(0.15f));
            g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 6.0f);
            g.setColour(srcCol.withAlpha(0.8f));
            g.drawLine(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, 2.0f);
            g.fillEllipse(lineEnd.x - 4.0f, lineEnd.y - 4.0f, 8.0f, 8.0f);
        }
        void timerCallback() override
        {
            const bool wasActive = wasDragActive;
            wasDragActive = NeuKnob::isModDragActive();

            if (!wasActive)
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};