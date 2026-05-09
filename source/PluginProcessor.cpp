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
        juce::ParameterID{"auxSendDelay", 1}, "Aux Send to Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"auxSendReverb", 1}, "Aux Send to Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

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

    for (int i = 1; i <= 8; ++i)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"lfo" + juce::String(i) + "Rate", 1},
            "LFO " + juce::String(i) + " Rate",
            juce::NormalisableRange<float>(0.01f, 20.0f, 0.0f, 0.3f), 1.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"lfo" + juce::String(i) + "Shape", 1},
            "LFO " + juce::String(i) + " Shape",
            juce::StringArray{"Sine", "Triangle", "Saw", "Square", "S&H", "Custom"}, 0));
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"lfo" + juce::String(i) + "Sync", 1},
            "LFO " + juce::String(i) + " Sync", false));
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID{"lfo" + juce::String(i) + "SyncDiv", 1},
            "LFO " + juce::String(i) + " Sync Div",
            juce::StringArray{"1/32", "1/16", "1/8", "1/4", "1/2", "1/1", "2/1", "4/1"}, 3));
    }

    for (int e = 2; e <= 3; ++e)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"modEnv" + juce::String(e) + "Attack", 1},
            "Mod Env " + juce::String(e) + " Attack",
            juce::NormalisableRange<float>(0.001f, 4.0f, 0.0f, 0.3f), 0.01f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"modEnv" + juce::String(e) + "Decay", 1},
            "Mod Env " + juce::String(e) + " Decay",
            juce::NormalisableRange<float>(0.001f, 4.0f, 0.0f, 0.3f), 0.1f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"modEnv" + juce::String(e) + "Sustain", 1},
            "Mod Env " + juce::String(e) + " Sustain",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"modEnv" + juce::String(e) + "Release", 1},
            "Mod Env " + juce::String(e) + " Release",
            juce::NormalisableRange<float>(0.001f, 8.0f, 0.0f, 0.3f), 0.2f));
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"fmAlgorithm", 1}, "FM Algorithm",
        juce::StringArray{"1", "2", "3", "4", "5", "6", "7", "8"}, 0));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"maxVoices", 1}, "Max Voices",
        juce::StringArray{"1", "2", "4", "6", "8", "10", "12", "16"}, 7));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"metronomeEnabled", 1}, "Metronome", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"metronomeVolume", 1}, "Metronome Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

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

    // Active oscillator count (1–4, default 3)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"oscCount", 1}, "Oscillator Count",
        juce::StringArray{"1", "2", "3", "4"}, 2));

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

    // Chord/Strum mode
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"chordModeEnabled", 1}, "Chord Mode", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"chordShape", 1}, "Chord Shape",
        juce::StringArray{"Root Only", "Major", "Minor", "Maj7", "Min7", "Dom7",
                          "Dim", "Aug", "Sus2", "Sus4", "Power", "Octave"}, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"chordStrumDelay", 1}, "Strum Delay",
        juce::NormalisableRange<float>(0.0f, 200.0f), 0.0f));

    // Global quality (oversampling)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"globalQuality", 1}, "Global Quality",
        juce::StringArray{"Off", "2x High", "4x Ultra"}, 0));

    // FX Mode (audio input passthrough)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"fxModeEnabled", 1}, "FX Mode", false));

    // Quick FX — Filter Modifier
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"filterModEnabled", 1}, "Filter Mod Enable", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filterModCutoff", 1}, "Filter Mod Cutoff",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filterModResonance", 1}, "Filter Mod Resonance",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filterModEnvDepth", 1}, "Filter Mod Env Depth",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    // Quick FX — Amp Modifier
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"ampModEnabled", 1}, "Amp Mod Enable", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ampModVolume", 1}, "Amp Mod Volume",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ampModPan", 1}, "Amp Mod Pan",
        juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    // Quick FX — Main Filter (post-effects global filter)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"mainFilterEnabled", 1}, "Main Filter Enable", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"mainFilterType", 1}, "Main Filter Type",
        juce::StringArray{"LP", "HP", "BP", "Notch"}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mainFilterCutoff", 1}, "Main Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mainFilterResonance", 1}, "Main Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.0f, 0.5f), 0.707f));

    return layout;
}

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                      .withInput  ("Audio In", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Output",  juce::AudioChannelSet::stereo(), true)
                      // Individual layer buses (1-8)
                      .withOutput ("Layer 1", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 2", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 3", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 4", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 5", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 6", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 7", juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Layer 8", juce::AudioChannelSet::stereo(), false)
                      // Individual drum track buses (9-16)
                      .withOutput ("Drum 1",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 2",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 3",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 4",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 5",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 6",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 7",  juce::AudioChannelSet::stereo(), false)
                      .withOutput ("Drum 8",  juce::AudioChannelSet::stereo(), false)),
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

    // Engines always run at native sample rate
    synthEngine.prepare (sampleRate, samplesPerBlock);
    granularEngine.prepare (sampleRate, samplesPerBlock);
    samplerEngine.prepare (sampleRate, samplesPerBlock);
    layerManager.prepare (sampleRate, samplesPerBlock);
    drumSequencer.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate, drumSequencer.getBPM());
    proSequencer.prepare (sampleRate, 120.0f);
    arpeggiator.prepare  (sampleRate, 120.0f);
    patternEngine.prepare (sampleRate, 120.0f);
    modulationMatrix.prepare (sampleRate, samplesPerBlock);
    modEnv2.setSampleRate(sampleRate);
    modEnv3.setSampleRate(sampleRate);

    // Main filter (QuickFX strip)
    mainFilter.setSampleRate(sampleRate);
    mainFilter.setFilterType(Filter::FilterType::LP);
    mainFilter.setCutoff(20000.0f);
    mainFilter.setResonance(0.707f);
    mainFilter.reset();

    // Global quality: set up oversampler and prepare effects at oversampled rate
    const int qualIdx = juce::jlimit(0, 2,
        (int)*apvts.getRawParameterValue("globalQuality"));
    oversamplingFactor = (qualIdx == 0) ? 1 : (qualIdx == 1) ? 2 : 4;

    if (qualIdx > 0)
    {
        activeOversampler = std::make_unique<juce::dsp::Oversampling<float>>(
            2, qualIdx,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
        activeOversampler->initProcessing ((size_t)samplesPerBlock);
        setLatencySamples ((int)std::round (activeOversampler->getLatencyInSamples()));
    }
    else
    {
        activeOversampler.reset();
        setLatencySamples (0);
    }

    // Effects chain prepared at oversampled rate
    const double osSR    = sampleRate * oversamplingFactor;
    const int    osBlock = samplesPerBlock * oversamplingFactor;

    for (int ch = 0; ch < 2; ++ch)
    {
        chorus[ch].prepare    (osSR, osBlock); chorus[ch].reset();
        distortion[ch].prepare(osSR, osBlock); distortion[ch].reset();
        eq[ch].prepare        (osSR, osBlock); eq[ch].reset();
        compressor[ch].prepare(osSR, osBlock); compressor[ch].reset();
    }
    delay.prepare (osSR, osBlock);  delay.reset();
    reverb.prepare(osSR, osBlock);  reverb.reset();

    // Aux sends always at native rate (they process the pre-effects dry signal)
    auxDelay.prepare (sampleRate, samplesPerBlock); auxDelay.reset();
    auxReverb.prepare(sampleRate, samplesPerBlock); auxReverb.reset();
    auxSendBuffer.setSize (2, samplesPerBlock, false, true, false);
    auxWorkBuffer.setSize (2, samplesPerBlock, false, true, false);
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
    // Main input bus must be stereo or disabled
    if (layouts.inputBuses.size() > 0)
    {
        const auto& inBus = layouts.inputBuses.getReference (0);
        if (!inBus.isDisabled() && inBus != juce::AudioChannelSet::stereo())
            return false;
    }

    // Main output bus must be stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // All additional output buses must be stereo or disabled
    for (int i = 1; i < layouts.outputBuses.size(); ++i)
    {
        const auto& ch = layouts.outputBuses.getReference (i);
        if (ch != juce::AudioChannelSet::stereo() && !ch.isDisabled())
            return false;
    }

    return true;
  #endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumSamples() <= 0 || buffer.getNumChannels() <= 0)
        return;

    // FX Mode: preserve input audio on channels 0-1; clear everything else.
    // Synth mode: clear all channels (normal instrument behavior).
    const bool fxMode = *apvts.getRawParameterValue("fxModeEnabled") > 0.5f;
    if (fxMode && getTotalNumInputChannels() >= 2)
    {
        for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
            buffer.clear (ch, 0, buffer.getNumSamples());
    }
    else
    {
        buffer.clear();
    }

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // Merge UI-generated MIDI (pitch bend, mod wheel from on-screen controls)
    {
        juce::ScopedLock sl(uiMidiLock);
        midiMessages.addEvents(uiMidiBuffer, 0, buffer.getNumSamples(), 0);
        uiMidiBuffer.clear();
    }

    // Preview note handling (message thread writes atomics, audio thread acts)
    {
        int prevNote = previewPrevNote.exchange(-1, std::memory_order_relaxed);
        if (prevNote >= 0)
        {
            synthEngine.noteOff(prevNote);
            samplerEngine.noteOff(prevNote);
            granularEngine.noteOff(prevNote);
            layerManager.noteOff(prevNote);
        }

        bool fireOn = previewNoteOn.exchange(false, std::memory_order_relaxed);
        if (fireOn)
        {
            int note = previewNote.load(std::memory_order_relaxed);
            if (note >= 0)
            {
                float vel = PREVIEW_VELOCITY / 127.0f;
                synthEngine.noteOn(note, vel);
                samplerEngine.noteOn(note, vel);
                granularEngine.noteOn(note, vel);
                layerManager.noteOn(note, vel, 1);
            }
        }

        int samplesLeft = previewSamplesLeft.load(std::memory_order_relaxed);
        int note = previewNote.load(std::memory_order_relaxed);
        if (note >= 0 && samplesLeft > 0)
        {
            samplesLeft -= buffer.getNumSamples();
            if (samplesLeft <= 0)
            {
                synthEngine.noteOff(note);
                samplerEngine.noteOff(note);
                granularEngine.noteOff(note);
                layerManager.noteOff(note);
                previewNote.store(-1, std::memory_order_relaxed);
                previewSamplesLeft.store(0, std::memory_order_relaxed);
            }
            else
            {
                previewSamplesLeft.store(samplesLeft, std::memory_order_relaxed);
            }
        }
    }

    // Chord/Strum: fire pending notes that are due this block
    {
        const bool chordOn = *apvts.getRawParameterValue("chordModeEnabled") > 0.5f;
        if (chordOn)
        {
            const int numSamplesLocal = buffer.getNumSamples();
            for (int i = 0; i < MAX_PENDING_NOTES; ++i)
            {
                auto& pn = pendingNotes[i];
                if (!pn.active) continue;
                pn.samplesRemaining -= numSamplesLocal;
                if (pn.samplesRemaining <= 0)
                {
                    const int   n = pn.note;
                    const float v = pn.velocity;
                    synthEngine.noteOn(n, v);
                    samplerEngine.noteOn(n, v);
                    granularEngine.noteOn(n, v);
                    layerManager.noteOn(n, v, 1);
                    // Mark note as fired in the owning ActiveChord
                    for (int ci = 0; ci < MAX_ACTIVE_CHORDS; ++ci)
                    {
                        auto& ac = activeChords[ci];
                        if (!ac.active || ac.rootNote != pn.rootNote) continue;
                        for (int ni = 0; ni < ac.noteCount; ++ni)
                        {
                            if (ac.chordNotes[ni] == n)
                                ac.noteFired[ni] = true;
                        }
                    }
                    pn.active = false;
                }
            }
        }
    }

    // CPU voice limiting — apply once per block before notes are processed
    {
        static const int voiceLimitValues[] = {1, 2, 4, 6, 8, 10, 12, 16};
        int limitIdx = juce::jlimit(0, 7, (int)*apvts.getRawParameterValue("maxVoices"));
        synthEngine.setVoiceLimit(voiceLimitValues[limitIdx]);
    }

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

    // Metronome beat detection
    {
        const bool metEnabled = *apvts.getRawParameterValue("metronomeEnabled") > 0.5f;
        if (metEnabled && dawPlaying && dawPpqPos >= 0.0 && prevDawPpqPos >= 0.0)
        {
            const double prevBeat = std::floor(prevDawPpqPos);
            const double curBeat  = std::floor(dawPpqPos);
            if (curBeat > prevBeat)
            {
                metClickDuration     = juce::roundToInt(0.025 * getSampleRate()); // 25ms
                metClickSamplesLeft  = metClickDuration;
                metClickSamplePos    = 0;
                // Downbeat = every 4 beats
                metClickIsDownbeat   = (std::fmod(dawPpqPos, 4.0) < 1.0);
            }
        }
        prevDawPpqPos = dawPpqPos;
    }

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

    // Generate audio from drum sequencer into a separate mix buffer
    juce::AudioBuffer<float> drumMainBuf (2, numSamples);
    drumMainBuf.clear();
    drumSequencer.process (drumMainBuf, numSamples);
    // Add drum main mix (bus-0 tracks) into the main output
    for (int ch = 0; ch < 2 && ch < buffer.getNumChannels(); ++ch)
        buffer.addFrom (ch, 0, drumMainBuf, ch, 0, numSamples);

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
            modEnv2.noteOn();
            modEnv3.noteOn();

            // Chord/Strum: schedule additional chord tones
            {
                const bool chordOn = *apvts.getRawParameterValue("chordModeEnabled") > 0.5f;
                if (chordOn && !mpeOn)
                {
                    static const int chordIntervals[12][8] = {
                        { 0, -1, -1, -1, -1, -1, -1, -1 },  // Root Only
                        { 0,  4,  7, -1, -1, -1, -1, -1 },  // Major
                        { 0,  3,  7, -1, -1, -1, -1, -1 },  // Minor
                        { 0,  4,  7, 11, -1, -1, -1, -1 },  // Maj7
                        { 0,  3,  7, 10, -1, -1, -1, -1 },  // Min7
                        { 0,  4,  7, 10, -1, -1, -1, -1 },  // Dom7
                        { 0,  3,  6, -1, -1, -1, -1, -1 },  // Dim
                        { 0,  4,  8, -1, -1, -1, -1, -1 },  // Aug
                        { 0,  2,  7, -1, -1, -1, -1, -1 },  // Sus2
                        { 0,  5,  7, -1, -1, -1, -1, -1 },  // Sus4
                        { 0,  7, 12, -1, -1, -1, -1, -1 },  // Power
                        { 0, 12, -1, -1, -1, -1, -1, -1 },  // Octave
                    };
                    const int shapeIdx = juce::jlimit(0, 11, (int)*apvts.getRawParameterValue("chordShape"));
                    const float strumMs = *apvts.getRawParameterValue("chordStrumDelay");
                    const float strumSamples = strumMs * 0.001f * (float)getSampleRate();

                    // Find a free ActiveChord slot
                    int ci = -1;
                    for (int k = 0; k < MAX_ACTIVE_CHORDS; ++k)
                        if (!activeChords[k].active) { ci = k; break; }

                    if (ci >= 0)
                    {
                        auto& ac = activeChords[ci];
                        ac.rootNote  = note;
                        ac.noteCount = 0;
                        ac.active    = true;

                        for (int ni = 0; ni < 8 && chordIntervals[shapeIdx][ni] >= 0; ++ni)
                        {
                            const int chordNote = juce::jlimit(0, 127, note + chordIntervals[shapeIdx][ni]);
                            ac.chordNotes[ac.noteCount] = chordNote;
                            if (ni == 0)
                            {
                                // Root already fired via normal path above
                                ac.noteFired[ac.noteCount] = true;
                            }
                            else
                            {
                                ac.noteFired[ac.noteCount] = false;
                                // Queue chord tone with strum delay
                                for (int pi = 0; pi < MAX_PENDING_NOTES; ++pi)
                                {
                                    if (!pendingNotes[pi].active)
                                    {
                                        pendingNotes[pi].note             = chordNote;
                                        pendingNotes[pi].velocity         = vel;
                                        pendingNotes[pi].rootNote         = note;
                                        pendingNotes[pi].samplesRemaining = juce::roundToInt((float)ni * strumSamples);
                                        pendingNotes[pi].active           = true;
                                        break;
                                    }
                                }
                            }
                            ac.noteCount++;
                        }
                    }
                }
            }
        }
        else if (message.isNoteOff())
        {
            int note = message.getNoteNumber();

            // Chord/Strum: cancel pending tones and release fired chord tones
            {
                const bool chordOn = *apvts.getRawParameterValue("chordModeEnabled") > 0.5f;
                if (chordOn && !mpeOn)
                {
                    // Cancel any still-pending strum notes for this root
                    for (int i = 0; i < MAX_PENDING_NOTES; ++i)
                        if (pendingNotes[i].active && pendingNotes[i].rootNote == note)
                            pendingNotes[i].active = false;

                    // Fire noteOff for chord tones that have already been triggered (skip index 0 = root, handled below)
                    for (int ci = 0; ci < MAX_ACTIVE_CHORDS; ++ci)
                    {
                        auto& ac = activeChords[ci];
                        if (!ac.active || ac.rootNote != note) continue;
                        for (int ni = 1; ni < ac.noteCount; ++ni)
                        {
                            if (ac.noteFired[ni])
                            {
                                const int cn = ac.chordNotes[ni];
                                synthEngine.noteOff(cn);
                                samplerEngine.noteOff(cn);
                                granularEngine.noteOff(cn);
                                layerManager.noteOff(cn);
                            }
                        }
                        ac.active = false;
                        break;
                    }
                }
            }

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
                modEnv2.noteOff();
                modEnv3.noteOff();
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

    // LFO rates (with optional DAW sync override per LFO)
    static const float lfoSyncDivisors[8] = { 1.0f/32, 1.0f/16, 1.0f/8, 1.0f/4, 1.0f/2, 1.0f, 2.0f, 4.0f };
    float lfoBaseRates[8];
    for (int i = 0; i < 8; ++i)
    {
        const juce::String idx = juce::String(i + 1);
        bool synced = *apvts.getRawParameterValue("lfo" + idx + "Sync") > 0.5f;
        if (synced)
        {
            int divIdx = juce::jlimit(0, 7, (int)*apvts.getRawParameterValue("lfo" + idx + "SyncDiv"));
            lfoBaseRates[i] = (float)(dawBPM / 60.0) / lfoSyncDivisors[divIdx];
        }
        else
        {
            lfoBaseRates[i] = *apvts.getRawParameterValue("lfo" + idx + "Rate");
        }
        modulationMatrix.setLFOShape(i, static_cast<LFOShape>(
            (int)*apvts.getRawParameterValue("lfo" + idx + "Shape")));
    }

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

    for (int lfo = 0; lfo < 8; ++lfo)
    {
        ModTargetType lfoRateTarget = lfo < 4
            ? static_cast<ModTargetType>(static_cast<int>(ModTargetType::LFO1Rate) + lfo)
            : static_cast<ModTargetType>(static_cast<int>(ModTargetType::LFO5Rate) + (lfo - 4));
        float newRate = lfoBaseRates[lfo] + modSums[static_cast<int>(lfoRateTarget)];
        modulationMatrix.setLFORate(lfo, juce::jlimit(0.01f, 100.0f, newRate));
    }

    // Mod envelopes 2 & 3 — set params and advance
    modEnv2.setParameters({
        *apvts.getRawParameterValue("modEnv2Attack"),
        *apvts.getRawParameterValue("modEnv2Decay"),
        *apvts.getRawParameterValue("modEnv2Sustain"),
        *apvts.getRawParameterValue("modEnv2Release")
    });
    modEnv3.setParameters({
        *apvts.getRawParameterValue("modEnv3Attack"),
        *apvts.getRawParameterValue("modEnv3Decay"),
        *apvts.getRawParameterValue("modEnv3Sustain"),
        *apvts.getRawParameterValue("modEnv3Release")
    });
    {
        float env2Level = 0.0f, env3Level = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            env2Level = modEnv2.getNextSample();
            env3Level = modEnv3.getNextSample();
        }
        modulationMatrix.setModulationValue(ModSourceType::Envelope2, 0, env2Level);
        modulationMatrix.setModulationValue(ModSourceType::Envelope3, 0, env3Level);
    }

    float effCutoff = juce::jlimit(20.0f, 20000.0f,
        baseFilterCutoff + baseFilterModAmount + modSums[static_cast<int>(ModTargetType::FilterCutoff)]);
    float effRes = juce::jlimit(0.1f, 10.0f,
        baseFilterResonance + modSums[static_cast<int>(ModTargetType::FilterResonance)]);

    // Quick FX — Filter Modifier offsets
    const bool filterModOn = *apvts.getRawParameterValue("filterModEnabled") > 0.5f;
    if (filterModOn)
    {
        const float fmCutoff = *apvts.getRawParameterValue("filterModCutoff");
        const float fmRes    = *apvts.getRawParameterValue("filterModResonance");
        const float fmEnv    = *apvts.getRawParameterValue("filterModEnvDepth");
        // Cutoff: ±1 maps to ±8000 Hz offset (musical range)
        effCutoff = juce::jlimit(20.0f, 20000.0f, effCutoff + fmCutoff * 8000.0f);
        // Resonance: ±1 maps to ±5.0 offset
        effRes = juce::jlimit(0.1f, 10.0f, effRes + fmRes * 5.0f);
        // Env depth: scale modulation value to cutoff offset
        const float env2Val = modulationMatrix.getModulationValue(ModSourceType::Envelope2, 0);
        effCutoff = juce::jlimit(20.0f, 20000.0f, effCutoff + fmEnv * env2Val * 8000.0f);
    }
    synthEngine.setFilterParams(effCutoff, effRes);

    float effVol = juce::jlimit(0.0f, 1.0f,
        masterVolume + modSums[static_cast<int>(ModTargetType::AmpVolume)]);
    float effPan = juce::jlimit(-1.0f, 1.0f, modSums[static_cast<int>(ModTargetType::AmpPan)]);

    // Quick FX — Amp Modifier offsets
    const bool ampModOn = *apvts.getRawParameterValue("ampModEnabled") > 0.5f;
    if (ampModOn)
    {
        const float amVol = *apvts.getRawParameterValue("ampModVolume");
        const float amPan = *apvts.getRawParameterValue("ampModPan");
        effVol = juce::jlimit(0.0f, 1.0f, effVol + amVol * 0.5f);
        effPan = juce::jlimit(-1.0f, 1.0f, effPan + amPan);
    }

    float effSynthVol = juce::jlimit(0.0f, 1.0f,
        effVol + modSums[static_cast<int>(ModTargetType::OscillatorLevel)]);
    synthEngine.setMasterVolume(effSynthVol);
    samplerEngine.setMasterVolume(effVol);

    synthEngine.setPitchBend(basePitchBend + modSums[static_cast<int>(ModTargetType::OscillatorPitch)]);

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

    // Aux sends: capture dry mix before effects chain (drums + all synth engines)
    const float auxSendDelayLevel  = *apvts.getRawParameterValue("auxSendDelay");
    const float auxSendReverbLevel = *apvts.getRawParameterValue("auxSendReverb");
    const bool hasAuxDelaySend  = auxSendDelayLevel  > 0.001f;
    const bool hasAuxReverbSend = auxSendReverbLevel > 0.001f;
    if ((hasAuxDelaySend || hasAuxReverbSend) && numChannels >= 2)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            const float* drums = buffer        .getReadPointer(juce::jmin(ch, numChannels - 1));
            const float* syn   = synthBuffer   .getReadPointer(ch);
            const float* smp   = samplerBuffer .getReadPointer(ch);
            const float* grn   = granularBuffer.getReadPointer(ch);
            const float* lyr   = layerBuffer   .getReadPointer(ch);
            float* dst = auxSendBuffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
                dst[i] = drums[i] + syn[i] + smp[i] + grn[i] + lyr[i];
        }
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

    // Step 1: Mix all synth engine outputs into the main stereo pair of buffer
    // (drums are already in buffer from drumSequencer.process above)
    {
        const int mixCh = juce::jmin(numChannels, 2);
        for (int i = 0; i < numSamples; ++i)
            for (int ch = 0; ch < mixCh; ++ch)
                buffer.getWritePointer(ch)[i] +=
                    synthBuffer   .getReadPointer(ch)[i]
                    + samplerBuffer .getReadPointer(ch)[i]
                    + granularBuffer.getReadPointer(ch)[i]
                    + layerBuffer   .getReadPointer(ch)[i];
    }

    // Step 2: Apply effects chain — oversampled if quality > 1x, else native rate
    {
        const int mixCh = juce::jmin(numChannels, 2);

        if (oversamplingFactor > 1 && activeOversampler)
        {
            juce::dsp::AudioBlock<float> mainBlock (
                buffer.getArrayOfWritePointers(), (size_t)mixCh, (size_t)numSamples);
            auto upBlock  = activeOversampler->processSamplesUp (mainBlock);
            const int upN = (int)upBlock.getNumSamples();
            const int upC = (int)upBlock.getNumChannels();

            // Pre-reverb effects
            for (int i = 0; i < upN; ++i)
                for (int ch = 0; ch < upC; ++ch)
                {
                    float s = upBlock.getSample (ch, i);
                    for (int slot = 0; slot < reverbPos; ++slot)
                        s = applyChainEffect (chain[slot], s, ch);
                    upBlock.setSample (ch, i, s);
                }

            // Reverb (stereo block-level)
            reverb.processBlock (upBlock.getChannelPointer (0),
                                 upBlock.getChannelPointer (1), upN);

            // Post-reverb effects
            if (reverbPos < 5)
                for (int i = 0; i < upN; ++i)
                    for (int ch = 0; ch < upC; ++ch)
                    {
                        float s = upBlock.getSample (ch, i);
                        for (int slot = reverbPos + 1; slot < 6; ++slot)
                            s = applyChainEffect (chain[slot], s, ch);
                        upBlock.setSample (ch, i, s);
                    }

            activeOversampler->processSamplesDown (mainBlock);
        }
        else
        {
            // Native-rate path (no oversampling)
            for (int i = 0; i < numSamples; ++i)
                for (int ch = 0; ch < mixCh; ++ch)
                {
                    float s = buffer.getReadPointer(ch)[i];
                    for (int slot = 0; slot < reverbPos; ++slot)
                        s = applyChainEffect (chain[slot], s, ch);
                    buffer.getWritePointer(ch)[i] = s;
                }

            if (mixCh >= 2)
                reverb.processBlock (buffer.getWritePointer(0),
                                     buffer.getWritePointer(1), numSamples);
            else if (mixCh == 1)
                for (int i = 0; i < numSamples; ++i)
                    buffer.getWritePointer(0)[i] = reverb.process (buffer.getReadPointer(0)[i]);

            if (reverbPos < 5)
                for (int i = 0; i < numSamples; ++i)
                    for (int ch = 0; ch < mixCh; ++ch)
                    {
                        float s = buffer.getReadPointer(ch)[i];
                        for (int slot = reverbPos + 1; slot < 6; ++slot)
                            s = applyChainEffect (chain[slot], s, ch);
                        buffer.getWritePointer(ch)[i] = s;
                    }
        }
    }

    // Pan (always at native rate, after any downsampling)
    const float panGainL = 1.0f - juce::jmax(0.0f, effPan);
    const float panGainR = 1.0f + juce::jmin(0.0f, effPan);
    for (int i = 0; i < numSamples; ++i)
    {
        if (numChannels > 0) buffer.getWritePointer(0)[i] *= panGainL;
        if (numChannels > 1) buffer.getWritePointer(1)[i] *= panGainR;
    }

    // Quick FX — Main Filter (post-effects global filter)
    const bool mainFilterOn = *apvts.getRawParameterValue("mainFilterEnabled") > 0.5f;
    if (mainFilterOn)
    {
        const float mfCutoff = *apvts.getRawParameterValue("mainFilterCutoff");
        const float mfRes    = *apvts.getRawParameterValue("mainFilterResonance");
        const int   mfType   = static_cast<int>(*apvts.getRawParameterValue("mainFilterType"));
        if (mfCutoff != mainFilterPrevCutoff || mfRes != mainFilterPrevRes || mfType != mainFilterPrevType)
        {
            mainFilter.setCutoff(mfCutoff);
            mainFilter.setResonance(mfRes);
            mainFilter.setFilterType(static_cast<Filter::FilterType>(mfType));
            mainFilterPrevCutoff = mfCutoff;
            mainFilterPrevRes    = mfRes;
            mainFilterPrevType   = mfType;
        }
        for (int i = 0; i < numSamples; ++i)
        {
            if (numChannels > 0)
                buffer.getWritePointer(0)[i] = mainFilter.process(buffer.getReadPointer(0)[i]);
            if (numChannels > 1)
                buffer.getWritePointer(1)[i] = mainFilter.process(buffer.getReadPointer(1)[i]);
        }
    }

    // Aux sends: parallel delay/reverb returns mixed into main output
    if (numChannels >= 2)
    {
        if (hasAuxDelaySend)
        {
            auxDelay.setTime    (juce::jlimit(0.0f,  2.0f,  baseDelayTime));
            auxDelay.setFeedback(juce::jlimit(0.0f, 0.95f, baseDelayFeedback));
            auxDelay.setMix(1.0f);
            for (int ch = 0; ch < 2; ++ch)
                auxWorkBuffer.copyFrom(ch, 0, auxSendBuffer, ch, 0, numSamples);
            auxWorkBuffer.applyGain(auxSendDelayLevel);
            for (int i = 0; i < numSamples; ++i)
                for (int ch = 0; ch < 2; ++ch)
                    auxWorkBuffer.getWritePointer(ch)[i] = auxDelay.process(auxWorkBuffer.getReadPointer(ch)[i]);
            for (int ch = 0; ch < 2; ++ch)
                buffer.addFrom(ch, 0, auxWorkBuffer, ch, 0, numSamples);
        }
        if (hasAuxReverbSend)
        {
            auxReverb.setRoomSize (juce::jlimit(0.0f, 1.0f, baseReverbRoom));
            auxReverb.setDamping  (juce::jlimit(0.0f, 1.0f, baseReverbDamp));
            auxReverb.setWetLevel (1.0f);
            auxReverb.setDryLevel (0.0f);
            auxReverb.setPreDelay (baseReverbPreDelay);
            auxReverb.setLFDamping(baseReverbLFDamp);
            auxReverb.setWidth    (baseReverbWidth);
            auxReverb.setFreeze   (baseReverbFreeze);
            for (int ch = 0; ch < 2; ++ch)
                auxWorkBuffer.copyFrom(ch, 0, auxSendBuffer, ch, 0, numSamples);
            auxWorkBuffer.applyGain(auxSendReverbLevel);
            auxReverb.processBlock(auxWorkBuffer.getWritePointer(0), auxWorkBuffer.getWritePointer(1), numSamples);
            for (int ch = 0; ch < 2; ++ch)
                buffer.addFrom(ch, 0, auxWorkBuffer, ch, 0, numSamples);
        }
    }

    // Route individual layer buses (buses 1-8)
    for (int li = 0; li < LayerManager::NUM_LAYERS; ++li)
    {
        auto& layer = layerManager.getLayer (li);
        const int busIdx = layer.getOutputBusIndex();
        if (busIdx <= 0 || layer.isMuted()) continue;
        if (layerManager.isAnyLayerSoloed() && !layer.isSoloed()) continue;
        if (busIdx >= getBusCount (false)) continue;
        auto busBuf = getBusBuffer (buffer, false, busIdx);
        if (busBuf.getNumChannels() < 2) continue;
        busBuf.clear();
        layer.processBlock (busBuf, numSamples);
    }

    // Route individual drum track buses (buses 9-16)
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        const int busIdx = drumSequencer.getTrackOutputBus (t);
        if (busIdx <= 0) continue;
        if (busIdx >= getBusCount (false)) continue;
        auto busBuf = getBusBuffer (buffer, false, busIdx);
        if (busBuf.getNumChannels() < 2) continue;
        const auto& tBuf = drumSequencer.getTrackBuffer (t);
        for (int ch = 0; ch < 2; ++ch)
            busBuf.addFrom (ch, 0, tBuf, juce::jmin (ch, tBuf.getNumChannels() - 1), 0, numSamples);
    }

    // Metronome click output — decaying sine into buffer (no allocation)
    if (metClickSamplesLeft > 0)
    {
        const float clickVol = *apvts.getRawParameterValue("metronomeVolume");
        constexpr float twoPi = 6.28318530718f;
        const float freqHz  = metClickIsDownbeat ? 1200.0f : 900.0f;
        const float phaseInc = twoPi * freqHz / static_cast<float>(getSampleRate());
        const int samplesToWrite = juce::jmin(metClickSamplesLeft, numSamples);

        for (int i = 0; i < samplesToWrite; ++i)
        {
            const float t   = 1.0f - (float)metClickSamplesLeft / (float)metClickDuration;
            const float env = std::exp(-t * 25.0f);
            const float s   = std::sin((float)metClickSamplePos * phaseInc) * env * clickVol * 0.35f;
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.getWritePointer(ch)[i] += s;
            ++metClickSamplePos;
            --metClickSamplesLeft;
        }
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

    // Preset metadata
    root.setProperty("category",    currentPresetCategory, nullptr);
    root.setProperty("author",      currentPresetAuthor, nullptr);
    root.setProperty("description", currentPresetDescription, nullptr);
    root.setProperty("tags",         currentPresetTags, nullptr);
    root.setProperty("characters",  currentPresetCharacters, nullptr);

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

    // Read preset metadata from root attributes
    currentPresetCategory    = xml->getStringAttribute("category", "Init");
    currentPresetAuthor      = xml->getStringAttribute("author", "MultiphaseAudio");
    currentPresetDescription = xml->getStringAttribute("description", "");
    currentPresetTags        = xml->getStringAttribute("tags", "");
    currentPresetCharacters  = xml->getStringAttribute("characters", "");

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
    presetManager.pushHistory(index);
    return true;
}

