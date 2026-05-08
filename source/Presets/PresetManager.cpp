#include "PresetManager.h"
#include "FactoryPresets.h"

PresetManager::PresetManager()
{
    presets.push_back(Preset("Init"));
    presetsDirectory = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                           .getChildFile("Library/Audio/Presets/MultiphaseAudio/MultiverseUltimate");
    presetsDirectory.createDirectory();
    favoritesFile = presetsDirectory.getChildFile("favorites.json");
    bookmarksFile = presetsDirectory.getChildFile("bookmarks.json");
    createFactoryPresetsIfNeeded();
    scanPresetsDirectory();
    loadFavorites();
    loadBookmarks();
}

juce::StringArray PresetManager::getBankNames() const
{
    return { "User", "Factory" };
}

void PresetManager::setCurrentBank(int bank)
{
    if (bank == Factory || bank == User)
    {
        currentBank = bank;
        scanPresetsDirectory();
    }
}

juce::File PresetManager::getBankDirectory() const
{
    if (currentBank == Factory)
        return presetsDirectory.getChildFile("Factory");
    return presetsDirectory.getChildFile("User");
}

void PresetManager::createFactoryPresetsIfNeeded()
{
    auto factoryDir = presetsDirectory.getChildFile("Factory");
    factoryDir.createDirectory();

    juce::StringArray categories = { "Init", "Bass", "Lead", "Pad", "Drums", "FX" };
    for (const auto& cat : categories)
    {
        auto dir = factoryDir.getChildFile(cat);
        if (!dir.exists())
            dir.createDirectory();
    }

    // Generate all factory presets if none exist yet (recursive search)
    juce::Array<juce::File> existing;
    factoryDir.findChildFiles(existing, juce::File::findFiles, true, "*.mvpreset");
    if (existing.size() < 10)
        FactoryPresets::writeToDirectory(factoryDir);
}

void PresetManager::loadPreset(const juce::String& path)
{
    juce::File file(path);
    if (!file.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml != nullptr)
    {
        Preset preset;
        preset.deserialize(*xml);
        addPreset(preset);
    }
}

void PresetManager::savePreset(const juce::String& path)
{
    juce::File file(path);
    juce::XmlElement xml("PRESET");
    getCurrentPreset().serialize(xml);
    file.replaceWithText(xml.toString());
}

void PresetManager::addPreset(const Preset& preset)
{
    presets.push_back(preset);
}

const Preset& PresetManager::getCurrentPreset() const
{
    if (presets.empty() || currentPresetIndex >= (int)presets.size())
    {
        static const Preset defaultPreset;
        return defaultPreset;
    }
    return presets[currentPresetIndex];
}

Preset& PresetManager::getCurrentPreset()
{
    if (presets.empty())
    {
        presets.push_back(Preset("Init"));
        currentPresetIndex = 0;
    }
    if (currentPresetIndex >= (int)presets.size())
        currentPresetIndex = (int)presets.size() - 1;
    return presets[currentPresetIndex];
}

void PresetManager::nextPreset()
{
    if (!presets.empty())
    {
        currentPresetIndex = (currentPresetIndex + 1) % presets.size();
    }
}

void PresetManager::previousPreset()
{
    if (!presets.empty())
    {
        currentPresetIndex = (currentPresetIndex - 1 + (int)presets.size()) % (int)presets.size();
    }
}

void PresetManager::saveState(const juce::String& name, const juce::MemoryBlock& state)
{
    auto bankDir = getBankDirectory();
    bankDir.createDirectory();

    // Determine category from the XML root attribute
    juce::String category = "Init";
    auto tempFile = bankDir.getChildFile("_temp_.mvpreset");
    tempFile.replaceWithData(state.getData(), state.getSize());

    if (auto xml = juce::XmlDocument::parse(tempFile))
    {
        category = xml->getStringAttribute("category", "Init");
        if (!juce::StringArray({ "Init", "Bass", "Lead", "Pad", "Drums", "FX" }).contains(category))
            category = "Init";
    }
    tempFile.deleteFile();

    auto catDir = bankDir.getChildFile(category);
    catDir.createDirectory();
    auto file = catDir.getChildFile(name + ".mvpreset");
    file.replaceWithData(state.getData(), state.getSize());
    scanPresetsDirectory();
}

bool PresetManager::loadState(int index, juce::MemoryBlock& state)
{
    if (index < 0 || index >= presetFiles.size())
        return false;
    return presetFiles[index].loadFileAsData(state);
}

void PresetManager::deletePreset(int index)
{
    if (index >= 0 && index < presetFiles.size())
    {
        presetFiles[index].deleteFile();
        scanPresetsDirectory();
    }
}

