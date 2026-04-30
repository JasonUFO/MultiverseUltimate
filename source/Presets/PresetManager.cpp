#include "PresetManager.h"

PresetManager::PresetManager()
{
    presets.push_back(Preset("Init"));
    presetsDirectory = juce::File::getSpecialLocation(juce::File::userHomeDirectory)
                           .getChildFile("Library/Audio/Presets/MultiphaseAudio/MultiverseUltimate");
    presetsDirectory.createDirectory();
    scanPresetsDirectory();
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
    presetsDirectory.createDirectory();
    auto file = presetsDirectory.getChildFile(name + ".mvpreset");
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
    if (!presetsDirectory.isDirectory())
        return;

    juce::Array<juce::File> found;
    presetsDirectory.findChildFiles(found, juce::File::findFiles, false, "*.mvpreset");

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