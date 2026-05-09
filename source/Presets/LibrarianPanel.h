#pragma once
#include <JuceHeader.h>

class PluginProcessor;
class LibrarianPanel;

//==============================================================================
// LibrarianPresetList — scrollable content with collapsible sections
//==============================================================================
class LibrarianPresetList : public juce::Component
{
public:
    explicit LibrarianPresetList(LibrarianPanel& owner);

    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    void rebuildSections();
    void selectPreset(int presetIndex);
    int getSelectedPresetIndex() const { return selectedPresetIndex; }

private:
    LibrarianPanel& owner;

    struct Section {
        juce::String title;
        bool isExpanded = true;
        struct SubSection {
            juce::String title;
            bool isExpanded = true;
            juce::Array<int> presetIndices;
            int bookmarkFolderIndex = -1;
        };
        juce::Array<SubSection> subSections;
        bool isBookmark = false;
        int bookmarkFolderIndex = -1;
    };

    juce::Array<Section> sections;
    int selectedPresetIndex = -1;
    int hoverPresetIndex = -1;

    static constexpr int sectionHeaderH = 22;
    static constexpr int subSectionHeaderH = 20;
    static constexpr int presetRowH = 28;
    static constexpr int indent = 12;

    struct HitZone {
        enum Type { kSectionHeader, kSubSectionHeader, kPresetRow, kNothing };
        Type type = kNothing;
        int sectionIdx = -1;
        int subIdx = -1;
        int presetIndex = -1;
        bool isInBookmarkSection = false;
        int bookmarkFolderIdx = -1;
    };

    HitZone hitTest(const juce::Point<int>& pos) const;
    void drawSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
        const juce::String& title, bool expanded);
    void drawSubSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
        const juce::String& title, bool expanded);
    void drawPresetRow(juce::Graphics& g, const juce::Rectangle<int>& bounds,
        int presetIndex, bool selected, bool hovered);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibrarianPresetList)
};

//==============================================================================
// LibrarianPanel
//==============================================================================
class LibrarianPanel : public juce::Component,
                       public juce::Button::Listener,
                       public juce::TextEditor::Listener,
                       private juce::Timer
{
public:
    // Character filter — public so LibrarianPresetList can access
    enum class CharFilter { None, Include, Exclude };
    struct CharPill {
        juce::TextButton button;
        CharFilter state = CharFilter::None;
        juce::String character;
    };

    explicit LibrarianPanel(PluginProcessor& p);

    void paint(juce::Graphics&) override;
    void resized() override;

    // Button::Listener
    void buttonClicked(juce::Button*) override;

    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor&) override;
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void textEditorEscapeKeyPressed(juce::TextEditor&) override;
    void textEditorFocusLost(juce::TextEditor&) override;

    void refresh();
    void focusSearchEditor();
    int getSelectedPresetIndex() const;
    bool isAutoPreviewEnabled() const { return autoPreviewEnabled; }

    // Tag filter management
    void addTagFilter(const juce::String& tag);
    void removeTagFilter(const juce::String& tag);
    void clearTagFilters();
    juce::StringArray getActiveTagFilters() const { return activeTagFilters; }

    // Public actions
    void saveCurrentPreset();
    void importPreset();
    void exportPreset();

    // Accessed by LibrarianPresetList
    PluginProcessor& processorRef;
    juce::String activeCategory { "All" };
    bool favFilterActive = false;
    juce::TextEditor searchEditor;
    juce::Array<CharPill*> characterPills;
    juce::StringArray activeTagFilters;

    // Favorite colors — public for LibrarianPresetList
    static const juce::Colour favColors[8];

    // Methods accessed by LibrarianPresetList
    void loadPresetByIndex(int presetIndex);
    void showPresetContextMenu(int presetIndex, const juce::Point<int>& screenPos, int bookmarkFolderIdx = -1);
    void showBookmarkFolderMenu(int folderIndex, const juce::Point<int>& screenPos);
    void updateMetadataStrip();
    void rebuildFilter();
    void promptNewBookmarkFolder();

private:
    CharPill charDark, charBright, charActive, charSpiky, charWide, charDirty;

    // History navigation
    juce::TextButton historyBack    { "\xe2\x97\x80" }; // ◀
    juce::TextButton historyForward { "\xe2\x96\xb6" }; // ▶

    // Category filter pills
    juce::TextButton catAll      { "All"   };
    juce::TextButton catBass     { "Bass"  };
    juce::TextButton catLead     { "Lead"  };
    juce::TextButton catPad      { "Pad"   };
    juce::TextButton catDrums    { "Drums" };
    juce::TextButton catFX       { "FX"    };
    juce::TextButton catKeys     { "Keys"  };
    juce::TextButton catArp      { "Arp"   };
    juce::Array<juce::TextButton*> categoryButtons;

    // Favorite filter
    juce::TextButton favFilterButton { "\xe2\x99\xa5" }; // ♥

    // Auto-preview
    juce::ToggleButton autoPreviewButton { "Auto" };
    bool autoPreviewEnabled = true;

    // Preset list (custom scrollable component with viewport)
    LibrarianPresetList presetListContent;
    juce::Viewport presetListViewport;

    // Tag filter area
    juce::Component tagFilterArea;

    // Metadata detail strip
    juce::Label authorLabel;
    juce::Label descriptionLabel;
    juce::Label tagsLabel;
    juce::Label charactersLabel;

    // Auto-preview timer state
    int lastPreviewedIndex = -1;

    // Timer for auto-preview
    void timerCallback() override;

    void setActiveCategory(juce::TextButton* cat);
    void cycleCharacter(CharPill& pill);

    void showSaveDialog();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibrarianPanel)
};