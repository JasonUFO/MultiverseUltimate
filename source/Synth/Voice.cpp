#include "Voice.h"
#include <cmath>

Voice::Voice()
{
    envelope.setParameters(attackTime, decayTime, sustainLevel, releaseTime);
    noiseOsc.noiseOsc.setWaveform(WaveformType::Noise);
    noiseOsc.noiseOsc.setSampleRate(storedSampleRate);
    subOsc.osc.setSampleRate(storedSampleRate);
}

void Voice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    active = true;

    baseFrequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    subOsc.osc.setFrequency(baseFrequency * 0.5f);
    updateOscillatorFrequencies();
    envelope.triggerAttack();
}

void Voice::setPitchBend(float semitones)
{
    pitchBendSemitones = semitones;
    updateOscillatorFrequencies();
}

void Voice::updateOscillatorFrequencies()
{
    float bentFreq = baseFrequency * std::pow(2.0f, pitchBendSemitones / 12.0f);
    for (auto& osc : oscStates)
    {
        float freq = bentFreq * std::pow(2.0f, osc.detuneSemitones / 12.0f);
        osc.classicOsc.setFrequency(freq);
        osc.wavetableOsc.setFrequency(freq);
    }
    subOsc.osc.setFrequency(bentFreq * 0.5f);
}

void Voice::noteOff()
{
    envelope.triggerRelease();
}

bool Voice::isActive() const
{
    return active && !envelope.isFinished();
}

void Voice::setEnvelopeParams(float a, float d, float s, float r)
{
    envelope.setParameters(a, d, s, r);
}

void Voice::setFilterParams(float cutoff, float resonance)
{
    filter.setCutoff(cutoff);
    filter.setResonance(resonance);
}

void Voice::setOversamplingMode(Filter::OversamplingMode mode)
{
    filter.setOversamplingMode(mode);
}

void Voice::setSampleRate(float sr)
{
    storedSampleRate = sr;
    for (auto& osc : oscStates)
    {
        osc.classicOsc.setSampleRate(sr);
        osc.wavetableOsc.prepare(sr);
    }
    subOsc.osc.setSampleRate(sr);
    noiseOsc.noiseOsc.setSampleRate(sr);
    // Recompute noise color coefficient: one-pole LP coeff = 1 - exp(-2pi*fc/sr)
    const float fc = juce::jlimit(20.0f, 20000.0f, noiseOsc.colorCutoffHz);
    noiseOsc.colorCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * fc / sr);
    filter.setSampleRate(sr);
    envelope.setSampleRate(sr);
}

void Voice::setOscillatorType(int index, OscillatorType t)
{
    if (index < 0 || index > 2) return;
    oscStates[index].type = t;
}

void Voice::setOscillatorLevel(int index, float level)
{
    if (index < 0 || index > 2) return;
    oscStates[index].level = juce::jlimit(0.0f, 1.0f, level);
}

void Voice::setOscillatorDetune(int index, float detuneSemitones)
{
    if (index < 0 || index > 2) return;
    oscStates[index].detuneSemitones = detuneSemitones;
    updateOscillatorFrequencies();
}

void Voice::setOscillatorWaveform(int index, WaveformType wf)
{
    if (index < 0 || index > 2) return;
    oscStates[index].classicOsc.setWaveform(wf);
}

void Voice::setOscillatorWavePosition(int index, float pos)
{
    if (index < 0 || index > 2) return;
    oscStates[index].wavetableOsc.setWavePosition(pos);
}

void Voice::setSubOscEnabled(bool e)  { subOsc.enabled = e; }
void Voice::setSubOscLevel(float l)   { subOsc.level = juce::jlimit(0.0f, 1.0f, l); }
void Voice::setSubOscWaveform(WaveformType wf) { subOsc.osc.setWaveform(wf); }

void Voice::setNoiseOscEnabled(bool e) { noiseOsc.enabled = e; }
void Voice::setNoiseOscLevel(float l)  { noiseOsc.level = juce::jlimit(0.0f, 1.0f, l); }
void Voice::setNoiseOscColor(float hz)
{
    noiseOsc.colorCutoffHz = juce::jlimit(20.0f, 20000.0f, hz);
    if (storedSampleRate > 0.0f)
        noiseOsc.colorCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi
                                               * noiseOsc.colorCutoffHz / storedSampleRate);
}

void Voice::setFilterType(Filter::FilterType t) { filter.setFilterType(t); }

void Voice::loadWavetableData(int oscIndex, const juce::AudioBuffer<float>& audio)
{
    if (oscIndex < 0 || oscIndex > 2) return;
    oscStates[oscIndex].wavetableOsc.loadWavetable(audio);
}

void Voice::setFrequencyDirect(float hz)
{
    baseFrequency = juce::jmax(1.0f, hz);
    updateOscillatorFrequencies();
}

void Voice::setNoteLegato(int note)
{
    midiNote = note;
    baseFrequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

float Voice::process()
{
    if (!active) return 0.0f;

    float oscMix = 0.0f;
    for (auto& osc : oscStates)
    {
        float s = 0.0f;
        if (osc.type == OscillatorType::Classic)
            s = osc.classicOsc.process();
        else
            s = osc.wavetableOsc.process();
        oscMix += s * osc.level;
    }

    if (subOsc.enabled)
        oscMix += subOsc.osc.process() * subOsc.level;

    if (noiseOsc.enabled)
    {
        float rawNoise = noiseOsc.noiseOsc.process();
        noiseOsc.colorFilterState = noiseOsc.colorCoeff * rawNoise
                                  + (1.0f - noiseOsc.colorCoeff) * noiseOsc.colorFilterState;
        oscMix += noiseOsc.colorFilterState * noiseOsc.level;
    }

    oscMix = envelope.process(oscMix);
    oscMix = filter.process(oscMix);

    return oscMix;
}
