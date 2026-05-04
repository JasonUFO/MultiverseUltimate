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

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"melodicSequencer", 1}, "Melodic Seq",
        juce::StringArray{"Pro Seq", "Arpeggiator", "Pattern"}, 2));

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

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbPreDelay", 1}, "Reverb Pre-Delay",
        juce::NormalisableRange<float>(0.0f, 200.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbLFDamp", 1}, "Reverb LF Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbWidth", 1}, "Reverb Width",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"reverbFreeze", 1}, "Reverb Freeze", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"chorusRate",  1}, "Chorus Rate",
        juce::NormalisableRange<float>(0.1f, 5.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"chorusDepth", 1}, "Chorus Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"chorusMix",   1}, "Chorus Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distDrive", 1}, "Distortion Drive",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.0f, 0.3f), 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distTone",  1}, "Distortion Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"distMix",   1}, "Distortion Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"eqLowGain",  1}, "EQ Low Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"eqMidGain",  1}, "EQ Mid Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"eqHighGain", 1}, "EQ High Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compThreshold", 1}, "Comp Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f), -20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compRatio",     1}, "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.0f, 0.4f), 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compAttack",    1}, "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.0f, 0.5f), 10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compRelease",   1}, "Comp Release",
        juce::NormalisableRange<float>(10.0f, 1000.0f, 0.0f, 0.4f), 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"compMakeup",    1}, "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 24.0f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"samplerVolume", 1}, "Sampler Volume",
        juce::NormalisableRange<float>(0.0f, 2.0f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"samplerPan", 1}, "Sampler Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    for (int i = 1; i <= 4; ++i)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"lfo" + juce::String(i) + "Rate", 1},
            "LFO " + juce::String(i) + " Rate",
            juce::NormalisableRange<float>(0.01f, 20.0f, 0.0f, 0.3f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"fmAlgorithm", 1}, "FM Algorithm",
        juce::StringArray{"1", "2", "3", "4", "5", "6", "7", "8"}, 0));

    for (int op = 1; op <= 4; ++op)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Ratio", 1},
            "FM Op " + juce::String(op) + " Ratio",
            juce::NormalisableRange<float>(0.5f, 32.0f, 0.0f, 0.5f), 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Level", 1},
            "FM Op " + juce::String(op) + " Level",
            juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Feedback", 1},
            "FM Op " + juce::String(op) + " Feedback",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Attack", 1},
            "FM Op " + juce::String(op) + " Attack",
            juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.01f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Decay", 1},
            "FM Op " + juce::String(op) + " Decay",
            juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.1f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Sustain", 1},
            "FM Op " + juce::String(op) + " Sustain",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"fmOp" + juce::String(op) + "Release", 1},
            "FM Op " + juce::String(op) + " Release",
            juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.4f), 0.3f));
     }

    // 8 Oscillator parameters (per oscillator: type, level, detune, waveform, wavePos, shapeType, shapeAmt, selfOsc, phaseDist)
    for (int osc = 1; osc <= 8; ++osc)
    {
        juce::String prefix = "osc" + juce::String(osc);
        const float defaultLevel = (osc <= 3) ? 1.0f : 0.0f;

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{prefix + "Type", 1}, prefix + " Type",
            juce::StringArray{"Classic", "Wavetable", "Additive", "Phase Dist", "Analog", "Digital"}, 0));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "Level", 1}, prefix + " Level",
            juce::NormalisableRange<float>(0.0f, 1.0f), defaultLevel));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "Detune", 1}, prefix + " Detune",
            juce::NormalisableRange<float>(-12.0f, 12.0f, 0.0f, 0.5f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{prefix + "Waveform", 1}, prefix + " Waveform",
            juce::StringArray{"Sine", "Saw", "Square", "Triangle", "Noise"}, 1));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "WavePos", 1}, prefix + " Wave Pos",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{prefix + "ShapeType", 1}, prefix + " Shape Type",
            juce::StringArray{"Off", "Drive", "Fold", "Clip"}, 0));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "ShapeAmt", 1}, prefix + " Shape Amount",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "SelfOsc", 1}, prefix + " Self Osc",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{prefix + "PhaseDist", 1}, prefix + " Phase Dist",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    }

    // Active oscillator count (1–8, default 3)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"oscCount", 1}, "Oscillator Count",
        juce::StringArray{"1", "2", "3", "4", "5", "6", "7", "8"}, 2));

    // Unison parameters
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"unisonVoices", 1}, "Unison Voices",
        juce::StringArray{"1", "2", "3", "4", "5", "6", "7", "8"}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"unisonDetune", 1}, "Unison Detune",
        juce::NormalisableRange<float>(0.0f, 100.0f), 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"unisonWidth", 1}, "Unison Width",
        juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"unisonSpreadMode", 1}, "Unison Spread",
        juce::StringArray{"Stacked", "Chord", "Random"}, 0));

    // Filter topology
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"filterType", 1}, "Filter Type",
        juce::StringArray{"LP", "HP", "BP", "Notch"}, 0));

    // Sub oscillator
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"subOscEnable", 1}, "Sub Osc", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"subOscLevel", 1}, "Sub Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"subOscWave", 1}, "Sub Wave",
        juce::StringArray{"Sine", "Square"}, 0));

    // Noise oscillator
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"noiseOscEnable", 1}, "Noise Osc", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noiseOscLevel", 1}, "Noise Level",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noiseOscColor", 1}, "Noise Color",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 0.0f, 0.3f), 5000.0f));

    // Voice mode / portamento
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"voiceMode", 1}, "Voice Mode",
        juce::StringArray{"Poly", "Mono", "Legato"}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"portamento", 1}, "Portamento",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.0f, 0.4f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"portaAlways", 1}, "Porta Always", false));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"mpeEnabled", 1}, "MPE", false));

    // Macro controls (8 DAW-automatable macro knobs, each 0-1)
    for (int m = 1; m <= 8; ++m)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"macro" + juce::String(m), 1},
            "Macro " + juce::String(m),
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    // Granular engine parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularPosition", 1}, "Gran Position",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularGrainSize", 1}, "Grain Size",
        juce::NormalisableRange<float>(0.01f, 0.5f, 0.0f, 0.4f), 0.08f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularSpray", 1}, "Spray",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularDensity", 1}, "Density",
        juce::NormalisableRange<float>(1.0f, 64.0f, 0.0f, 0.4f), 12.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularPitchScatter", 1}, "Pitch Scatter",
        juce::NormalisableRange<float>(0.0f, 24.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"granularEnvShape", 1}, "Env Shape",
        juce::StringArray{"Gaussian", "Hann", "Trapezoid", "Triangle"}, 1));
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"granularReverse", 1}, "Reverse", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularStereoSpread", 1}, "Stereo Spread",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularAttack", 1}, "Gran Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularDecay", 1}, "Gran Decay",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.0f, 0.4f), 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularSustain", 1}, "Gran Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"granularRelease", 1}, "Gran Release",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.0f, 0.4f), 0.5f));

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
        sampleRate = 44100.0;
    if (samplesPerBlock <= 0)
        samplesPerBlock = 512;

    synthEngine.prepare (sampleRate, samplesPerBlock);
    granularEngine.prepare (sampleRate, samplesPerBlock);
    samplerEngine.prepare (sampleRate, samplesPerBlock);
    layerManager.prepare (sampleRate, samplesPerBlock);
    drumSequencer.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate, drumSequencer.getBPM());
    proSequencer.prepare (sampleRate, 120.0f);
    arpeggiator.prepare  (sampleRate, 120.0f);
    patternEngine.prepare (sampleRate, 120.0f);
    delay.prepare (sampleRate, samplesPerBlock);
    delay.reset();
    reverb.prepare (sampleRate, samplesPerBlock);
    reverb.reset();
    for (int ch = 0; ch < 2; ++ch)
    {
        chorus[ch].prepare    (sampleRate, samplesPerBlock); chorus[ch].reset();
        distortion[ch].prepare(sampleRate, samplesPerBlock); distortion[ch].reset();
        eq[ch].prepare        (sampleRate, samplesPerBlock); eq[ch].reset();
        compressor[ch].prepare(sampleRate, samplesPerBlock); compressor[ch].reset();
    }
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
        return;

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
    const float baseReverbPreDelay  = *apvts.getRawParameterValue("reverbPreDelay");
    const float baseReverbLFDamp    = *apvts.getRawParameterValue("reverbLFDamp");
    const float baseReverbWidth     = *apvts.getRawParameterValue("reverbWidth");
    const bool  baseReverbFreeze    = *apvts.getRawParameterValue("reverbFreeze") > 0.5f;
    const float baseChorusRate  = *apvts.getRawParameterValue("chorusRate");
    const float baseChorusDepth = *apvts.getRawParameterValue("chorusDepth");
    const float baseChorusMix   = *apvts.getRawParameterValue("chorusMix");
    const float baseDistDrive   = *apvts.getRawParameterValue("distDrive");
    const float baseDistTone    = *apvts.getRawParameterValue("distTone");
    const float baseDistMix     = *apvts.getRawParameterValue("distMix");
    const float baseEQLow       = *apvts.getRawParameterValue("eqLowGain");
    const float baseEQMid       = *apvts.getRawParameterValue("eqMidGain");
    const float baseEQHigh      = *apvts.getRawParameterValue("eqHighGain");
    const float baseCompThresh  = *apvts.getRawParameterValue("compThreshold");
    const float baseCompRatio   = *apvts.getRawParameterValue("compRatio");
    const float baseCompAttack  = *apvts.getRawParameterValue("compAttack");
    const float baseCompRelease = *apvts.getRawParameterValue("compRelease");
    const float baseCompMakeup  = *apvts.getRawParameterValue("compMakeup");

    synthEngine.setEnvelopeParams(
        *apvts.getRawParameterValue("attack"),
        *apvts.getRawParameterValue("decay"),
        *apvts.getRawParameterValue("sustain"),
        *apvts.getRawParameterValue("release")
    );

    // Active oscillator count
    const int oscCount = static_cast<int>(*apvts.getRawParameterValue("oscCount")) + 1;
    synthEngine.setOscCount(oscCount);

    // 8 Oscillator parameters
    for (int osc = 0; osc < 8; ++osc)
    {
        juce::String prefix = "osc" + juce::String(osc + 1);
        const int typeChoice = static_cast<int>(*apvts.getRawParameterValue(prefix + "Type"));
        const OscillatorType otype = static_cast<OscillatorType>(typeChoice);
        synthEngine.setOscillatorType(osc, otype);
        synthEngine.setOscillatorLevel(osc, *apvts.getRawParameterValue(prefix + "Level"));
        synthEngine.setOscillatorDetune(osc, *apvts.getRawParameterValue(prefix + "Detune"));

        const int wfChoice = static_cast<int>(*apvts.getRawParameterValue(prefix + "Waveform"));
        synthEngine.setOscillatorWaveform(osc, static_cast<WaveformType>(wfChoice));
        synthEngine.setOscillatorWavePosition(osc, *apvts.getRawParameterValue(prefix + "WavePos"));

        const int shapeChoice = static_cast<int>(*apvts.getRawParameterValue(prefix + "ShapeType"));
        synthEngine.setOscillatorShapeType(osc, static_cast<OscShapeType>(shapeChoice));
        synthEngine.setOscillatorShapeAmount(osc, *apvts.getRawParameterValue(prefix + "ShapeAmt"));
        synthEngine.setOscillatorSelfOsc(osc, *apvts.getRawParameterValue(prefix + "SelfOsc"));
        synthEngine.setOscillatorPhaseDistAmount(osc, *apvts.getRawParameterValue(prefix + "PhaseDist"));
    }

    // Unison
    synthEngine.setUnisonVoices(static_cast<int>(*apvts.getRawParameterValue("unisonVoices")) + 1);
    synthEngine.setUnisonDetune(*apvts.getRawParameterValue("unisonDetune") / 100.0f);
    synthEngine.setUnisonWidth(*apvts.getRawParameterValue("unisonWidth"));
    synthEngine.setUnisonSpreadMode(static_cast<SynthEngine::UnisonSpreadMode>(
        static_cast<int>(*apvts.getRawParameterValue("unisonSpreadMode"))));

    // Filter type
    synthEngine.setFilterType(static_cast<Filter::FilterType>(
        static_cast<int>(*apvts.getRawParameterValue("filterType"))));

    // Sub oscillator
    synthEngine.setSubOscEnabled(*apvts.getRawParameterValue("subOscEnable") > 0.5f);
    synthEngine.setSubOscLevel(*apvts.getRawParameterValue("subOscLevel"));
    {
        const int subWaveChoice = static_cast<int>(*apvts.getRawParameterValue("subOscWave"));
        synthEngine.setSubOscWaveform(subWaveChoice == 1 ? WaveformType::Square : WaveformType::Sine);
    }

    // Noise oscillator
    synthEngine.setNoiseOscEnabled(*apvts.getRawParameterValue("noiseOscEnable") > 0.5f);
    synthEngine.setNoiseOscLevel(*apvts.getRawParameterValue("noiseOscLevel"));
    synthEngine.setNoiseOscColor(*apvts.getRawParameterValue("noiseOscColor"));

    // Voice mode / portamento
    synthEngine.setVoiceMode(static_cast<VoiceMode>(static_cast<int>(*apvts.getRawParameterValue("voiceMode"))));
    synthEngine.setPortamento(*apvts.getRawParameterValue("portamento"));
    synthEngine.setPortaAlways(*apvts.getRawParameterValue("portaAlways") > 0.5f);

    synthEngine.setFMAlgorithm(
        static_cast<int>(*apvts.getRawParameterValue("fmAlgorithm")) + 1
    );

    for (int op = 0; op < 4; ++op)
    {
        const int opIdx = op + 1;
        synthEngine.setFMOperatorParams(
            opIdx,
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Ratio"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Level"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Feedback"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Attack"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Decay"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Sustain"),
            *apvts.getRawParameterValue("fmOp" + juce::String(opIdx) + "Release")
        );
    }

    reverb.setDamping(baseReverbDamp);

    bool   dawPlaying = false;
    double dawBPM     = 120.0;
    double dawPpqPos  = -1.0;

    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            dawBPM    = pos->getBpm().orFallback(120.0);
            dawPlaying = pos->getIsPlaying();
            if (auto ppqOpt = pos->getPpqPosition())
                dawPpqPos = *ppqOpt;
        }
    }

    if (dawPlaying)
    {
        drumSequencer.setBPM (static_cast<float> (dawBPM));
        sequencer.setBPM     (static_cast<float> (dawBPM));
        proSequencer.setBPM  (static_cast<float> (dawBPM));
        arpeggiator.setBPM   (static_cast<float> (dawBPM));
        patternEngine.setBPM (static_cast<float> (dawBPM));

        if (!drumSequencer.isPlaying()) drumSequencer.start();
        if (!sequencer.isPlaying())     sequencer.start();
        if (!proSequencer.isPlaying())  proSequencer.start();
        if (!arpeggiator.isPlaying())   arpeggiator.start();
        if (!patternEngine.isPlaying()) patternEngine.start();

        if (dawPpqPos >= 0.0)
        {
            const double ppqStepPos = dawPpqPos * 4.0;
            drumSequencer.syncToDAWPosition (ppqStepPos);
            sequencer.syncToDAWPosition     (ppqStepPos);
            proSequencer.syncToDAWPosition  (ppqStepPos);
            arpeggiator.syncToDAWPosition   (ppqStepPos);
            patternEngine.syncToDAWPosition (ppqStepPos);
        }
    }
    else
    {
        if (sequencer.isPlaying() || proSequencer.isPlaying())
        {
            synthEngine.allNotesOff();
            samplerEngine.allNotesOff();
            granularEngine.allNotesOff();
            layerManager.allNotesOff();
            sequencer.stop();
            proSequencer.stop();
        }
        if (arpeggiator.isPlaying())
            arpeggiator.stop();
        if (patternEngine.isPlaying())
            patternEngine.stop();
        if (drumSequencer.isPlaying())
            drumSequencer.stop();
    }

    dawWasPlaying = dawPlaying;

    const int numSamples = buffer.getNumSamples();

    // Arpeggiator: capture keyboard NoteOn/Off before sequencer output is added,
    // then remove them from the main buffer so the synth doesn't double-trigger.
    juce::MidiBuffer arpMidi;
    juce::MidiBuffer patternMidi;

    if (arpeggiator.isEnabled())
    {
        for (const auto md : midiMessages)
        {
            auto msg = md.getMessage();
            if (msg.isNoteOn())  arpeggiator.noteOn  (msg.getNoteNumber());
            if (msg.isNoteOff()) arpeggiator.noteOff (msg.getNoteNumber());
        }

        // Rebuild midiMessages without keyboard NoteOn/Off (arp owns them)
        juce::MidiBuffer filtered;
        for (const auto md : midiMessages)
        {
            auto msg = md.getMessage();
            if (!msg.isNoteOn() && !msg.isNoteOff())
                filtered.addEvent (msg, md.samplePosition);
        }
        midiMessages.clear();
        midiMessages.addEvents (filtered, 0, numSamples, 0);
    }

    // Generate audio from drum sequencer
    drumSequencer.process (buffer, numSamples);

    // Which melodic sequencer is active?
    const int activeMelodicSeq = static_cast<int> (*apvts.getRawParameterValue("melodicSequencer"));

    // Process selected melodic sequencer; filter out keyboard notes from others
    if (activeMelodicSeq == 0)
    {
        sequencer.process (midiMessages, numSamples);
        {
            int numSteps = sequencer.getNumSteps();
            float normStep = numSteps > 1 ? static_cast<float> (sequencer.getCurrentStep()) / static_cast<float> (numSteps - 1) : 0.0f;
            modulationMatrix.setModulationValue (ModSourceType::SequencerStep, 0, normStep);
        }
        juce::MidiBuffer proMidi;
        proSequencer.process (proMidi, numSamples);
        midiMessages.addEvents (proMidi, 0, numSamples, 0);
    }
    else if (activeMelodicSeq == 1)
    {
        juce::MidiBuffer proMidi;
        proSequencer.process (proMidi, numSamples);
        midiMessages.addEvents (proMidi, 0, numSamples, 0);
    }
    else
    {
    }

    // Run arpeggiator or PatternEngine based on selection
    if (activeMelodicSeq == 1)
    {
        if (arpeggiator.isEnabled() && arpeggiator.isPlaying())
            arpeggiator.process (arpMidi, numSamples);
    }
    else if (activeMelodicSeq == 2)
    {
        if (patternEngine.isEnabled())
        {
            for (const auto md : midiMessages)
            {
                auto msg = md.getMessage();
                if (msg.isNoteOn())  patternEngine.noteOn  (msg.getNoteNumber());
                if (msg.isNoteOff()) patternEngine.noteOff (msg.getNoteNumber());
            }
        }
        if (patternEngine.isEnabled() && patternEngine.isPlaying())
            patternEngine.process (patternMidi, numSamples);
    }

    // Process all MIDI (keyboard input + sequencer output)
    const bool mpeOn = *apvts.getRawParameterValue("mpeEnabled") > 0.5f;
    synthEngine.setMPEEnabled(mpeOn);

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const int ch = message.getChannel();  // 1-based; ch 1 = MPE master

        // MIDI Learn / CC-to-parameter mapping
        if (midiLearnActive)
            handleMidiForLearn(message);
        else
            applyMidiMapping(message);

        if (message.isNoteOn())
        {
            int   note = message.getNoteNumber();
            float vel  = message.getVelocity() / 127.0f;
            sustainedNoteHeld[note] = false;
            if (sostenutoPedalDown)
                sostenutoNoteHeld[note] = true;
            modulationMatrix.setModulationValue(ModSourceType::Velocity,    0, vel);
            modulationMatrix.setModulationValue(ModSourceType::NoteNumber,  0, note / 127.0f);
            modulationMatrix.setModulationValue(ModSourceType::Random,      0, juce::Random::getSystemRandom().nextFloat());
            if (mpeOn && ch > 1)
                synthEngine.noteOnMPE(ch, note, vel);
            else
                synthEngine.noteOn(note, vel);
            samplerEngine.noteOn(note, vel);
            granularEngine.noteOn(note, vel);
            layerManager.noteOn(note, vel, ch);
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
                if (mpeOn && ch > 1)
                    synthEngine.noteOffMPE(ch, note);
                else
                    synthEngine.noteOff(note);
                samplerEngine.noteOff(note);
                granularEngine.noteOff(note);
                layerManager.noteOff(note);
            }
        }
        else if (message.isPitchWheel())
        {
            if (mpeOn && ch > 1)
            {
                // Member channel: per-note pitch bend (±48 semitones, standard MPE)
                const float semitones = ((message.getPitchWheelValue() - 8192) / 8192.0f)
                                        * SynthEngine::MPE_PITCH_BEND_RANGE;
                synthEngine.setMPEPitchBend(ch, semitones);
            }
            else
            {
                // Master channel / non-MPE: global pitch bend (±2 semitones)
                basePitchBend = ((message.getPitchWheelValue() - 8192) / 8192.0f) * 2.0f;
            }
        }
        else if (message.isChannelPressure())
        {
            if (mpeOn && ch > 1)
            {
                // Member channel pressure = per-note expression (Z-axis)
                const float pressure = message.getChannelPressureValue() / 127.0f;
                synthEngine.setMPEPressure(ch, pressure);
                modulationMatrix.setModulationValue(ModSourceType::MPEPressure, 0, pressure);
            }
        }
        else if (message.isController())
        {
            int cc  = message.getControllerNumber();
            int val = message.getControllerValue();

            if (mpeOn && ch > 1)
            {
                // Member channels: only CC74 (slide / Y-axis); other CCs ignored per MPE spec
                if (cc == 74)
                {
                    // Neutral value is 63, normalise to -1..+1
                    const float slide = (val - 63) / 63.0f;
                    synthEngine.setMPESlide(ch, slide);
                    modulationMatrix.setModulationValue(ModSourceType::MPESlide, 0, slide);
                }
            }
            else
            {
                // Master channel / non-MPE: existing global CC handling
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
                                synthEngine.noteOff(n);
                                samplerEngine.noteOff(n);
                                granularEngine.noteOff(n);
                                sustainedNoteHeld[n] = false;
                            }
                        }
                    }
                }
                else if (cc == 66)  // sostenuto pedal
                {
                    if (val >= 64)
                    {
                        sostenutoPedalDown = true;
                    }
                    else
                    {
                        sostenutoPedalDown = false;
                        for (int n = 0; n < 128; ++n)
                        {
                            if (sostenutoNoteHeld[n])
                            {
                                synthEngine.noteOff(n);
                                samplerEngine.noteOff(n);
                                sostenutoNoteHeld[n] = false;
                            }
                        }
                    }
                }
                else if (cc == 1)  // modulation wheel -> filter cutoff mod
                {
                    float modWheel = val / 127.0f;
                    baseFilterModAmount = modWheel * 10000.0f;
                }
                else if (cc == 123)  // all notes off
                {
                    synthEngine.allNotesOff();
                    samplerEngine.allNotesOff();
                    granularEngine.allNotesOff();
                    layerManager.allNotesOff();
                    for (int n = 0; n < 128; ++n)
                        sustainedNoteHeld[n] = false;
                    sustainPedalDown = false;
                }
            }
        }
    }

    // Route selected sequencer output to synth engines
    if (activeMelodicSeq == 1)
    {
        for (const auto md : arpMidi)
        {
            auto msg = md.getMessage();
            if (msg.isNoteOn())
            {
                synthEngine.noteOn  (msg.getNoteNumber(), msg.getVelocity() / 127.0f);
                samplerEngine.noteOn (msg.getNoteNumber(), msg.getVelocity() / 127.0f);
            }
            else if (msg.isNoteOff())
            {
                synthEngine.noteOff  (msg.getNoteNumber());
                samplerEngine.noteOff (msg.getNoteNumber());
            }
        }
    }
    else if (activeMelodicSeq == 2)
    {
        for (const auto md : patternMidi)
        {
            auto msg = md.getMessage();
            if (msg.isNoteOn())
            {
                synthEngine.noteOn  (msg.getNoteNumber(), msg.getVelocity() / 127.0f);
                samplerEngine.noteOn (msg.getNoteNumber(), msg.getVelocity() / 127.0f);
            }
            else if (msg.isNoteOff())
            {
                synthEngine.noteOff  (msg.getNoteNumber());
                samplerEngine.noteOff (msg.getNoteNumber());
            }
        }
    }

    const int numChannels = buffer.getNumChannels();

    // Read LFO base rates once per block
    const float lfoBaseRates[4] = {
        *apvts.getRawParameterValue("lfo1Rate"),
        *apvts.getRawParameterValue("lfo2Rate"),
        *apvts.getRawParameterValue("lfo3Rate"),
        *apvts.getRawParameterValue("lfo4Rate"),
    };

    // Push envelope follower into modulation matrix before computing sums
    modulationMatrix.setModulationValue(ModSourceType::EnvelopeFollower, 0, envFollowerLevel);

    // Per-block effect parameter modulation
    {
        float preSums[MAX_MOD_TARGETS] = {};
        modulationMatrix.computeModulationSums(preSums);
        const float effectMixMod = preSums[static_cast<int>(ModTargetType::EffectMix)];
        delay.setTime    (juce::jlimit(0.0f,  2.0f,  baseDelayTime     + preSums[static_cast<int>(ModTargetType::EffectParam1)]));
        delay.setFeedback(juce::jlimit(0.0f,  0.95f, baseDelayFeedback + preSums[static_cast<int>(ModTargetType::EffectParam2)]));
        delay.setMix     (juce::jlimit(0.0f,  1.0f,  baseDelayMix      + effectMixMod));
        reverb.setRoomSize(juce::jlimit(0.0f, 1.0f,  baseReverbRoom    + preSums[static_cast<int>(ModTargetType::EffectParam3)]));
        reverb.setWetLevel(juce::jlimit(0.0f, 1.0f,  baseReverbWet     + effectMixMod));
        reverb.setPreDelay(baseReverbPreDelay);
        reverb.setLFDamping(baseReverbLFDamp);
        reverb.setWidth(baseReverbWidth);
        reverb.setFreeze(baseReverbFreeze);

        // Apply parameters to the two stereo instances of each new effect
        for (int ch = 0; ch < 2; ++ch)
        {
            chorus[ch].setRate (baseChorusRate);
            chorus[ch].setDepth(baseChorusDepth);
            chorus[ch].setMix  (baseChorusMix);
            distortion[ch].setDrive(baseDistDrive);
            distortion[ch].setTone (baseDistTone);
            distortion[ch].setMix  (baseDistMix);
            eq[ch].setLowGain (baseEQLow);
            eq[ch].setMidGain (baseEQMid);
            eq[ch].setHighGain(baseEQHigh);
            compressor[ch].setThreshold(baseCompThresh);
            compressor[ch].setRatio    (baseCompRatio);
            compressor[ch].setAttack   (baseCompAttack);
            compressor[ch].setRelease  (baseCompRelease);
            compressor[ch].setMakeup   (baseCompMakeup);
        }
    }

    float modSums[MAX_MOD_TARGETS];
    modulationMatrix.computeModulationSums(modSums);

    float waveIdx = static_cast<float>(baseWaveform)
                  + modSums[static_cast<int>(ModTargetType::OscillatorWaveform)];
    synthEngine.setOscillatorWaveform(0, static_cast<WaveformType>(
        static_cast<int>(juce::jlimit(0.0f, 4.0f, waveIdx))));

    for (int lfo = 0; lfo < 4; ++lfo)
    {
        ModTargetType lfoRateTarget = static_cast<ModTargetType>(static_cast<int>(ModTargetType::LFO1Rate) + lfo);
        float newRate = lfoBaseRates[lfo] + modSums[static_cast<int>(lfoRateTarget)];
        modulationMatrix.setLFORate(lfo, juce::jlimit(0.01f, 100.0f, newRate));
    }

    float effCutoff = juce::jlimit(20.0f, 20000.0f,
        baseFilterCutoff + baseFilterModAmount + modSums[static_cast<int>(ModTargetType::FilterCutoff)]);
    float effRes = juce::jlimit(0.1f, 10.0f,
        baseFilterResonance + modSums[static_cast<int>(ModTargetType::FilterResonance)]);
    synthEngine.setFilterParams(effCutoff, effRes);

    float effVol = juce::jlimit(0.0f, 1.0f,
        masterVolume + modSums[static_cast<int>(ModTargetType::AmpVolume)]);
    float effSynthVol = juce::jlimit(0.0f, 1.0f,
        effVol + modSums[static_cast<int>(ModTargetType::OscillatorLevel)]);
    synthEngine.setMasterVolume(effSynthVol);
    samplerEngine.setMasterVolume(effVol);

    synthEngine.setPitchBend(basePitchBend + modSums[static_cast<int>(ModTargetType::OscillatorPitch)]);

    const float pan = juce::jlimit(-1.0f, 1.0f, modSums[static_cast<int>(ModTargetType::AmpPan)]);

    // Granular engine — push params and render (modulated targets use scaled modSums)
    granularEngine.setPosition(juce::jlimit(0.0f, 1.0f,
        *apvts.getRawParameterValue("granularPosition")
        + modSums[static_cast<int>(ModTargetType::GranularPosition)]));
    granularEngine.setGrainSize(juce::jlimit(0.01f, 0.5f,
        *apvts.getRawParameterValue("granularGrainSize")
        + modSums[static_cast<int>(ModTargetType::GranularGrainSize)] * 0.25f));
    granularEngine.setSpray(juce::jlimit(0.0f, 1.0f,
        *apvts.getRawParameterValue("granularSpray")
        + modSums[static_cast<int>(ModTargetType::GranularSpray)]));
    granularEngine.setDensity(juce::jlimit(1.0f, 64.0f,
        *apvts.getRawParameterValue("granularDensity")
        + modSums[static_cast<int>(ModTargetType::GranularDensity)] * 32.0f));
    granularEngine.setPitchScatter(juce::jlimit(0.0f, 24.0f,
        *apvts.getRawParameterValue("granularPitchScatter")
        + modSums[static_cast<int>(ModTargetType::GranularPitchScatter)] * 12.0f));

    // New mod targets: osc 0 shape amount + phase dist amount
    {
        const float baseShapeAmt = *apvts.getRawParameterValue("osc1ShapeAmt");
        synthEngine.setOscillatorShapeAmount(0, juce::jlimit(0.0f, 1.0f,
            baseShapeAmt + modSums[static_cast<int>(ModTargetType::OscShapeAmount)]));
        const float basePDamt = *apvts.getRawParameterValue("osc1PhaseDist");
        synthEngine.setOscillatorPhaseDistAmount(0, juce::jlimit(0.0f, 1.0f,
            basePDamt + modSums[static_cast<int>(ModTargetType::OscPhaseDistAmount)]));
    }

    granularEngine.setEnvShape     (static_cast<int>(*apvts.getRawParameterValue("granularEnvShape")));
    granularEngine.setReverse      (*apvts.getRawParameterValue("granularReverse") > 0.5f);
    granularEngine.setStereoSpread (*apvts.getRawParameterValue("granularStereoSpread"));
    granularEngine.setAttack       (*apvts.getRawParameterValue("granularAttack"));
    granularEngine.setDecay        (*apvts.getRawParameterValue("granularDecay"));
    granularEngine.setSustain      (*apvts.getRawParameterValue("granularSustain"));
    granularEngine.setRelease      (*apvts.getRawParameterValue("granularRelease"));
    granularEngine.setMasterVolume (effVol);

    juce::AudioBuffer<float> synthBuffer(2, numSamples);
    juce::AudioBuffer<float> samplerBuffer(2, numSamples);
    juce::AudioBuffer<float> granularBuffer(2, numSamples);
    juce::AudioBuffer<float> layerBuffer(2, numSamples);
    synthBuffer.clear();
    samplerBuffer.clear();
    granularBuffer.clear();
    layerBuffer.clear();
    synthEngine.processBuffer(synthBuffer, numSamples);
    samplerEngine.processBuffer(samplerBuffer, numSamples);
    granularEngine.processBuffer(granularBuffer, numSamples);
    layerManager.processBlock(layerBuffer, numSamples);

    {
        const float svol = *apvts.getRawParameterValue("samplerVolume");
        const float span = *apvts.getRawParameterValue("samplerPan");
        samplerBuffer.applyGain(0, 0, numSamples, svol * juce::jlimit(0.0f, 1.0f, 1.0f - span));
        if (samplerBuffer.getNumChannels() > 1)
            samplerBuffer.applyGain(1, 0, numSamples, svol * juce::jlimit(0.0f, 1.0f, 1.0f + span));
    }

    // Load effect chain order once per block (6 nibbles → slot 0..5 each holds an EffectID)
    const uint32_t chainPacked = effectChainOrder.load(std::memory_order_relaxed);
    int chain[6];
    for (int s = 0; s < 6; ++s)
        chain[s] = static_cast<int>((chainPacked >> (s * 4)) & 0xFu);

    // Find where Reverb sits so we can split the loop around it
    int reverbPos = 5;
    for (int s = 0; s < 6; ++s)
        if (chain[s] == static_cast<int>(EffectID::Reverb)) { reverbPos = s; break; }

    // Mix sources + apply all chain effects that come before Reverb
    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s = buffer.getReadPointer(ch)[i]
                    + synthBuffer   .getReadPointer(ch)[i]
                    + samplerBuffer .getReadPointer(ch)[i]
                    + granularBuffer.getReadPointer(ch)[i]
                    + layerBuffer   .getReadPointer(ch)[i];
            for (int slot = 0; slot < reverbPos; ++slot)
                s = applyChainEffect(chain[slot], s, ch);
            buffer.getWritePointer(ch)[i] = s;
        }
    }

    // Reverb: stereo block-level (pre-delay, LF damp, width, freeze all inside)
    if (numChannels >= 2)
        reverb.processBlock(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
    else if (numChannels == 1)
        for (int i = 0; i < numSamples; ++i)
            buffer.getWritePointer(0)[i] = reverb.process(buffer.getReadPointer(0)[i]);

    // Apply chain effects that come after Reverb
    if (reverbPos < 5)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float s = buffer.getReadPointer(ch)[i];
                for (int slot = reverbPos + 1; slot < 6; ++slot)
                    s = applyChainEffect(chain[slot], s, ch);
                buffer.getWritePointer(ch)[i] = s;
            }
        }
    }

    // Pan
    const float panGainL = 1.0f - juce::jmax(0.0f, pan);
    const float panGainR = 1.0f + juce::jmin(0.0f, pan);
    for (int i = 0; i < numSamples; ++i)
    {
        if (numChannels > 0) buffer.getWritePointer(0)[i] *= panGainL;
        if (numChannels > 1) buffer.getWritePointer(1)[i] *= panGainR;
    }

    // Push mono mix to display FIFO (no allocation — writes directly to pre-allocated buffer)
    {
        const float* L = buffer.getReadPointer(0);
        const float* R = numChannels > 1 ? buffer.getReadPointer(1) : L;
        pushDisplaySamples(L, R, numSamples);
    }

    modulationMatrix.advanceLFOs(numSamples);

    // Compute envelope follower from output for use as modulation source next block
    float blockPeak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            blockPeak = std::max(blockPeak, std::abs(data[i]));
    }
    const float envDecay = std::exp(-static_cast<float>(numSamples) / (static_cast<float>(getSampleRate()) * 0.3f));
    envFollowerLevel = (blockPeak > envFollowerLevel) ? blockPeak : envFollowerLevel * envDecay;
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

    root.appendChild(apvts.copyState(), nullptr);

    juce::ValueTree synthParams("SynthParams");
    synthParams.setProperty("synthMode",   static_cast<int>(synthEngine.getSynthMode()), nullptr);
    synthParams.setProperty("fmAlgorithm", synthEngine.getFMAlgorithm(),                 nullptr);
    // Save 8 oscillator settings
    synthParams.setProperty("oscCount", synthEngine.getOscCount(), nullptr);
    for (int osc = 0; osc < 8; ++osc)
    {
        juce::ValueTree oscNode("Osc");
        oscNode.setProperty("index",         osc, nullptr);
        oscNode.setProperty("type",          static_cast<int>(synthEngine.getOscillatorType(osc)), nullptr);
        oscNode.setProperty("level",         synthEngine.getOscillatorLevel(osc), nullptr);
        oscNode.setProperty("detune",        synthEngine.getOscillatorDetune(osc), nullptr);
        oscNode.setProperty("waveform",      static_cast<int>(synthEngine.getOscillatorWaveform(osc)), nullptr);
        oscNode.setProperty("wavePos",       synthEngine.getOscillatorWavePosition(osc), nullptr);
        oscNode.setProperty("wavetableFile", synthEngine.getWavetableFilePath(osc), nullptr);
        oscNode.setProperty("shapeType",     static_cast<int>(synthEngine.getOscillatorShapeType(osc)), nullptr);
        oscNode.setProperty("shapeAmt",      synthEngine.getOscillatorShapeAmount(osc), nullptr);
        oscNode.setProperty("selfOsc",       synthEngine.getOscillatorSelfOsc(osc), nullptr);
        oscNode.setProperty("phaseDist",     synthEngine.getOscillatorPhaseDistAmount(osc), nullptr);
        synthParams.appendChild(oscNode, nullptr);
    }
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

    juce::ValueTree reverbExtra("ReverbExtra");
    reverbExtra.setProperty("dry", reverb.getDryLevel(), nullptr);
    root.appendChild(reverbExtra, nullptr);

    root.appendChild(drumSequencer.getState(), nullptr);
    root.appendChild(modulationMatrix.getState(), nullptr);
    root.appendChild(sequencer.getState(), nullptr);
    root.appendChild(proSequencer.getState(), nullptr);
    root.appendChild(arpeggiator.getState(),  nullptr);
    root.appendChild(samplerEngine.getState(), nullptr);

    // Effect chain order
    juce::ValueTree chainState("EffectChain");
    chainState.setProperty("order", static_cast<int>(effectChainOrder.load(std::memory_order_relaxed)), nullptr);
    root.appendChild(chainState, nullptr);

    // Macro manager state
    root.appendChild(macroManager.getState(), nullptr);

    // Granular engine state
    root.appendChild(granularEngine.getState(), nullptr);

    // Layer manager state
    root.appendChild(layerManager.getState(), nullptr);

    // Add MIDI mappings to state
    updateMidiMappingsInState(root);

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

    {
        auto apvtsState = apvts.state;
        auto savedState = root.getChildWithName("APVTSState");
        const int numParams = apvtsState.getNumChildren();
        const int numSaved  = savedState.getNumChildren();
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

    auto synthParams = root.getChildWithName("SynthParams");
    if (synthParams.isValid())
    {
        if (synthParams.hasProperty("synthMode"))
            synthEngine.setSynthMode(static_cast<SynthMode>((int)synthParams.getProperty("synthMode")));
        if (synthParams.hasProperty("fmAlgorithm"))
            synthEngine.setFMAlgorithm((int)synthParams.getProperty("fmAlgorithm"));

        // Restore oscillator count
        if (synthParams.hasProperty("oscCount"))
            synthEngine.setOscCount((int)synthParams.getProperty("oscCount"));

        // Restore 8 oscillator settings
        for (auto oscNode : synthParams)
        {
            if (oscNode.hasType("Osc"))
            {
                int idx = (int)oscNode.getProperty("index");
                if (idx >= 0 && idx < 8)
                {
                    synthEngine.setOscillatorType(idx, static_cast<OscillatorType>((int)oscNode.getProperty("type")));
                    synthEngine.setOscillatorLevel(idx, (float)oscNode.getProperty("level"));
                    synthEngine.setOscillatorDetune(idx, (float)oscNode.getProperty("detune"));
                    synthEngine.setOscillatorWaveform(idx, static_cast<WaveformType>((int)oscNode.getProperty("waveform")));
                    synthEngine.setOscillatorWavePosition(idx, (float)oscNode.getProperty("wavePos"));
                    if (oscNode.hasProperty("wavetableFile"))
                    {
                        juce::String path = oscNode.getProperty("wavetableFile").toString();
                        if (path.isNotEmpty())
                            synthEngine.loadWavetableFile(idx, juce::File(path));
                    }
                    if (oscNode.hasProperty("shapeType"))
                        synthEngine.setOscillatorShapeType(idx, static_cast<OscShapeType>((int)oscNode.getProperty("shapeType")));
                    if (oscNode.hasProperty("shapeAmt"))
                        synthEngine.setOscillatorShapeAmount(idx, (float)oscNode.getProperty("shapeAmt"));
                    if (oscNode.hasProperty("selfOsc"))
                        synthEngine.setOscillatorSelfOsc(idx, (float)oscNode.getProperty("selfOsc"));
                    if (oscNode.hasProperty("phaseDist"))
                        synthEngine.setOscillatorPhaseDistAmount(idx, (float)oscNode.getProperty("phaseDist"));
                }
            }
        }

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

    auto reverbExtra = root.getChildWithName("ReverbExtra");
    if (reverbExtra.isValid() && reverbExtra.hasProperty("dry"))
        reverb.setDryLevel((float)reverbExtra.getProperty("dry"));

    auto drumNode = root.getChildWithName("DrumSequencer");
    if (drumNode.isValid())
        drumSequencer.setState(drumNode);

    auto mmNode = root.getChildWithName("ModulationMatrix");
    if (mmNode.isValid())
        modulationMatrix.setState(mmNode);

    auto seqNode = root.getChildWithName("Sequencer");
    if (seqNode.isValid())
        sequencer.setState(seqNode);

    auto proSeqNode = root.getChildWithName("ProSequencer");
    if (proSeqNode.isValid())
        proSequencer.setState(proSeqNode);

    auto arpNode = root.getChildWithName("Arpeggiator");
    if (arpNode.isValid())
        arpeggiator.setState(arpNode);

    auto samplerNode = root.getChildWithName("SamplerEngine");
    if (samplerNode.isValid())
        samplerEngine.setState(samplerNode);

    // Effect chain order
    auto chainNode = root.getChildWithName("EffectChain");
    if (chainNode.isValid() && chainNode.hasProperty("order"))
        effectChainOrder.store(static_cast<uint32_t>(static_cast<int>(chainNode.getProperty("order"))),
                               std::memory_order_relaxed);

    // Load MIDI mappings from state
    loadMidiMappingsFromState(root);

    // Load macro manager state
    macroManager.setState(root.getChildWithName("MacroManager"));

    // Load granular engine state
    granularEngine.setState(root.getChildWithName("GranularEngine"));

    // Load layer manager state
    layerManager.setState(root.getChildWithName("LayerManager"));

    undoManager.clearUndoHistory();
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
// Effect chain helpers

void PluginProcessor::swapChainSlots(int a, int b) noexcept
{
    if (a == b || a < 0 || b < 0 || a >= 6 || b >= 6) return;
    uint32_t old = effectChainOrder.load(std::memory_order_relaxed);
    const int va = (old >> (a * 4)) & 0xF;
    const int vb = (old >> (b * 4)) & 0xF;
    uint32_t updated = old;
    updated &= ~(0xFu << (a * 4));
    updated &= ~(0xFu << (b * 4));
    updated |= static_cast<uint32_t>(vb) << (a * 4);
    updated |= static_cast<uint32_t>(va) << (b * 4);
    effectChainOrder.store(updated, std::memory_order_relaxed);
}

float PluginProcessor::applyChainEffect(int effectID, float sample, int ch)
{
    const int c = juce::jmin(ch, 1);
    switch (effectID)
    {
        case static_cast<int>(EffectID::Chorus):     return chorus[c].process(sample);
        case static_cast<int>(EffectID::Distortion): return distortion[c].process(sample);
        case static_cast<int>(EffectID::EQ):         return eq[c].process(sample);
        case static_cast<int>(EffectID::Compressor): return compressor[c].process(sample);
        case static_cast<int>(EffectID::Delay):      return delay.process(sample);
        default:                                     return sample; // Reverb handled separately
    }
}

//==============================================================================
// Display FIFO

void PluginProcessor::pushDisplaySamples(const float* L, const float* R, int n) noexcept
{
    int start1, size1, start2, size2;
    const int toWrite = juce::jmin(n, DISPLAY_FIFO_SIZE);
    displayFifo.prepareToWrite(toWrite, start1, size1, start2, size2);
    for (int i = 0; i < size1; ++i)
        displayFifoBuffer[start1 + i] = (L[i] + R[i]) * 0.5f;
    for (int i = 0; i < size2; ++i)
        displayFifoBuffer[start2 + i] = (L[size1 + i] + R[size1 + i]) * 0.5f;
    displayFifo.finishedWrite(size1 + size2);
}

int PluginProcessor::pullDisplaySamples(float* dest, int maxSamples) noexcept
{
    const int avail  = displayFifo.getNumReady();
    const int toRead = juce::jmin(avail, maxSamples);
    if (toRead <= 0) return 0;
    int start1, size1, start2, size2;
    displayFifo.prepareToRead(toRead, start1, size1, start2, size2);
    if (size1 > 0) juce::FloatVectorOperations::copy(dest,         displayFifoBuffer + start1, size1);
    if (size2 > 0) juce::FloatVectorOperations::copy(dest + size1, displayFifoBuffer + start2, size2);
    displayFifo.finishedRead(size1 + size2);
    return size1 + size2;
}

//==============================================================================
// MIDI Learn Implementation
void PluginProcessor::startMidiLearnForParameter(int parameterIndex)
{
    midiLearnActive = true;
    learnParameterIndex = parameterIndex;
}

void PluginProcessor::stopMidiLearn()
{
    midiLearnActive = false;
    learnParameterIndex = -1;
}

void PluginProcessor::handleMidiForLearn(const juce::MidiMessage& message)
{
    if (!midiLearnActive || learnParameterIndex < 0)
        return;

    juce::String paramID = getParameterIDFromIndex(learnParameterIndex);
    if (paramID.isEmpty())
        return;

    MidiMapping newMapping;
    newMapping.paramID = paramID;
    newMapping.midiChannel = message.getChannel(); // 1-16; 0 = omni

    if (message.isController())
    {
        newMapping.type = MidiMapping::CC;
        newMapping.controllerNumber = message.getControllerNumber();
    }
    else if (message.isPitchWheel())
    {
        newMapping.type = MidiMapping::PitchWheel;
    }
    else if (message.isChannelPressure())
    {
        newMapping.type = MidiMapping::ChannelPressure;
    }
    else
    {
        return; // ignore notes, clock, sysex, etc. — don't cancel the learn
    }

    // Replace existing mapping for this param, or add new
    for (auto& m : midiMappings)
    {
        if (m.paramID == paramID)
        {
            m = newMapping;
            stopMidiLearn();
            return;
        }
    }
    midiMappings.push_back(newMapping);
    stopMidiLearn();
}

void PluginProcessor::applyMidiMapping(const juce::MidiMessage& message)
{
    if (midiLearnActive || midiMappings.empty())
        return;

    const int msgChannel = message.getChannel(); // 1-16

    auto matchesChannel = [&](const MidiMapping& m) {
        return m.midiChannel == 0 || m.midiChannel == msgChannel;
    };

    if (message.isController())
    {
        const int cc = message.getControllerNumber();
        const float value = message.getControllerValue() / 127.0f;
        for (auto& m : midiMappings)
        {
            if (m.type == MidiMapping::CC && m.controllerNumber == cc && matchesChannel(m))
                if (auto* param = apvts.getParameter(m.paramID))
                    param->setValueNotifyingHost(value);
        }
    }
    else if (message.isPitchWheel())
    {
        // Normalise pitch-bend (-8192..8191) to 0..1
        const float value = (message.getPitchWheelValue() + 8192) / 16383.0f;
        for (auto& m : midiMappings)
        {
            if (m.type == MidiMapping::PitchWheel && matchesChannel(m))
                if (auto* param = apvts.getParameter(m.paramID))
                    param->setValueNotifyingHost(value);
        }
    }
    else if (message.isChannelPressure())
    {
        const float value = message.getChannelPressureValue() / 127.0f;
        for (auto& m : midiMappings)
        {
            if (m.type == MidiMapping::ChannelPressure && matchesChannel(m))
                if (auto* param = apvts.getParameter(m.paramID))
                    param->setValueNotifyingHost(value);
        }
    }
}

juce::String PluginProcessor::getParameterIDFromIndex(int index) const
{
    auto& params = getParameters();
    if (index < 0 || index >= params.size())
        return {};
    if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(params[index]))
        return rp->getParameterID();
    return {};
}

int PluginProcessor::getParameterIndexFromID(const juce::String& paramID) const
{
    auto& params = getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(params[i]))
            if (rp->getParameterID() == paramID)
                return i;
    }
    return -1;
}

