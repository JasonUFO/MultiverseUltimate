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
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"masterVolume", 1}, "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"attack", 1}, "Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.01f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"decay", 1}, "Decay",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sustain", 1}, "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"release", 1}, "Release",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.4f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filterCutoff", 1}, "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 20000.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filterResonance", 1}, "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.0f, 0.5f), 0.707f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayTime", 1}, "Delay Time",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.0f, 0.4f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayFeedback", 1}, "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f), 0.3f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayMix", 1}, "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbRoom", 1}, "Reverb Room",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbDamp", 1}, "Reverb Damp",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbWet", 1}, "Reverb Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.33f));

    for (int i = 1; i <= 4; ++i)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"lfo" + juce::String(i) + "Rate", 1},
            "LFO " + juce::String(i) + " Rate",
            juce::NormalisableRange<float>(0.01f, 20.0f, 0.0f, 0.3f), 1.0f));

    return layout;
}

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, &undoManager, "APVTSState", createParameterLayout())
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

    // Read automatable parameters from APVTS (atomic loads — safe on audio thread)
    const float masterVolume        = *apvts.getRawParameterValue("masterVolume");
    const float baseFilterCutoff    = *apvts.getRawParameterValue("filterCutoff");
    const float baseFilterResonance = *apvts.getRawParameterValue("filterResonance");
    const float baseDelayTime       = *apvts.getRawParameterValue("delayTime");
    const float baseDelayFeedback   = *apvts.getRawParameterValue("delayFeedback");
    const float baseDelayMix        = *apvts.getRawParameterValue("delayMix");
    const float baseReverbRoom      = *apvts.getRawParameterValue("reverbRoom");
    const float baseReverbDamp      = *apvts.getRawParameterValue("reverbDamp");
    const float baseReverbWet       = *apvts.getRawParameterValue("reverbWet");

    // Apply envelope and reverb damping once per block
    synthEngine.setEnvelopeParams(
        *apvts.getRawParameterValue("attack"),
        *apvts.getRawParameterValue("decay"),
        *apvts.getRawParameterValue("sustain"),
        *apvts.getRawParameterValue("release")
    );
    reverb.setDamping(baseReverbDamp);

    drumSequencer.process (buffer, buffer.getNumSamples());

    bool dawPlaying = false;
    double dawBPM = 120.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            dawBPM = pos->getBpm().orFallback(120.0);

            if (pos->getIsPlaying())
                dawPlaying = true;
        }
    }

    if (dawPlaying)
    {
        if (!sequencer.isPlaying())
            sequencer.start();

        sequencer.setBPM(static_cast<float>(dawBPM));
        sequencer.process (midiMessages, buffer.getNumSamples());
    }
    else if (sequencer.isPlaying())
    {
        sequencer.stop();
    }

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
            basePitchBend = ((message.getPitchWheelValue() - 8192) / 8192.0f) * 2.0f;
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

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Read LFO base rates once per block (atomic loads, safe on audio thread)
    const float lfoBaseRates[4] = {
        *apvts.getRawParameterValue("lfo1Rate"),
        *apvts.getRawParameterValue("lfo2Rate"),
        *apvts.getRawParameterValue("lfo3Rate"),
        *apvts.getRawParameterValue("lfo4Rate"),
    };

    // Per-block modulation: effect params applied here to avoid per-sample coefficient recalculation
    {
        float preSums[MAX_MOD_TARGETS] = {};
        modulationMatrix.computeModulationSums(preSums);
        const float effectMixMod = preSums[static_cast<int>(ModTargetType::EffectMix)];
        delay.setTime    (juce::jlimit(0.0f,  2.0f,  baseDelayTime     + preSums[static_cast<int>(ModTargetType::EffectParam1)]));
        delay.setFeedback(juce::jlimit(0.0f,  0.95f, baseDelayFeedback + preSums[static_cast<int>(ModTargetType::EffectParam2)]));
        delay.setMix     (juce::jlimit(0.0f,  1.0f,  baseDelayMix      + effectMixMod));
        reverb.setRoomSize(juce::jlimit(0.0f, 1.0f,  baseReverbRoom    + preSums[static_cast<int>(ModTargetType::EffectParam3)]));
        reverb.setWetLevel(juce::jlimit(0.0f, 1.0f,  baseReverbWet     + effectMixMod));
    }

    float modSums[MAX_MOD_TARGETS];

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Compute modulation sums from current source values
        modulationMatrix.computeModulationSums(modSums);

        {
            float waveIdx = static_cast<float>(baseWaveform)
                        + modSums[static_cast<int>(ModTargetType::OscillatorWaveform)];
            synthEngine.setWaveform(static_cast<WaveformType>(
                static_cast<int>(juce::jlimit(0.0f, 4.0f, waveIdx))));
        }

        // 2. Apply modulation to LFO rates
        for (int lfo = 0; lfo < 4; ++lfo)
        {
            ModTargetType lfoRateTarget = static_cast<ModTargetType>(static_cast<int>(ModTargetType::LFO1Rate) + lfo);
            float newRate = lfoBaseRates[lfo] + modSums[static_cast<int>(lfoRateTarget)];
            modulationMatrix.setLFORate(lfo, juce::jlimit(0.01f, 100.0f, newRate));
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
        // OscillatorLevel modulates synth oscillator gain independently of sampler
        float effSynthVol = juce::jlimit(0.0f, 1.0f, effVol + modSums[static_cast<int>(ModTargetType::OscillatorLevel)]);
        synthEngine.setMasterVolume(effSynthVol);
        samplerEngine.setMasterVolume(effVol);

        // 5. OscillatorPitch modulation (semitone offset added to MIDI pitch wheel base)
        synthEngine.setPitchBend(basePitchBend + modSums[static_cast<int>(ModTargetType::OscillatorPitch)]);

        // 6. AmpPan modulation: [-1, 1] — negative = left, positive = right
        const float pan = juce::jlimit(-1.0f, 1.0f, modSums[static_cast<int>(ModTargetType::AmpPan)]);

        // 7. Process synth and sampler (mono)
        float synthOut = synthEngine.process();
        float samplerOut = samplerEngine.process();

        // 8. Mix per channel with drums, apply effects and pan
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float drumOut = buffer.getReadPointer(ch)[i];
            float mixed = drumOut + synthOut + samplerOut;
            mixed = delay.process(mixed);
            mixed = reverb.process(mixed);
            const float panGain = (ch == 0) ? (1.0f - juce::jmax(0.0f, pan))
                                            : (1.0f + juce::jmin(0.0f, pan));
            buffer.getWritePointer(ch)[i] = mixed * panGain;
        }

        // 9. Advance LFOs for next sample
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

    // All APVTS parameters (masterVolume, ADSR, filter, delay, reverb)
    root.appendChild(apvts.copyState(), nullptr);

    // Non-APVTS synth params: mode, waveform, FM algorithm + operators, LFO rates
    juce::ValueTree synthParams("SynthParams");
    synthParams.setProperty("waveform",    static_cast<int>(synthEngine.getWaveform()),  nullptr);
    synthParams.setProperty("synthMode",   static_cast<int>(synthEngine.getSynthMode()), nullptr);
    synthParams.setProperty("fmAlgorithm", synthEngine.getFMAlgorithm(),                 nullptr);
    for (int op = 0; op < 4; ++op)
    {
        float ratio, level, fb, att, dec, sus, rel;
        synthEngine.getFMOperatorParams(op, ratio, level, fb, att, dec, sus, rel);
        juce::ValueTree opNode("FmOp");
        opNode.setProperty("index",    op,    nullptr);
        opNode.setProperty("ratio",    ratio, nullptr);
        opNode.setProperty("level",    level, nullptr);
        opNode.setProperty("feedback", fb,    nullptr);
        opNode.setProperty("attack",   att,   nullptr);
        opNode.setProperty("decay",    dec,   nullptr);
        opNode.setProperty("sustain",  sus,   nullptr);
        opNode.setProperty("release",  rel,   nullptr);
        synthParams.appendChild(opNode, nullptr);
    }
    root.appendChild(synthParams, nullptr);

    // Reverb dry level has no UI and is not automated — preserve current value
    juce::ValueTree reverbExtra("ReverbExtra");
    reverbExtra.setProperty("dry", reverb.getDryLevel(), nullptr);
    root.appendChild(reverbExtra, nullptr);

    // Subsystem states
    root.appendChild(drumSequencer.getState(), nullptr);
    root.appendChild(modulationMatrix.getState(), nullptr);
    root.appendChild(sequencer.getState(), nullptr);
    root.appendChild(samplerEngine.getState(), nullptr);

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

    // Restore APVTS parameters one-by-one so each change is recorded in undo history
    {
        auto apvtsState = apvts.state;
        auto savedState = root.getChildWithName("APVTSState");
        const int numParams = apvtsState.getNumChildren();
        const int numSaved = savedState.getNumChildren();
        for (int i = 0; i < numParams; ++i)
        {
            auto paramId = apvtsState.getChild(i).getProperty("id").toString();
            for (int j = 0; j < numSaved; ++j)
            {
                auto savedParam = savedState.getChild(j);
                if (savedParam.getProperty("id").toString() == paramId)
                {
                    if (savedParam.hasProperty("value"))
                        apvts.getParameter(paramId)->setValueNotifyingHost(savedParam.getProperty("value"));
                    break;
                }
            }
        }
    }

    // Restore non-APVTS synth params
    auto synthParams = root.getChildWithName("SynthParams");
    if (synthParams.isValid())
    {
        if (synthParams.hasProperty("waveform"))
            synthEngine.setWaveform(static_cast<WaveformType>((int)synthParams.getProperty("waveform")));
        if (synthParams.hasProperty("synthMode"))
            synthEngine.setSynthMode(static_cast<SynthMode>((int)synthParams.getProperty("synthMode")));
        if (synthParams.hasProperty("fmAlgorithm"))
            synthEngine.setFMAlgorithm((int)synthParams.getProperty("fmAlgorithm"));
        for (auto opNode : synthParams)
        {
            if (opNode.hasType("FmOp"))
            {
                int idx = (int)opNode.getProperty("index");
                if (idx >= 0 && idx < 4)
                    synthEngine.setFMOperatorParams(idx,
                        (float)opNode.getProperty("ratio"),
                        (float)opNode.getProperty("level"),
                        (float)opNode.getProperty("feedback"),
                        (float)opNode.getProperty("attack"),
                        (float)opNode.getProperty("decay"),
                        (float)opNode.getProperty("sustain"),
                        (float)opNode.getProperty("release"));
            }
        }
    }

    // Reverb dry level
    auto reverbExtra = root.getChildWithName("ReverbExtra");
    if (reverbExtra.isValid() && reverbExtra.hasProperty("dry"))
        reverb.setDryLevel((float)reverbExtra.getProperty("dry"));

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

    auto samplerNode = root.getChildWithName("SamplerEngine");
    if (samplerNode.isValid())
        samplerEngine.setState(samplerNode);
}

//==============================================================================
void PluginProcessor::saveNamedPreset(const juce::String& name)
{
    juce::MemoryBlock state;
    getStateInformation(state);
    presetManager.saveState(name, state);
}

bool PluginProcessor::loadPresetAtIndex(int index)
{
    juce::MemoryBlock state;
    if (!presetManager.loadState(index, state))
        return false;
    setStateInformation(state.getData(), (int)state.getSize());
    return true;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