void PresetManager::scanPresetsDirectory()
{
    presetFiles.clear();
    presetNames.clear();

    auto bankDir = getBankDirectory();
    if (!bankDir.isDirectory())
        bankDir.createDirectory();

    juce::Array<juce::File> found;

    // Scan category subdirectories
    juce::StringArray categories = { "Init", "Bass", "Lead", "Pad", "Drums", "FX" };
    for (const auto& cat : categories)
    {
        auto catDir = bankDir.getChildFile(cat);
        if (catDir.isDirectory())
        {
            juce::Array<juce::File> catFiles;
            catDir.findChildFiles(catFiles, juce::File::findFiles, false, "*.mvpreset");
            found.addArray(catFiles);
        }
    }

    // Also scan root of bank dir for unorganized presets
    juce::Array<juce::File> rootFiles;
    bankDir.findChildFiles(rootFiles, juce::File::findFiles, false, "*.mvpreset");
    found.addArray(rootFiles);

    struct AlphaSort
    {
        static int compareElements(const juce::File& a, const juce::File& b)
        {
            return a.getFileNameWithoutExtension()
                    .compareNatural(b.getFileNameWithoutExtension());
        }
    } cmp;
    found.sort(cmp);

    for (auto& f : found)
    {
        presetFiles.add(f);
        presetNames.add(f.getFileNameWithoutExtension());
    }

    // Build metadata cache
    scanPresetMetadata();
    buildTagIndex();
    buildCharacterIndex();
}

void PresetManager::scanPresetMetadata()
{
    presetMeta.clear();
    presetMeta.resize(presetFiles.size());

    static const juce::StringArray validCategories { "Init", "Bass", "Lead", "Pad", "Drums", "FX" };

    for (int i = 0; i < presetFiles.size(); ++i)
    {
        auto& m = presetMeta[i];
        m.index = i;
        m.name = presetNames[i];

        // Default category from directory name
        auto parentName = presetFiles[i].getParentDirectory().getFileName();
        m.category = validCategories.contains(parentName) ? parentName : juce::String("Init");
        m.author = "MultiphaseAudio";

        // Parse XML for metadata (only root element attributes)
        if (auto xml = juce::XmlDocument::parse(presetFiles[i]))
        {
            juce::String xmlCat = xml->getStringAttribute("category", "");
            if (validCategories.contains(xmlCat))
                m.category = xmlCat;

            juce::String xmlAuthor = xml->getStringAttribute("author", "");
            if (xmlAuthor.isNotEmpty())
                m.author = xmlAuthor;

            m.description = xml->getStringAttribute("description", "");

            // Parse tags from attribute
            juce::String tagsStr = xml->getStringAttribute("tags", "");
            if (tagsStr.isNotEmpty())
            {
                auto parts = juce::StringArray::fromTokens(tagsStr, ",", "");
                for (auto& p : parts)
                {
                    p = p.trim().toLowerCase();
                    if (p.isNotEmpty() && !m.tags.contains(p))
                        m.tags.add(p);
                }
            }

            // Extract #hashtags from description
            if (m.description.contains("#"))
            {
                auto words = juce::StringArray::fromTokens(m.description, " ,.;:!?\n\r\t", "'");
                for (auto& w : words)
                {
                    if (w.startsWith("#") && w.length() > 1)
                    {
                        auto tag = w.substring(1).toLowerCase();
                        if (tag.isNotEmpty() && !m.tags.contains(tag))
                            m.tags.add(tag);
                    }
                }
            }

            // Parse characters from attribute
            juce::String charStr = xml->getStringAttribute("characters", "");
            if (charStr.isNotEmpty())
            {
                auto parts = juce::StringArray::fromTokens(charStr, ",", "");
                for (auto& p : parts)
                {
                    p = p.trim().toLowerCase();
                    if (p.isNotEmpty() && !m.characters.contains(p))
                        m.characters.add(p);
                }
            }
        }
    }
}

void PresetManager::buildTagIndex()
{
    tagIndex.clear();
    for (int i = 0; i < (int)presetMeta.size(); ++i)
    {
        for (const auto& tag : presetMeta[i].tags)
        {
            tagIndex[tag].push_back(i);
        }
    }
}

const PresetManager::PresetMetadata& PresetManager::getPresetMetadata(int index) const
{
    if (index >= 0 && index < (int)presetMeta.size())
        return presetMeta[index];
    static const PresetMetadata empty;
    return empty;
}