void PluginProcessor::updateMidiMappingsInState(juce::ValueTree& stateTree) const
{
    for (int i = stateTree.getNumChildren() - 1; i >= 0; --i)
        if (stateTree.getChild(i).hasType("MIDIMappings"))
            stateTree.removeChild(i, nullptr);

    if (midiMappings.empty())
        return;

    juce::ValueTree midiMappingsTree("MIDIMappings");
    for (const auto& m : midiMappings)
    {
        juce::ValueTree mappingTree("Mapping");
        mappingTree.setProperty("paramID",           m.paramID,           nullptr);
        mappingTree.setProperty("type",              (int)m.type,         nullptr);
        mappingTree.setProperty("controllerNumber",  m.controllerNumber,  nullptr);
        mappingTree.setProperty("midiChannel",       m.midiChannel,       nullptr);
        midiMappingsTree.appendChild(mappingTree, nullptr);
    }
    stateTree.appendChild(midiMappingsTree, nullptr);
}

void PluginProcessor::loadMidiMappingsFromState(const juce::ValueTree& stateTree)
{
    midiMappings.clear();

    juce::ValueTree midiMappingsTree = stateTree.getChildWithName("MIDIMappings");
    if (!midiMappingsTree.isValid())
        return;

    for (const auto& mappingTree : midiMappingsTree)
    {
        if (!mappingTree.hasType("Mapping"))
            continue;

        MidiMapping m;
        m.paramID          = mappingTree.getProperty("paramID", juce::String()).toString();
        m.type             = (MidiMapping::Type)(int)mappingTree.getProperty("type", (int)MidiMapping::None);
        m.controllerNumber = (int)mappingTree.getProperty("controllerNumber", 0);
        m.midiChannel      = (int)mappingTree.getProperty("midiChannel", 0);

        if (!m.paramID.isEmpty() && getParameterIndexFromID(m.paramID) >= 0)
            midiMappings.push_back(m);
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
