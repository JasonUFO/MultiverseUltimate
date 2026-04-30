#pragma once
#include <JuceHeader.h>
#include <array>
#include <cstdint>

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
    float microTiming = 0.0f;  // -1.0 (early) to +1.0 (late); offset = microTiming * samplesPerStep * 0.5
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

    void setBPM        (float newBpm);
    void setSwingAmount (float swing);   // 0.0 – 1.0
    void syncToDAWPosition (double ppqStepPos);

    ProLane&       getLane (int lane)       { return lanes[lane]; }
    const ProLane& getLane (int lane) const { return lanes[lane]; }

    void setStepActive      (int lane, int step, bool  active);
    void setStepNote        (int lane, int step, int   note);
    void setStepVelocity    (int lane, int step, float vel);
    void setStepGate        (int lane, int step, float gate);
    void setStepProbability (int lane, int step, float prob);
    void setStepRatchet     (int lane, int step, int   ratchet);
    void setStepMicroTiming (int lane, int step, float mt);

    int   getCurrentStep (int lane) const { return stepIndex[lane]; }
    float getSwingAmount ()         const { return swingAmount; }

    float getBPM() const { return bpm; }

    juce::MidiFile exportMidi() const;

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& state);

private:
    std::array<ProLane, PRO_SEQ_LANES> lanes {};

    double sampleRate     = 44100.0;
    float  bpm            = 120.0f;
    float  swingAmount    = 0.0f;
    double samplesPerStep = 0.0;

    // Timeline-based position tracking (replaces count-down sampleCounter)
    int64_t currentSamplePos = 0;
    std::array<double, PRO_SEQ_LANES> nextStepSample {};  // absolute sample of next trigger (with groove)
    std::array<double, PRO_SEQ_LANES> nextGridSample {};  // pure grid position (no groove offsets)

    bool playing = false;

    std::array<int,    PRO_SEQ_LANES> stepIndex       {};
    std::array<int,    PRO_SEQ_LANES> activeNote       {};
    std::array<double, PRO_SEQ_LANES> noteOffCountdown {};
    std::array<int,    PRO_SEQ_LANES> ratchetCount     {};
    std::array<double, PRO_SEQ_LANES> ratchetSubPhase  {};
    std::array<int,    PRO_SEQ_LANES> currentNote      {};
    std::array<float,  PRO_SEQ_LANES> currentVelocity  {};
    std::array<float,  PRO_SEQ_LANES> currentGate      {};

    mutable uint32_t rngState = 12345u;

    void  updateSamplesPerStep();
    // Compute and store nextStepSample[lane] from a grid position.
    // afterSampleOffset: index within current block just used; pass -1 when called outside process().
    void  scheduleNextStep (int lane, double fromGridSample, int afterSampleOffset);
    float fastRand01() const;
    int   getNextStep  (int lane) const;
    void  triggerStep  (juce::MidiBuffer& midi, int lane, int sampleOffset);
    void  sendNoteOff  (juce::MidiBuffer& midi, int lane, int sampleOffset);
    void  retriggerNote (juce::MidiBuffer& midi, int lane, int sampleOffset);
};
