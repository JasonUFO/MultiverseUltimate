#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>

constexpr int MAX_STEPS = 16;
constexpr int MAX_PATTERNS = 8;

struct Step
{
    int noteNumber = 60;
    float velocity = 0.8f;
    bool active = false;
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

    void setNumSteps (int n);
    int getNumSteps() const { return currentPattern.numSteps; }

    void setStepActive (int step, bool active);
    void setStepNote (int step, int noteNumber);
    void setStepVelocity (int step, float velocity);
    Step getStep (int step) const;
    int getCurrentStep() const { return currentStep.load(); }

    void savePattern (int slot);
    void loadPattern (int slot);
    int getCurrentPatternSlot() const { return currentPatternSlot; }

    void addArpNote (int noteNumber);
    void removeArpNote (int noteNumber);
    void clearArpNotes();

    juce::MidiFile exportMidi() const;

    // State persistence
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

 private:
    SeqPattern currentPattern;
    std::array<SeqPattern, MAX_PATTERNS> savedPatterns {};
    int currentPatternSlot = 0;

    SequencerMode mode { SequencerMode::Sequencer };

    float bpm = 120.0f;
    double sampleRate = 44100.0;
    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;

    std::atomic<int> currentStep { 0 };
    std::atomic<bool> playing { false };
    int lastNoteOn = -1;

    std::array<int, 16> arpNotes {};
    int arpNoteCount = 0;
    int arpIndex = 0;

    void updateSamplesPerStep();
    void triggerStep (juce::MidiBuffer& midi, int sampleOffset);
};
