#pragma once
#include <JuceHeader.h>
#include <array>
#include <memory>
#include "LayerEngine.h"

class LayerManager
{
public:
    static constexpr int NUM_LAYERS = 8;

    LayerManager();
    ~LayerManager() = default;

    void prepare(double sampleRate, int samplesPerBlock);

    // Process all layers, mix to output
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples);

    // MIDI routing (midiChannel: 1-16; 0 = omni)
    void noteOn(int midiNote, float velocity, int midiChannel = 0);
    void noteOff(int midiNote);
    void allNotesOff();

    // Layer access
    LayerEngine& getLayer(int index);
    const LayerEngine& getLayer(int index) const;

    // Solo logic (if any layer is soloed, only play soloed layers)
    bool isAnyLayerSoloed() const;

    // State persistence
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& tree);

private:
    std::array<std::unique_ptr<LayerEngine>, NUM_LAYERS> layers;
};
