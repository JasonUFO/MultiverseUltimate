#pragma once
#include <JuceHeader.h>

class PluginProcessor;

class PresetBrowserPanel : public juce::Component,
                           public juce::ListBoxModel,
                           public juce::Button::Listener,
                           public juce::ComboBox::Listener,
                           public juce::TextEditor::Listener
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

    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor&) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void textEditorEscapeKeyPressed(juce::TextEditor&) override;
    void textEditorFocusLost(juce::TextEditor&) override;

    void refresh();

private:
    PluginProcessor& processorRef;

    // Search
    juce::TextEditor searchEditor;

    // Categories (vertical pill tabs)
    juce::TextButton catAll   { "All"    };
    juce::TextButton catInit  { "Init"   };
    juce::TextButton catBass  { "Bass"   };
    juce::TextButton catLead  { "Lead"   };
    juce::TextButton catPad   { "Pad"    };
    juce::TextButton catDrums{ "Drums"  };
    juce::TextButton catFX    { "FX"     };
    juce::Array<juce::TextButton*> categoryButtons;

    // Bank selector
    juce::ComboBox   bankSelector;

    // Name + Save
    juce::Label      nameLabel  { {}, "Name:" };
    juce::TextEditor nameEditor;
    juce::TextButton saveButton  { "Save"   };

    // Action buttons
    juce::TextButton loadButton  { "Load"   };
    juce::TextButton deleteButton{ "Delete" };
    juce::TextButton importButton{ "Import" };
    juce::TextButton exportButton{ "Export" };

    // Preset list
    juce::ListBox    presetList  { "presets", this };

    // Filtered preset indices
    juce::Array<int> filteredPresetIndices;

    void rebuildFilter();
    void setActiveCategory(juce::TextButton* cat);
    void saveCurrentPreset();
    void loadSelectedPreset();
    void deleteSelectedPreset();
    void importPreset();
    void exportPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};
