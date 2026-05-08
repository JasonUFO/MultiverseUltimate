#pragma once
#include "Preset.h"
#include <vector>
#include <map>

class PresetManager
{
public:
    enum Bank { Factory, User };

    PresetManager();

    void loadPreset(const juce::String& path);
    void savePreset(const juce::String& path);

    void addPreset(const Preset& preset);
    const Preset& getCurrentPreset() const;
    Preset& getCurrentPreset();

    void nextPreset();
    void previousPreset();

    void saveState(const juce::String& name, const juce::MemoryBlock& state);
    bool loadState(int index, juce::MemoryBlock& state);
    void deletePreset(int index);
    void scanPresetsDirectory();

    juce::StringArray getPresetNames() const { return presetNames; }
    int getPresetCount() const              { return presetFiles.size(); }
    std::vector<Preset> getPresets() const  { return presets; }
    juce::String getPresetCategory(int index) const;

    // Bank support
    juce::StringArray getBankNames() const;
    void setCurrentBank(int bank);
    int getCurrentBank() const { return currentBank; }
    bool isCurrentBankReadOnly() const { return currentBank == Factory; }

    // Import/Export
    void importPreset(const juce::File& sourceFile);
    void exportPreset(int index, const juce::File& destFile);
    void exportBank(const juce::File& destDirectory);

    // Metadata
    struct PresetMetadata {
        juce::String name, category, author, description;
        juce::StringArray tags;
        juce::StringArray characters;
        int index = 0;
    };
    const PresetMetadata& getPresetMetadata(int index) const;
    const std::map<juce::String, std::vector<int>>& getTagIndex() const { return tagIndex; }
    juce::StringArray getAllTags() const;
    const std::map<juce::String, std::vector<int>>& getCharacterIndex() const { return characterIndex; }
    juce::StringArray getAllCharacters() const;

    // Favorites
    static constexpr int NUM_FAV_COLORS = 8;
    void loadFavorites();
    void saveFavorites();
    bool isFavorite(int presetIndex) const;
    int  getFavoriteColor(int presetIndex) const;   // 0-7, -1 if not favorite
    void setFavorite(int presetIndex, int colorIndex); // -1 to remove
    juce::Array<int> getFavoritesIndices() const;
    juce::String getRelativePresetPath(int index) const;

    // History
    static constexpr int MAX_HISTORY = 32;
    void pushHistory(int presetIndex);
    bool canGoBack() const;
    bool canGoForward() const;
    int goBack();
    int goForward();

    // Bookmarks
    struct BookmarkFolder {
        juce::String name;
        juce::StringArray presetPaths; // relative paths from presetsDirectory
    };
    void loadBookmarks();
    void saveBookmarks();
    const std::vector<BookmarkFolder>& getBookmarkFolders() const { return bookmarkFolders; }
    void createBookmarkFolder(const juce::String& name);
    void deleteBookmarkFolder(int index);
    void renameBookmarkFolder(int index, const juce::String& name);
    void addPresetToBookmark(int folderIndex, const juce::String& presetPath);
    void removePresetFromBookmark(int folderIndex, const juce::String& presetPath);

private:
    std::vector<Preset> presets;
    int currentPresetIndex = 0;
    juce::File presetsDirectory;
    juce::Array<juce::File> presetFiles;
    juce::StringArray       presetNames;

    // Metadata cache
    std::vector<PresetMetadata> presetMeta;
    std::map<juce::String, std::vector<int>> tagIndex;
    std::map<juce::String, std::vector<int>> characterIndex;
    void buildTagIndex();
    void buildCharacterIndex();
    void scanPresetMetadata();

    // Favorites
    struct FavoriteEntry {
        int colorIndex = 0;
        juce::String timestamp;
    };
    std::map<juce::String, FavoriteEntry> favorites;
    juce::File favoritesFile;

    // History
    std::vector<int> historyStack;
    int historyPosition = -1;

    // Bookmarks
    std::vector<BookmarkFolder> bookmarkFolders;
    juce::File bookmarksFile;

    int currentBank = User;
    juce::File getBankDirectory() const;
    void createFactoryPresetsIfNeeded();
};