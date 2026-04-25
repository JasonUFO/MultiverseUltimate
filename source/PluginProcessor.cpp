#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdio>
#include <cstring>
#include "Sequencer/Sequencer.h"

// Force compile-time check
#if ! defined(JucePlugin_IsSynth)
 #error "JucePlugin_IsSynth is not defined!"
#endif

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                        )
{
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
    samplerEngine.prepare (sampleRate, samplesPerBlock);
    drumSequencer.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate, drumSequencer.getBPM());
    delay.prepare (sampleRate, samplesPerBlock);
    delay.reset();
    reverb.prepare (sampleRate, samplesPerBlock);
    reverb.reset();
    modulationMatrix.prepare (sampleRate, samplesPerBlock);
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

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            float vel = message.getVelocity() / 127.0f;
            sustainedNoteHeld[note] = false;
            synthEngine.noteOn (note, vel);
            samplerEngine.noteOn (note, vel);
        }
        else if (message.isNoteOff())
        {
            int note = message.getNoteNumber();
            if (sustainPedalDown)
            {
                sustainedNoteHeld[note] = true;
            }
            else
            {
                synthEngine.noteOff (note);
                samplerEngine.noteOff (note);
            }
        }
        else if (message.isPitchWheel())
        {
            // MIDI pitch wheel: 0–16383, center = 8192; map to ±2 semitones
            float semitones = ((message.getPitchWheelValue() - 8192) / 8192.0f) * 2.0f;
            synthEngine.setPitchBend (semitones);
        }
        else if (message.isController())
        {
            int cc  = message.getControllerNumber();
            int val = message.getControllerValue();

            if (cc == 64)  // sustain pedal
            {
                if (val >= 64)
                {
                    sustainPedalDown = true;
                }
                else
                {
                    sustainPedalDown = false;
                    for (int n = 0; n < 128; ++n)
                    {
                        if (sustainedNoteHeld[n])
                        {
                            synthEngine.noteOff (n);
                            samplerEngine.noteOff (n);
                            sustainedNoteHeld[n] = false;
                        }
                    }
                }
            }
            else if (cc == 123)  // all notes off
            {
                synthEngine.allNotesOff();
                samplerEngine.allNotesOff();
                for (int n = 0; n < 128; ++n)
                    sustainedNoteHeld[n] = false;
                sustainPedalDown = false;
            }
        }
    }

    synthEngine.setMasterVolume (masterVolume);
    samplerEngine.setMasterVolume (masterVolume);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    float modSums[MAX_MOD_TARGETS];

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Compute modulation sums from current source values
        modulationMatrix.computeModulationSums(modSums);

        // 2. Apply modulation to LFO rates (target indices: LFO1Rate=7, LFO2Rate=8, LFO3Rate=9, LFO4Rate=10)
        for (int lfo = 0; lfo < 4; ++lfo)
        {
            ModTargetType lfoRateTarget = static_cast<ModTargetType>(static_cast<int>(ModTargetType::LFO1Rate) + lfo);
            float mod = modSums[static_cast<int>(lfoRateTarget)];
            float newRate = baseLfoRates[lfo] + mod;
            newRate = juce::jlimit(0.01f, 100.0f, newRate);
            modulationMatrix.setLFORate(lfo, newRate);
        }

        // 3. Compute and apply filter modulation
        float effCutoff = baseFilterCutoff + modSums[static_cast<int>(ModTargetType::FilterCutoff)];
        effCutoff = juce::jlimit(20.0f, 20000.0f, effCutoff);
        float effRes = baseFilterResonance + modSums[static_cast<int>(ModTargetType::FilterResonance)];
        effRes = juce::jlimit(0.1f, 10.0f, effRes);
        synthEngine.setFilterParams(effCutoff, effRes);

        // 4. Compute and apply volume modulation
        float effVol = masterVolume + modSums[static_cast<int>(ModTargetType::AmpVolume)];
        effVol = juce::jlimit(0.0f, 1.0f, effVol);
        synthEngine.setMasterVolume(effVol);
        samplerEngine.setMasterVolume(effVol);

        // 5. Process synth and sampler (mono)
        float synthOut = synthEngine.process();
        float samplerOut = samplerEngine.process();

        // 6. Mix per channel with drums, apply effects
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float drumOut = buffer.getReadPointer(ch)[i];
            float mixed = drumOut + synthOut + samplerOut;
            mixed = delay.process(mixed);
            mixed = reverb.process(mixed);
            buffer.getWritePointer(ch)[i] = mixed;
        }

        // 7. Advance LFOs for next sample
        modulationMatrix.advanceLFOs();
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
    juce::ValueTree root("Multiverse");

    // Parameters from PluginProcessor
    juce::ValueTree params("Parameters");
    params.setProperty("masterVolume", masterVolume, nullptr);
    params.setProperty("baseFilterCutoff", baseFilterCutoff, nullptr);
    params.setProperty("baseFilterResonance", baseFilterResonance, nullptr);
    for (int i = 0; i < 4; ++i)
    {
        juce::String propName = "lfoRate" + juce::String(i+1);
        params.setProperty(propName, baseLfoRates[i], nullptr);
    }
    // Envelope from SynthEngine
    float envA, envD, envS, envR;
    synthEngine.getEnvelopeParams(envA, envD, envS, envR);
    params.setProperty("envA", envA, nullptr);
    params.setProperty("envD", envD, nullptr);
    params.setProperty("envS", envS, nullptr);
    params.setProperty("envR", envR, nullptr);
    // Waveform and mode
    params.setProperty("waveform", static_cast<int>(synthEngine.getWaveform()), nullptr);
    params.setProperty("synthMode", static_cast<int>(synthEngine.getSynthMode()), nullptr);
    params.setProperty("fmAlgorithm", synthEngine.getFMAlgorithm(), nullptr);
    // FM operator params
    for (int op = 0; op < 4; ++op)
    {
        float ratio, level, fb, att, dec, sus, rel;
        synthEngine.getFMOperatorParams(op, ratio, level, fb, att, dec, sus, rel);
        juce::ValueTree opNode("FmOp");
        opNode.setProperty("index", op, nullptr);
        opNode.setProperty("ratio", ratio, nullptr);
        opNode.setProperty("level", level, nullptr);
        opNode.setProperty("feedback", fb, nullptr);
        opNode.setProperty("attack", att, nullptr);
        opNode.setProperty("decay", dec, nullptr);
        opNode.setProperty("sustain", sus, nullptr);
        opNode.setProperty("release", rel, nullptr);
        params.appendChild(opNode, nullptr);
    }
    root.appendChild(params, nullptr);

    // Effects: Delay and Reverb
    juce::ValueTree effects("Effects");
    juce::ValueTree delayNode("Delay");
    delayNode.setProperty("time", delay.getTime(), nullptr);
    delayNode.setProperty("feedback", delay.getFeedback(), nullptr);
    delayNode.setProperty("mix", delay.getMix(), nullptr);
    effects.appendChild(delayNode, nullptr);
    juce::ValueTree reverbNode("Reverb");
    reverbNode.setProperty("roomSize", reverb.getRoomSize(), nullptr);
    reverbNode.setProperty("damping", reverb.getDamping(), nullptr);
    reverbNode.setProperty("wet", reverb.getWetLevel(), nullptr);
    reverbNode.setProperty("dry", reverb.getDryLevel(), nullptr);
    effects.appendChild(reverbNode, nullptr);
    root.appendChild(effects, nullptr);

    // Subsystem states
    root.appendChild(drumSequencer.getState(), nullptr);
    root.appendChild(modulationMatrix.getState(), nullptr);
    root.appendChild(sequencer.getState(), nullptr);

    auto xml = root.createXml();
    if (xml != nullptr)
    {
        juce::MemoryOutputStream stream;
        xml->writeTo(stream);
        destData.append(stream.getData(), stream.getDataSize());
    }
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::XmlDocument doc(juce::String::createStringFromData(data, sizeInBytes));
    std::unique_ptr<juce::XmlElement> xml(doc.getDocumentElement());
    if (xml == nullptr || !xml->hasTagName("Multiverse"))
        return;

    juce::ValueTree root = juce::ValueTree::fromXml(*xml);

    // Parameters
    auto params = root.getChildWithName("Parameters");
    if (params.isValid())
    {
        if (params.hasProperty("masterVolume"))
            masterVolume = (float)params.getProperty("masterVolume");
        if (params.hasProperty("baseFilterCutoff"))
            baseFilterCutoff = (float)params.getProperty("baseFilterCutoff");
        if (params.hasProperty("baseFilterResonance"))
            baseFilterResonance = (float)params.getProperty("baseFilterResonance");
        for (int i = 0; i < 4; ++i)
        {
            juce::String propName = "lfoRate" + juce::String(i+1);
            if (params.hasProperty(propName))
                baseLfoRates[i] = (float)params.getProperty(propName);
        }
        // Envelope
        if (params.hasProperty("envA"))
        {
            float a = (float)params.getProperty("envA");
            float d = (float)params.getProperty("envD");
            float s = (float)params.getProperty("envS");
            float r = (float)params.getProperty("envR");
            synthEngine.setEnvelopeParams(a, d, s, r);
        }
        // Waveform
        if (params.hasProperty("waveform"))
        {
            WaveformType wf = static_cast<WaveformType>((int)params.getProperty("waveform"));
            synthEngine.setWaveform(wf);
        }
        // Synth mode
        if (params.hasProperty("synthMode"))
        {
            SynthMode mode = static_cast<SynthMode>((int)params.getProperty("synthMode"));
            synthEngine.setSynthMode(mode);
        }
        // FM Algorithm
        if (params.hasProperty("fmAlgorithm"))
        {
            int alg = (int)params.getProperty("fmAlgorithm");
            synthEngine.setFMAlgorithm(alg);
        }
        // FM operator params
        for (auto opNode : params)
        {
            if (opNode.hasType("FmOp"))
            {
                int idx = (int)opNode.getProperty("index");
                if (idx >= 0 && idx < 4)
                {
                    float ratio = (float)opNode.getProperty("ratio");
                    float level = (float)opNode.getProperty("level");
                    float fb = (float)opNode.getProperty("feedback");
                    float att = (float)opNode.getProperty("attack");
                    float dec = (float)opNode.getProperty("decay");
                    float sus = (float)opNode.getProperty("sustain");
                    float rel = (float)opNode.getProperty("release");
                    synthEngine.setFMOperatorParams(idx, ratio, level, fb, att, dec, sus, rel);
                }
            }
        }
    }

    // Effects
    auto effects = root.getChildWithName("Effects");
    if (effects.isValid())
    {
        auto delayNode = effects.getChildWithName("Delay");
        if (delayNode.isValid())
        {
            if (delayNode.hasProperty("time"))
                delay.setTime((float)delayNode.getProperty("time"));
            if (delayNode.hasProperty("feedback"))
                delay.setFeedback((float)delayNode.getProperty("feedback"));
            if (delayNode.hasProperty("mix"))
                delay.setMix((float)delayNode.getProperty("mix"));
        }
        auto reverbNode = effects.getChildWithName("Reverb");
        if (reverbNode.isValid())
        {
            if (reverbNode.hasProperty("roomSize"))
                reverb.setRoomSize((float)reverbNode.getProperty("roomSize"));
            if (reverbNode.hasProperty("damping"))
                reverb.setDamping((float)reverbNode.getProperty("damping"));
            if (reverbNode.hasProperty("wet"))
                reverb.setWetLevel((float)reverbNode.getProperty("wet"));
            if (reverbNode.hasProperty("dry"))
                reverb.setDryLevel((float)reverbNode.getProperty("dry"));
        }
    }

    // Subsystems
    auto drumNode = root.getChildWithName("DrumSequencer");
    if (drumNode.isValid())
        drumSequencer.setState(drumNode);

    auto mmNode = root.getChildWithName("ModulationMatrix");
    if (mmNode.isValid())
        modulationMatrix.setState(mmNode);

    auto seqNode = root.getChildWithName("Sequencer");
    if (seqNode.isValid())
        sequencer.setState(seqNode);
}

//==============================================================================

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}