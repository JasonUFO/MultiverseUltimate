#pragma once
#include <JuceHeader.h>
#include "GrainState.h"

static constexpr int MAX_GRAINS = 32;

enum class GrainEnvShape { Gaussian = 0, Hann = 1, Trapezoid = 2, Triangle = 3 };

class GranularVoice
{
public:
    GranularVoice() = default;

    void prepare (double sampleRate);

    // Called when a note is triggered
    void noteOn  (int midiNote, float velocity);
    void noteOff ();

    bool isActive() const noexcept { return active; }
    int  getMidiNote() const noexcept { return note; }

    // Called once per block; writes output into outL/outR (additive)
    void processBlock (float* outL, float* outR, int numSamples,
                       const float* srcL, const float* srcR, int srcLength,
                       float position,        // 0..1 normalized position in source
                       float grainSizeSec,
                       float spray,
                       float density,         // grains/sec
                       float pitchScatter,    // semitones
                       GrainEnvShape envShape,
                       bool  reverse,
                       float stereoSpread,
                       float attackSec, float decaySec,
                       float sustainLevel, float releaseSec);

private:
    GrainState grains[MAX_GRAINS];

    double sampleRate  = 44100.0;
    bool   active      = false;
    bool   noteReleased = false;
    int    note        = 60;
    float  velocity    = 1.0f;
    float  pitchRatio  = 1.0f;   // base ratio from MIDI note (relative to middle C)

    // Voice ADSR envelope
    enum class ADSRStage { Idle, Attack, Decay, Sustain, Release };
    ADSRStage adsrStage   = ADSRStage::Idle;
    float     adsrLevel   = 0.0f;
    float     adsrAttackCoeff  = 0.0f;
    float     adsrDecayCoeff   = 0.0f;
    float     adsrReleaseCoeff = 0.0f;
    float     adsrSustainLevel = 0.7f;

    float spawnTimer    = 0.0f;  // samples until next grain spawn

    juce::Random rng;

    int  findFreeGrain() const noexcept;
    void spawnGrain (const float* srcL, const float* srcR, int srcLength,
                     float position, float grainSizeSec, float spray,
                     float pitchScatter, bool reverse, float stereoSpread);
    float applyEnvelope (GrainEnvShape shape, float phase) const noexcept;
    void  updateADSRCoeffs (float attackSec, float decaySec,
                            float sustainLevel, float releaseSec);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularVoice)
};