juce::StringArray PresetManager::getAllTags() const
{
    juce::StringArray result;
    for (const auto& entry : tagIndex)
        result.add(entry.first);
    result.sortNatural();
    return result;
}

void PresetManager::buildCharacterIndex()
{
    characterIndex.clear();
    for (int i = 0; i < (int)presetMeta.size(); ++i)
    {
        for (const auto& ch : presetMeta[i].characters)
        {
            characterIndex[ch].push_back(i);
        }
    }
}

juce::StringArray PresetManager::getAllCharacters() const
{
    juce::StringArray result;
    for (const auto& entry : characterIndex)
        result.add(entry.first);
    result.sortNatural();
    return result;
}

//==============================================================================
// Favorites

void PresetManager::loadFavorites()
{
    favorites.clear();
    if (!favoritesFile.existsAsFile())
        return;

    auto json = juce::JSON::parse(favoritesFile.loadFileAsString());
    if (auto* obj = json.getDynamicObject())
    {
        auto entries = obj->getProperty("entries");
        if (auto* entriesObj = entries.getDynamicObject())
        {
            for (const auto& key : entriesObj->getProperties())
            {
                auto value = entriesObj->getProperty(key.name);
                if (auto* entry = value.getDynamicObject())
                {
                    FavoriteEntry fe;
                    fe.colorIndex = (int)entry->getProperty("color");
                    fe.timestamp = entry->getProperty("added").toString();
                    if (fe.colorIndex >= 0 && fe.colorIndex < NUM_FAV_COLORS)
                        favorites[key.name.toString()] = fe;
                }
            }
        }
    }
}

void PresetManager::saveFavorites()
{
    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);

    auto* entries = new juce::DynamicObject();
    for (const auto& [path, entry] : favorites)
    {
        auto* e = new juce::DynamicObject();
        e->setProperty("color", entry.colorIndex);
        e->setProperty("added", entry.timestamp);
        entries->setProperty(path, e);
    }
    root->setProperty("entries", entries);

    juce::String jsonText = juce::JSON::toString(juce::var(root), false);
    favoritesFile.replaceWithText(jsonText);
}

bool PresetManager::isFavorite(int presetIndex) const
{
    return getFavoriteColor(presetIndex) >= 0;
}

int PresetManager::getFavoriteColor(int presetIndex) const
{
    auto relPath = getRelativePresetPath(presetIndex);
    if (relPath.isEmpty())
        return -1;
    auto it = favorites.find(relPath);
    return (it != favorites.end()) ? it->second.colorIndex : -1;
}

void PresetManager::setFavorite(int presetIndex, int colorIndex)
{
    auto relPath = getRelativePresetPath(presetIndex);
    if (relPath.isEmpty())
        return;

    if (colorIndex < 0 || colorIndex >= NUM_FAV_COLORS)
    {
        favorites.erase(relPath);
    }
    else
    {
        FavoriteEntry fe;
        fe.colorIndex = colorIndex;
        juce::Time now = juce::Time::getCurrentTime();
        fe.timestamp = now.toISO8601(true);
        favorites[relPath] = fe;
    }
    saveFavorites();
}

juce::Array<int> PresetManager::getFavoritesIndices() const
{
    juce::Array<int> result;
    for (int i = 0; i < presetFiles.size(); ++i)
    {
        if (isFavorite(i))
            result.add(i);
    }
    return result;
}

juce::String PresetManager::getRelativePresetPath(int index) const
{
    if (index < 0 || index >= presetFiles.size())
        return {};
    return presetFiles[index].getRelativePathFrom(presetsDirectory);
}

//==============================================================================
// History

void PresetManager::pushHistory(int presetIndex)
{
    // Truncate forward history
    if (historyPosition >= 0 && historyPosition < (int)historyStack.size())
        historyStack.resize(historyPosition + 1);

    // Don't push duplicates at the same position
    if (!historyStack.empty() && historyStack.back() == presetIndex)
        return;

    historyStack.push_back(presetIndex);
    if ((int)historyStack.size() > MAX_HISTORY)
        historyStack.erase(historyStack.begin());

    historyPosition = (int)historyStack.size() - 1;
}

bool PresetManager::canGoBack() const
{
    return historyPosition > 0;
}

bool PresetManager::canGoForward() const
{
    return historyPosition >= 0 && historyPosition < (int)historyStack.size() - 1;
}

int PresetManager::goBack()
{
    if (!canGoBack())
        return currentPresetIndex;
    historyPosition--;
    return historyStack[historyPosition];
}

int PresetManager::goForward()
{
    if (!canGoForward())
        return currentPresetIndex;
    historyPosition++;
    return historyStack[historyPosition];
}

