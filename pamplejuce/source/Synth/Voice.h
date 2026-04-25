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
    void setWaveform(WaveformType type);
    void setSampleRate(float sr);
    
    float process();
    
    int getMidiNote() const { return midiNote; }
    float getVelocity() const { return velocity; }

private:
    Oscillator oscillator;
    Envelope envelope;
    Filter filter;
    
    int midiNote = -1;
    float velocity = 0.0f;
    bool active = false;
};