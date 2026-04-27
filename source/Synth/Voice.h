#pragma once
#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

class Voice
{
public:
    Voice();
    
    void noteOn(int midiNote, float velocity);
    void noteOff();
    bool isActive() const;
    
    void setEnvelopeParams(float a, float d, float s, float r);
    void setFilterParams(float cutoff, float resonance);
    void setOversamplingMode(Filter::OversamplingMode mode);
    void setWaveform(WaveformType type);
    void setSampleRate(float sr);
    void setPitchBend(float semitones);
    
    float process();
    
    int getMidiNote() const { return midiNote; }
    float getVelocity() const { return velocity; }

private:
    int midiNote = 60;
    float velocity = 0.5f;
    float baseFrequency = 440.0f;
    bool active = false;
    float attackTime = 0.01f;
    float decayTime = 0.1f;
    float sustainLevel = 0.7f;
    float releaseTime = 0.3f;
    float filterCutoff = 20000.0f;
    float filterResonance = 0.0f;
    WaveformType waveform = WaveformType::Saw;

    Oscillator oscillator;
    Filter filter;
    Envelope envelope;
};