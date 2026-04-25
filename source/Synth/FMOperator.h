#pragma once
#include <JuceHeader.h>
#include <cmath>

class FMOperator
{
public:
    float ratio = 1.0f;        // frequency ratio relative to base note
    float level = 1.0f;        // modulation index (modulator) or amplitude (carrier)
    float feedback = 0.0f;     // self-feedback amount (0-1)
    float attackTime = 0.01f;  // seconds
    float decayTime = 0.1f;    // seconds
    float sustainLevel = 0.7f; // 0-1
    float releaseTime = 0.3f;  // seconds

    void setSampleRate(double sr);
    void noteOn(float baseFrequency);
    void setFrequency(float baseFrequency);
    void noteOff();
    void reset();

    // modulationPhase: summed phase offset from upstream operators (radians)
    float process(float modulationPhase = 0.0f);
    bool isActive() const;

private:
    enum class EnvStage
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    float processEnvelope();

    EnvStage envStage = EnvStage::Idle;
    float envValue = 0.0f;
    float attackInc = 0.0f;
    float decayInc = 0.0f;
    float releaseInc = 0.0f;

    double phase = 0.0;
    double phaseInc = 0.0;
    float feedbackSample = 0.0f;
    double sampleRate = 44100.0;
};
