#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>
#include <cstring>

// Force compile-time check
#if ! defined(JucePlugin_IsSynth)
 #error "JucePlugin_IsSynth is not defined!"
#endif

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    fprintf(stderr, "[Multiverse] CONSTRUCTOR: IsSynth=%d\n", JucePlugin_IsSynth);
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
    return true;
}

bool PluginProcessor::producesMidi() const
{
    return false;
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 2.0;
}

int PluginProcessor::getNumPrograms()
{
    return 128;
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return "Program " + juce::String (index);
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    if (sampleRate <= 0.0 || sampleRate > 1000000.0)
    {
        sampleRate = 44100.0;
    }
    if (samplesPerBlock <= 0)
    {
        samplesPerBlock = 512;
    }

    synthEngine.prepare (sampleRate, samplesPerBlock);
    drumSequencer.prepare (sampleRate, samplesPerBlock);
    delay.prepare (sampleRate, samplesPerBlock);
    delay.reset();
    reverb.prepare (sampleRate, samplesPerBlock);
    reverb.reset();
}

void PluginProcessor::releaseResources()
{
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
    {
        return;
    }

    buffer.clear();

    drumSequencer.process (buffer, buffer.getNumSamples());

    for (const auto& metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            float vel = message.getVelocity() / 127.0f;
            synthEngine.noteOn (note, vel);
        }
        else if (message.isNoteOff())
        {
            synthEngine.noteOff (message.getNoteNumber());
        }
    }

    synthEngine.setMasterVolume (masterVolume);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float output = synthEngine.process();
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            if (data != nullptr)
                data[i] += output;
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    destData.append (reinterpret_cast<const void*> (&masterVolume), sizeof (float));
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (sizeInBytes >= static_cast<int> (sizeof (float)))
    {
        std::memcpy (&masterVolume, data, sizeof (float));
        synthEngine.setMasterVolume (masterVolume);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}