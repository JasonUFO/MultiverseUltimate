#pragma once
#include <JuceHeader.h>
class Envelope
{
public:
    Envelope();

    void setParameters(float attack, float decay, float sustain, float release);
    void setSampleRate(float sr);
    void triggerAttack();
    void triggerRelease();
    bool isActive() const;
    bool isFinished() const;

    float process(float input);

    float getAttack() const { return attack; }
    float getDecay() const { return decay; }
    float getSustain() const { return sustain; }
    float getRelease() const { return release; }

private:
    enum class Stage
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    void recomputeIncrements();

    Stage stage = Stage::Idle;
    float value = 0.0f;
    float attack = 0.01f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float release = 0.3f;
    float sampleRate = 44100.0f;

    // Pre-computed per-sample increments
    float attackInc  = 1.0f / (0.01f  * 44100.0f);
    float decayInc   = 1.0f / (0.1f   * 44100.0f);
    float releaseInc = 1.0f / (0.3f   * 44100.0f);
};