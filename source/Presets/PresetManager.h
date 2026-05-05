#pragma once
#include "Preset.h"
#include <vector>

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

private:
    std::vector<Preset> presets;
    int currentPresetIndex = 0;
    juce::File presetsDirectory;
    juce::Array<juce::File> presetFiles;
    juce::StringArray       presetNames;

    int currentBank = User;
    juce::File getBankDirectory() const;
    void createFactoryPresetsIfNeeded();
};