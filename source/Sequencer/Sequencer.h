#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>

constexpr int MAX_STEPS = 16;
constexpr int MAX_PATTERNS = 8;

struct Step
{
    bool active = false;
    int noteNumber = 60;
    float velocity = 0.8f;
    float gate = 0.5f;
};

struct SeqPattern
{
    std::array<Step, MAX_STEPS> steps {};
    int numSteps = 16;
};

enum class SequencerMode
{
    Sequencer,
    Arpeggiator
};

enum class PlayMode
{
    Forward,
    Reverse,
    Random
};

class Sequencer
{
public:
    Sequencer();

    void prepare (double sampleRate, float bpm);
    void process (juce::MidiBuffer& midi, int numSamples);

    void start();
    void stop();
    bool isPlaying() const { return playing.load(); }

    void setBPM (float bpm);
    float getBPM() const { return bpm; }

    void setMode (SequencerMode m) { mode = m; }
    SequencerMode getMode() const { return mode; }

    void setPlayMode (PlayMode m) { playMode = m; }
    PlayMode getPlayMode() const { return playMode; }

    void setNumSteps (int n);
    int getNumSteps() const { return currentPattern.numSteps; }

    void setStepActive (int step, bool active);
    void setStepNote (int step, int noteNumber);
    void setStepVelocity (int step, float velocity);
    void setStepGate (int step, float gate);
    Step getStep (int step) const;
    int getCurrentStep() const { return currentStep.load(); }

    void savePattern (int slot);
    void loadPattern (int slot);
    int getCurrentPatternSlot() const { return currentPatternSlot; }

    void addArpNote (int noteNumber);
    void removeArpNote (int noteNumber);
    void clearArpNotes();

    juce::MidiFile exportMidi() const;

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& state);

private:
    SeqPattern currentPattern;
    std::array<SeqPattern, MAX_PATTERNS> savedPatterns {};
    int currentPatternSlot = 0;

    SequencerMode mode { SequencerMode::Sequencer };
    PlayMode playMode { PlayMode::Forward };

    float bpm = 120.0f;
    double sampleRate = 44100.0;
    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;

    double noteOffCounter = 0.0;
    int lastNoteOn = -1;

    std::atomic<int> currentStep { 0 };
    std::atomic<bool> playing { false };

    std::array<int, 16> arpNotes {};
    int arpNoteCount = 0;
    int arpIndex = 0;

    uint32_t rngState = 1;

    void updateSamplesPerStep();
    int getNextStepIndex (int current) const;
    uint32_t fastRand();
    void triggerNoteOn (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity);
    void scheduleNoteOff (juce::MidiBuffer& midi, int sampleOffset);
};