void PluginProcessor::triggerPreviewNote()
{
    // Cancel any existing preview first
    int cur = previewNote.load(std::memory_order_relaxed);
    if (cur >= 0)
    {
        previewPrevNote.store(cur, std::memory_order_relaxed);
        previewNote.store(-1, std::memory_order_relaxed);
        previewSamplesLeft.store(0, std::memory_order_relaxed);
        previewNoteOn.store(false, std::memory_order_relaxed);
    }

    // Start new preview after a tiny delay (1 block for prev note to release)
    previewNote.store(PREVIEW_NOTE, std::memory_order_relaxed);
    double sr = getSampleRate() > 0 ? getSampleRate() : 44100.0;
    previewSamplesLeft.store(static_cast<int>(sr * 0.3), std::memory_order_relaxed);
    previewNoteOn.store(true, std::memory_order_relaxed);
}

void PluginProcessor::cancelPreviewNote()
{
    int cur = previewNote.load(std::memory_order_relaxed);
    if (cur >= 0)
    {
        previewPrevNote.store(cur, std::memory_order_relaxed);
        previewNote.store(-1, std::memory_order_relaxed);
        previewSamplesLeft.store(0, std::memory_order_relaxed);
        previewNoteOn.store(false, std::memory_order_relaxed);
    }
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
