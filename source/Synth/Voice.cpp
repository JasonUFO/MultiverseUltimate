#include "Voice.h"
#include <cmath>

Voice::Voice()
{
    envelope.setParameters(attackTime, decayTime, sustainLevel, releaseTime);
}

void Voice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    active = true;

    baseFrequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
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
    for (auto& osc : oscStates)
    {
        osc.classicOsc.setSampleRate(sr);
        osc.wavetableOsc.prepare(sr);
    }
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

    oscMix = envelope.process(oscMix);
    oscMix = filter.process(oscMix);

    return oscMix;
}
