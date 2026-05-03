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

struct SubOscState
{
    bool enabled = false;
    float level = 0.5f;
    WaveformType waveform = WaveformType::Sine;
    Oscillator osc;
};

struct NoiseOscState
{
    bool enabled = false;
    float level = 0.3f;
    float colorCutoffHz = 20000.0f;
    Oscillator noiseOsc;       // WaveformType::Noise
    float colorFilterState = 0.0f;
    float colorCoeff = 1.0f;   // 1 - exp(-2pi*fc/sr), 1.0 = full white noise
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

    // Sub oscillator
    void setSubOscEnabled(bool e);
    void setSubOscLevel(float l);
    void setSubOscWaveform(WaveformType wf);

    // Noise oscillator
    void setNoiseOscEnabled(bool e);
    void setNoiseOscLevel(float l);
    void setNoiseOscColor(float cutoffHz);

    // Filter topology
    void setFilterType(Filter::FilterType t);

    WavetableOscillator& getWavetableOsc(int oscIndex) { return oscStates[juce::jlimit(0, 2, oscIndex)].wavetableOsc; }

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
    float storedSampleRate = 44100.0f;

    std::array<OscState, 3> oscStates;
    SubOscState  subOsc;
    NoiseOscState noiseOsc;
    Filter filter;
    Envelope envelope;
};
