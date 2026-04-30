#pragma once
#include <JuceHeader.h>

class PluginProcessor;

class PresetBrowserPanel : public juce::Component,
                           public juce::ListBoxModel,
                           public juce::Button::Listener
{
public:
    explicit PresetBrowserPanel(PluginProcessor& p);

    void paint(juce::Graphics&) override;
    void resized() override;

    // ListBoxModel
    int  getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int w, int h, bool selected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    // Button::Listener
    void buttonClicked(juce::Button*) override;

    void refresh();

private:
    PluginProcessor& processorRef;

    juce::Label      nameLabel  { {}, "Name:" };
    juce::TextEditor nameEditor;
    juce::TextButton saveButton  { "Save"   };
    juce::TextButton loadButton  { "Load"   };
    juce::TextButton deleteButton{ "Delete" };
    juce::ListBox    presetList  { "presets", this };

    void saveCurrentPreset();
    void loadSelectedPreset();
    void deleteSelectedPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};
