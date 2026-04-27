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
    oscillator.setFrequency(baseFrequency);
    envelope.triggerAttack();
}

void Voice::setPitchBend(float semitones)
{
    oscillator.setFrequency(baseFrequency * std::pow(2.0f, semitones / 12.0f));
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

void Voice::setWaveform(WaveformType type)
{
    if (type != oscillator.getWaveform())
        oscillator.setWaveform(type);
}

void Voice::setSampleRate(float sr)
{
    oscillator.setSampleRate(sr);
    filter.setSampleRate(sr);
    envelope.setSampleRate(sr);
}

float Voice::process()
{
    if (!active) return 0.0f;
    
    float output = oscillator.process();
    output = envelope.process(output);
    output = filter.process(output);
    
    return output;
}