#include "LayerManager.h"
#include "LayerEngine.h"

LayerManager::LayerManager()
{
    for (int i = 0; i < NUM_LAYERS; ++i)
        layers[i] = std::make_unique<LayerEngine>(i);
}

void LayerManager::prepare(double sampleRate, int samplesPerBlock)
{
    for (auto& l : layers)
        l->prepare(sampleRate, samplesPerBlock);
}

void LayerManager::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    buffer.clear();
    bool anySoloed = isAnyLayerSoloed();

    for (auto& layer : layers)
    {
        if (anySoloed && !layer->isSoloed()) continue;
        if (layer->isMuted()) continue;

        layer->processBlock(buffer, numSamples);
    }
}

void LayerManager::noteOn(int midiNote, float velocity, int midiChannel)
{
    for (auto& layer : layers)
    {
        if (layer->isMuted()) continue;
        if (isAnyLayerSoloed() && !layer->isSoloed()) continue;
        layer->noteOn(midiNote, velocity, midiChannel);
    }
}

void LayerManager::noteOff(int midiNote)
{
    for (auto& layer : layers)
        layer->noteOff(midiNote);
}

void LayerManager::allNotesOff()
{
    for (auto& layer : layers)
        layer->allNotesOff();
}

LayerEngine& LayerManager::getLayer(int index)
{
    return *layers[juce::jlimit(0, NUM_LAYERS - 1, index)];
}

const LayerEngine& LayerManager::getLayer(int index) const
{
    return *layers[juce::jlimit(0, NUM_LAYERS - 1, index)];
}

bool LayerManager::isAnyLayerSoloed() const
{
    for (auto& layer : layers)
        if (layer->isSoloed()) return true;
    return false;
}

juce::ValueTree LayerManager::getState() const
{
    juce::ValueTree tree("LayerManager");
    for (int i = 0; i < NUM_LAYERS; ++i)
        tree.appendChild(layers[i]->getState(), nullptr);
    return tree;
}

void LayerManager::setState(const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;
    for (int i = 0; i < NUM_LAYERS && i < tree.getNumChildren(); ++i)
        layers[i]->setState(tree.getChild(i));
}
