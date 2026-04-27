#pragma once
#include <JuceHeader.h>
#include <array>
#include <cstdint>
#include <vector>

static constexpr int PATTERN_MAX_STEPS = 32;
static constexpr int PATTERN_MAX_NOTES = 32;

enum class PatternMode { Sequencer, Arp, Phrase };
enum class NoteOrder { Up, Down, Random, Chord };

struct PatternStep
{
    bool active = true;
    int note = 60;
    int noteOffset = 0;
    int octave = 0;
    float velocity = 0.8f;
    float gate = 0.5f;
    float probability = 1.0f;
    int ratchet = 1;
    float microTiming = 0.0f;
    bool tie = false;
};

class PatternEngine
{
public:
    PatternEngine();

    void prepare (double sampleRate, float bpm);
    void process (juce::MidiBuffer& midi, int numSamples);

    void start();
    void stop();
    void stopWithMidi (juce::MidiBuffer& midi);
    bool isPlaying() const { return playing; }

    void setEnabled (bool e) { enabled = e; }
    bool isEnabled() const { return enabled; }

    void setBPM (float newBpm);
    void setSwingAmount (float swing);
    void syncToDAWPosition (double ppqStepPos);

    void noteOn (int midiNote);
    void noteOff (int midiNote);
    void clearInputNotes();
    const std::vector<int>& getInputNotes() const { return inputNotes; }

    void setMode (PatternMode m) { mode = m; }
    PatternMode getMode() const { return mode; }

    void setNoteOrder (NoteOrder o) { noteOrder = o; }
    NoteOrder getNoteOrder() const { return noteOrder; }

    void setNumSteps (int n) { numSteps = juce::jlimit (1, PATTERN_MAX_STEPS, n); }
    int getNumSteps() const { return numSteps; }

    PatternStep& getStep (int i) { return steps[i]; }
    const PatternStep& getStep (int i) const { return steps[i]; }

    int getCurrentStep() const { return stepIndex; }
    float getSwingAmount() const { return swingAmount; }

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& state);

private:
    std::array<PatternStep, PATTERN_MAX_STEPS> steps {};
    int numSteps = 16;

    PatternMode mode = PatternMode::Sequencer;
    NoteOrder noteOrder = NoteOrder::Up;
    bool enabled = false;
    bool playing = false;

    std::vector<int> inputNotes;
    std::vector<int> activeNotes;

    double sampleRate = 44100.0;
    float bpm = 120.0f;
    float swingAmount = 0.0f;
    double samplesPerStep = 0.0;

    int64_t currentSamplePos = 0;
    double nextStepSample = 0.0;
    double nextGridSample = 0.0;

    int stepIndex = 0;
    int noteIndex = 0;
    int noteDirection = 1;

    double noteOffCountdown = 0.0;
    int activeNote = -1;
    bool tieActive = false;
    int ratchetCount = 0;
    double ratchetSubPhase = 0.0;
    float currentVelocity = 0.8f;
    float currentGate = 0.5f;

    mutable uint32_t rngState = 12345u;

    void updateSamplesPerStep();
    void scheduleNextStep (double fromGridSample, int afterSampleOffset);
    float fastRand01() const;

    int selectNoteIndex();
    int generateNote (int stepIdx);

    void triggerStep (juce::MidiBuffer& midi, int sampleOffset);
    void sendNoteOff (juce::MidiBuffer& midi, int sampleOffset);
};