//==============================================================================
// Import/Export

void PresetManager::importPreset(const juce::File& sourceFile)
{
    if (!sourceFile.existsAsFile())
        return;

    auto bankDir = getBankDirectory();
    bankDir.createDirectory();

    juce::String category = "Init";
    if (auto xml = juce::XmlDocument::parse(sourceFile))
    {
        juce::String xmlCat = xml->getStringAttribute("category", "");
        if (juce::StringArray({ "Init", "Bass", "Lead", "Pad", "Drums", "FX" }).contains(xmlCat))
            category = xmlCat;
    }

    auto catDir = bankDir.getChildFile(category);
    catDir.createDirectory();
    auto destFile = catDir.getChildFile(sourceFile.getFileName());
    sourceFile.copyFileTo(destFile);
    scanPresetsDirectory();
}

void PresetManager::exportPreset(int index, const juce::File& destFile)
{
    if (index < 0 || index >= presetFiles.size())
        return;
    presetFiles[index].copyFileTo(destFile);
}

juce::String PresetManager::getPresetCategory(int index) const
{
    if (index >= 0 && index < (int)presetMeta.size())
        return presetMeta[index].category;
    if (index < 0 || index >= presetFiles.size())
        return "Init";
    auto parentName = presetFiles[index].getParentDirectory().getFileName();
    static const juce::StringArray valid { "Init", "Bass", "Lead", "Pad", "Drums", "FX" };
    return valid.contains(parentName) ? parentName : "Init";
}

void PresetManager::exportBank(const juce::File& destDirectory)
{
    destDirectory.createDirectory();
    for (auto& f : presetFiles)
        f.copyFileTo(destDirectory.getChildFile(f.getFileName()));
}

//==============================================================================
// Bookmarks

void PresetManager::loadBookmarks()
{
    bookmarkFolders.clear();
    if (!bookmarksFile.existsAsFile())
        return;

    auto json = juce::JSON::parse(bookmarksFile.loadFileAsString());
    if (auto* obj = json.getDynamicObject())
    {
        auto folders = obj->getProperty("folders");
        if (auto* arr = folders.getArray())
        {
            for (const auto& item : *arr)
            {
                if (auto* folder = item.getDynamicObject())
                {
                    BookmarkFolder bf;
                    bf.name = folder->getProperty("name").toString();
                    auto paths = folder->getProperty("presets");
                    if (auto* pathsArr = paths.getArray())
                    {
                        for (const auto& p : *pathsArr)
                            bf.presetPaths.add(p.toString());
                    }
                    if (bf.name.isNotEmpty())
                        bookmarkFolders.push_back(bf);
                }
            }
        }
    }
}

void PresetManager::saveBookmarks()
{
    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);

    juce::Array<juce::var> foldersArr;
    for (const auto& folder : bookmarkFolders)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("name", folder.name);
        juce::Array<juce::var> pathsArr;
        for (const auto& path : folder.presetPaths)
            pathsArr.add(juce::var(path));
        obj->setProperty("presets", pathsArr);
        foldersArr.add(juce::var(obj));
    }
    root->setProperty("folders", foldersArr);

    juce::String jsonText = juce::JSON::toString(juce::var(root), false);
    bookmarksFile.replaceWithText(jsonText);
}

void PresetManager::createBookmarkFolder(const juce::String& name)
{
    bookmarkFolders.push_back({name, {}});
    saveBookmarks();
}

void PresetManager::deleteBookmarkFolder(int index)
{
    if (index >= 0 && index < (int)bookmarkFolders.size())
    {
        bookmarkFolders.erase(bookmarkFolders.begin() + index);
        saveBookmarks();
    }
}

void PresetManager::renameBookmarkFolder(int index, const juce::String& name)
{
    if (index >= 0 && index < (int)bookmarkFolders.size())
    {
        bookmarkFolders[index].name = name;
        saveBookmarks();
    }
}

void PresetManager::addPresetToBookmark(int folderIndex, const juce::String& presetPath)
{
    if (folderIndex >= 0 && folderIndex < (int)bookmarkFolders.size())
    {
        if (!bookmarkFolders[folderIndex].presetPaths.contains(presetPath))
        {
            bookmarkFolders[folderIndex].presetPaths.add(presetPath);
            saveBookmarks();
        }
    }
}

void PresetManager::removePresetFromBookmark(int folderIndex, const juce::String& presetPath)
{
    if (folderIndex >= 0 && folderIndex < (int)bookmarkFolders.size())
    {
        bookmarkFolders[folderIndex].presetPaths.removeString(presetPath);
        saveBookmarks();
    }
}