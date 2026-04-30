#pragma once
#include "Oscillator.h"
#include "WavetableOscillator.h"
#include "Envelope.h"
#include "Filter.h"

enum class OscillatorType { Classic, Wavetable };

struct OscState
{
    OscillatorType type = OscillatorType::Classic;
    Oscillator classicOsc;
    WavetableOscillator wavetableOsc;
    float level = 1.0f;
    float detuneSemitones = 0.0f;
};

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
    void setSampleRate(float sr);
    void setPitchBend(float semitones);

    float process();

    int getMidiNote() const { return midiNote; }
    float getVelocity() const { return velocity; }

    void setOscillatorType(int index, OscillatorType t);
    void setOscillatorLevel(int index, float level);
    void setOscillatorDetune(int index, float detuneSemitones);
    void setOscillatorWaveform(int index, WaveformType wf);
    void setOscillatorWavePosition(int index, float pos);
    void loadWavetableData(int oscIndex, const juce::AudioBuffer<float>& audio);

    void setFrequencyDirect(float hz);
    void setNoteLegato(int note);

private:
    void updateOscillatorFrequencies();

    int midiNote = 60;
    float velocity = 0.5f;
    float baseFrequency = 440.0f;
    float pitchBendSemitones = 0.0f;
    bool active = false;

    float attackTime = 0.01f;
    float decayTime = 0.1f;
    float sustainLevel = 0.7f;
    float releaseTime = 0.3f;
    float filterCutoff = 20000.0f;
    float filterResonance = 0.0f;

    std::array<OscState, 3> oscStates;
    Filter filter;
    Envelope envelope;
};
