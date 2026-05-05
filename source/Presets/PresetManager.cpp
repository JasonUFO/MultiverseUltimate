#include "PresetManager.h"
#include "FactoryPresets.h"

PresetManager::PresetManager()
{
    presets.push_back(Preset("Init"));
    presetsDirectory = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                           .getChildFile("Library/Audio/Presets/MultiphaseAudio/MultiverseUltimate");
    presetsDirectory.createDirectory();
    createFactoryPresetsIfNeeded();
    scanPresetsDirectory();
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
        currentPresetIndex = (currentPresetIndex - 1 + (int)presets.size()) % presets.size();
    }
}

void PresetManager::saveState(const juce::String& name, const juce::MemoryBlock& state)
{
    auto bankDir = getBankDirectory();
    bankDir.createDirectory();

    // Determine category subfolder from current preset if loading from XML
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
}

void PresetManager::importPreset(const juce::File& sourceFile)
{
    if (!sourceFile.existsAsFile())
        return;

    auto bankDir = getBankDirectory();
    bankDir.createDirectory();

    juce::String category = "Init";
    if (auto xml = juce::XmlDocument::parse(sourceFile))
        category = xml->getStringAttribute("category", "Init");

    if (!juce::StringArray({ "Init", "Bass", "Lead", "Pad", "Drums", "FX" }).contains(category))
        category = "Init";

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

void PresetManager::exportBank(const juce::File& destDirectory)
{
    destDirectory.createDirectory();
    for (auto& f : presetFiles)
        f.copyFileTo(destDirectory.getChildFile(f.getFileName()));
}