#pragma once
#include "SamplerZone.h"

class SamplerVoice
{
public:
    void prepare (double sampleRate);
    void noteOn (const SamplerZone* zone, int midiNote, float velocity);
    void noteOff();
    void forceStop();
    bool isActive() const noexcept { return active; }
    float process();
    int getMidiNote() const noexcept { return midiNote; }

private:
    float getSampleAt (double pos) const noexcept;

    const SamplerZone* currentZone = nullptr;
    double readPos = 0.0;
    double playbackRate = 1.0;
    bool pingPongForward = true;

    // Time-based ADSR (increments per sample, computed in noteOn)
    enum class EnvStage { Idle, Attack, Decay, Sustain, Release };
    EnvStage envStage = EnvStage::Idle;
    float envValue = 0.0f;
    float attackInc = 0.0f;
    float decayDec = 0.0f;
    float sustainLevel = 0.7f;
    float releaseDec = 0.0f;

    int midiNote = -1;
    float velocity = 0.0f;
    bool active = false;
    double sampleRate = 44100.0;
};
