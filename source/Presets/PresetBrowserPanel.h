#pragma once
#include <JuceHeader.h>

class PluginProcessor;

class PresetBrowserPanel : public juce::Component,
                           public juce::ListBoxModel,
                           public juce::Button::Listener,
                           public juce::ComboBox::Listener
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

    // ComboBox::Listener
    void comboBoxChanged(juce::ComboBox*) override;

    void refresh();

private:
    PluginProcessor& processorRef;

    juce::ComboBox   bankSelector;
    juce::Label      nameLabel  { {}, "Name:" };
    juce::TextEditor nameEditor;
    juce::TextButton saveButton  { "Save"   };
    juce::TextButton loadButton  { "Load"   };
    juce::TextButton deleteButton{ "Delete" };
    juce::TextButton importButton{ "Import" };
    juce::TextButton exportButton{ "Export" };
    juce::ListBox    presetList  { "presets", this };

    void saveCurrentPreset();
    void loadSelectedPreset();
    void deleteSelectedPreset();
    void importPreset();
    void exportPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};
