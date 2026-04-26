#pragma once
#include "Preset.h"
#include <vector>

class PresetManager
{
public:
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
    
    std::vector<Preset> getPresets() const { return presets; }
    
private:
    std::vector<Preset> presets;
    int currentPresetIndex = 0;
    juce::File presetsDirectory;
};