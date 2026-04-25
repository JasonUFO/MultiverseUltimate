#include "Voice.h"
#include <cmath>

Voice::Voice() {}

void Voice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    active = true;
    
    float freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    oscillator.setFrequency(freq);
    envelope.triggerAttack();
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

void Voice::setWaveform(WaveformType type)
{
    oscillator.setWaveform(type);
}

void Voice::setSampleRate(float sr)
{
    oscillator.setSampleRate(sr);
    filter.setSampleRate(sr);
}

float Voice::process()
{
    if (!active) return 0.0f;
    
    float output = oscillator.process();
    output = envelope.process(output);
    output = filter.process(output);
    
    return output;
}