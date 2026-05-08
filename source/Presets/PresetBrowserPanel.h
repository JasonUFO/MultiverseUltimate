#pragma once
#include <JuceHeader.h>

class PluginProcessor;

class PresetBrowserPanel : public juce::Component,
                           public juce::ListBoxModel,
                           public juce::Button::Listener,
                           public juce::ComboBox::Listener,
                           public juce::TextEditor::Listener,
                           private juce::Timer
{
public:
    explicit PresetBrowserPanel(PluginProcessor& p);

    void paint(juce::Graphics&) override;
    void resized() override;

    // ListBoxModel
    int  getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int w, int h, bool selected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;

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
    void focusSearchEditor();

    // Current preset index (for header navigation sync)
    int getSelectedPresetIndex() const;

    // Tag filter management
    void addTagFilter(const juce::String& tag);
    void removeTagFilter(const juce::String& tag);
    void clearTagFilters();
    juce::StringArray getActiveTagFilters() const { return activeTagFilters; }

    // Auto-preview
    bool isAutoPreviewEnabled() const { return autoPreviewEnabled; }

private:
    PluginProcessor& processorRef;

    // Favorite colors
    static const juce::Colour favColors[8];

    // Search
    juce::TextEditor searchEditor;

    // Categories (vertical pill tabs) — includes virtual "Favorites"
    juce::TextButton catAll      { "All"    };
    juce::TextButton catInit    { "Init"   };
    juce::TextButton catBass    { "Bass"   };
    juce::TextButton catLead    { "Lead"   };
    juce::TextButton catPad     { "Pad"    };
    juce::TextButton catDrums   { "Drums"  };
    juce::TextButton catFX      { "FX"     };
    juce::TextButton catFavorites { "Favs" };
    juce::Array<juce::TextButton*> categoryButtons;

    // Action buttons (compact icon buttons in search bar)
    juce::TextButton saveButton    { "Save"   };
    juce::TextButton importButton  { "Import" };
    juce::TextButton exportButton  { "Export" };
    juce::ToggleButton autoPreviewButton { "Preview" };
    bool autoPreviewEnabled = true;

    // Preset list
    juce::ListBox    presetList  { "presets", this };

    // Count label
    juce::Label       countLabel;

    // Tag filter
    juce::StringArray activeTagFilters;
    juce::Component   tagFilterArea;  // container for tag pills

    // Metadata detail strip
    juce::Label authorLabel;
    juce::Label descriptionLabel;
    juce::Label tagsLabel;

    // Filtered preset indices
    juce::Array<int> filteredPresetIndices;

    // Auto-preview timer state
    int lastPreviewedRow = -1;

    // Timer for auto-preview
    void timerCallback() override;

    void rebuildFilter();
    void setActiveCategory(juce::TextButton* cat);
    void saveCurrentPreset();
    void loadSelectedPreset();
    void deleteSelectedPreset();
    void importPreset();
    void exportPreset();
    void updateMetadataStrip();
    void showSaveDialog();
    void showRightClickMenu(int row);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowserPanel)
};