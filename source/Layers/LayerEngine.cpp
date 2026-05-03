#include "LayerEngine.h"

LayerEngine::LayerEngine(int idx)
    : layerIndex(idx)
{
    synthEngine = std::make_unique<SynthEngine>();
    granularEngine = std::make_unique<GranularEngine>();
    samplerEngine = std::make_unique<SamplerEngine>();
}

void LayerEngine::prepare(double sampleRate, int samplesPerBlock)
{
    synthEngine->prepare(sampleRate, samplesPerBlock);
    granularEngine->prepare(sampleRate, samplesPerBlock);
    samplerEngine->prepare(sampleRate, samplesPerBlock);
}

void LayerEngine::setEngineType(LayerEngineType type)
{
    if (type == engineType) return;
    engineType = type;

    // Reset other engines when switching
    if (type != LayerEngineType::Synth) synthEngine->allNotesOff();
    if (type != LayerEngineType::Granular) granularEngine->allNotesOff();
    if (type != LayerEngineType::Sampler) { /* sampler all notes off if available */ }
}

int LayerEngine::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (mute || engineType == LayerEngineType::Off)
        return 0;

    juce::AudioBuffer<float> layerBuf;
    layerBuf.setSize(2, numSamples);
    layerBuf.clear();

    int samplesProduced = 0;

    if (engineType == LayerEngineType::Synth)
    {
        samplesProduced = synthEngine->processBuffer(layerBuf, numSamples);
    }
    else if (engineType == LayerEngineType::Granular)
    {
        granularEngine->processBuffer(layerBuf, numSamples);
        samplesProduced = numSamples;
    }
    else if (engineType == LayerEngineType::Sampler)
    {
        samplesProduced = samplerEngine->processBuffer(layerBuf, numSamples);
    }

    if (samplesProduced > 0)
    {
        applyPan(layerBuf, samplesProduced);
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
            buffer.addFrom(ch, 0, layerBuf, ch, 0, samplesProduced, level);
    }

    return samplesProduced;
}

void LayerEngine::noteOn(int midiNote, float velocity)
{
    if (engineType == LayerEngineType::Synth)
        synthEngine->noteOn(midiNote, velocity);
    else if (engineType == LayerEngineType::Granular)
        granularEngine->noteOn(midiNote, velocity);
    else if (engineType == LayerEngineType::Sampler)
        samplerEngine->noteOn(midiNote, velocity);
}

void LayerEngine::noteOff(int midiNote)
{
    if (engineType == LayerEngineType::Synth)
        synthEngine->noteOff(midiNote);
    else if (engineType == LayerEngineType::Granular)
        granularEngine->noteOff(midiNote);
    else if (engineType == LayerEngineType::Sampler)
        samplerEngine->noteOff(midiNote);
}

void LayerEngine::allNotesOff()
{
    synthEngine->allNotesOff();
    granularEngine->allNotesOff();
    if (samplerEngine)
        samplerEngine->allNotesOff();
}

void LayerEngine::setModulationValue(int target, float value)
{
    // Route to underlying engine's modulation matrix if available
    if (engineType == LayerEngineType::Synth)
    {
        // synthEngine->getModulationMatrix()->setSourceValue(...)
    }
}

void LayerEngine::applyPan(juce::AudioBuffer<float>& buffer, int numSamples)
{
    float panL = 1.0f, panR = 1.0f;
    if (pan < 0.0f)       { panL = 1.0f; panR = 1.0f + pan; }
    else if (pan > 0.0f)  { panL = 1.0f - pan; panR = 1.0f; }

    auto* l = buffer.getWritePointer(0);
    auto* r = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        l[i] *= panL;
        if (r) r[i] *= panR;
    }
}

juce::ValueTree LayerEngine::getState() const
{
    juce::ValueTree tree("LayerEngine");
    tree.setProperty("type",  static_cast<int>(engineType), nullptr);
    tree.setProperty("level", level,  nullptr);
    tree.setProperty("pan",   pan,    nullptr);
    tree.setProperty("mute",  mute,   nullptr);
    tree.setProperty("solo",  solo,   nullptr);

    if (granularEngine) tree.appendChild(granularEngine->getState(), nullptr);
    if (samplerEngine)  tree.appendChild(samplerEngine->getState(),  nullptr);

    return tree;
}

void LayerEngine::setState(const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;
    engineType = static_cast<LayerEngineType>(static_cast<int>(tree.getProperty("type",  0)));
    level = static_cast<float>(tree.getProperty("level", 1.0f));
    pan   = static_cast<float>(tree.getProperty("pan",   0.0f));
    mute  = static_cast<bool> (tree.getProperty("mute",  false));
    solo  = static_cast<bool> (tree.getProperty("solo",  false));

    if (granularEngine) granularEngine->setState(tree.getChildWithName("GranularEngine"));
    if (samplerEngine)  samplerEngine->setState(tree.getChildWithName("SamplerEngine"));
}
