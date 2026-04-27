#pragma once
#include <JuceHeader.h>
#include <array>

static constexpr int PRO_SEQ_LANES = 4;
static constexpr int PRO_SEQ_STEPS = 32;

enum class ProPlayMode { Forward, Reverse, Random };

struct ProStep
{
    bool  active      = false;
    int   note        = 60;
    float velocity    = 0.8f;
    float gate        = 0.5f;
    float probability = 1.0f;
    int   ratchet     = 1;
};

struct ProLane
{
    std::array<ProStep, PRO_SEQ_STEPS> steps {};
    int         numSteps = 16;
    ProPlayMode playMode = ProPlayMode::Forward;
};

class ProSequencer
{
public:
    ProSequencer();

    void prepare (double sampleRate, float bpm);
    void process (juce::MidiBuffer& midi, int numSamples);

    void start();
    void stop();
    bool isPlaying() const { return playing; }

    void setBPM (float newBpm);
    void syncToDAWPosition (double ppqStepPos);

    ProLane&       getLane (int lane)       { return lanes[lane]; }
    const ProLane& getLane (int lane) const { return lanes[lane]; }

    void setStepActive      (int lane, int step, bool  active);
    void setStepNote        (int lane, int step, int   note);
    void setStepVelocity    (int lane, int step, float vel);
    void setStepGate        (int lane, int step, float gate);
    void setStepProbability (int lane, int step, float prob);
    void setStepRatchet     (int lane, int step, int   ratchet);

    int getCurrentStep (int lane) const { return stepIndex[lane]; }

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& state);

private:
    std::array<ProLane, PRO_SEQ_LANES> lanes {};

    double sampleRate     = 44100.0;
    float  bpm            = 120.0f;
    double samplesPerStep = 0.0;
    double sampleCounter  = 0.0;   // counts DOWN; fires when <= 0

    bool playing = false;

    std::array<int,    PRO_SEQ_LANES> stepIndex       {};
    std::array<int,    PRO_SEQ_LANES> activeNote       {};  // -1 = none
    std::array<double, PRO_SEQ_LANES> noteOffCountdown {};  // counts DOWN to noteOff
    std::array<int,    PRO_SEQ_LANES> ratchetCount     {};  // re-fires remaining
    std::array<double, PRO_SEQ_LANES> ratchetSubPhase  {};  // samples per ratchet sub-step
    std::array<int,    PRO_SEQ_LANES> currentNote      {};
    std::array<float,  PRO_SEQ_LANES> currentVelocity  {};
    std::array<float,  PRO_SEQ_LANES> currentGate      {};

    mutable uint32_t rngState = 12345u;

    void  updateSamplesPerStep();
    float fastRand01() const;
    int   getNextStep (int lane) const;
    void  triggerStep (juce::MidiBuffer& midi, int lane, int sampleOffset);
    void  sendNoteOff (juce::MidiBuffer& midi, int lane, int sampleOffset);
    void  retriggerNote (juce::MidiBuffer& midi, int lane, int sampleOffset);
};
