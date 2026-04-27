#pragma once
#include <JuceHeader.h>
#include <array>
#include <cstdint>

static constexpr int ARP_MAX_STEPS = 32;
static constexpr int ARP_MAX_NOTES = 32;

enum class ArpMode { Up, Down, UpDown, Random, Chord };

struct ArpStep
{
    bool  active     = true;
    int   noteOffset = 0;    // -12 to +12 semitones applied on top of the held note
    int   octave     = 0;    // -2 to +2 octave shift
    float velocity   = 0.8f;
    float gate       = 0.5f; // 0.01 – 0.99 fraction of step duration
    bool  tie        = false; // hold previous note through this step
};

class Arpeggiator
{
public:
    Arpeggiator();

    // Call once from prepareToPlay
    void prepare (double sampleRate, float bpm);

    // Call every block; outputs NoteOn/NoteOff into midi
    void process (juce::MidiBuffer& midi, int numSamples);

    void start();
    void stop();
    bool isPlaying() const { return playing; }

    void setEnabled (bool e);
    bool isEnabled()  const { return enabled; }

    void setBPM            (float newBpm);
    void syncToDAWPosition (double ppqStepPos);

    // Call with raw keyboard NoteOn/Off BEFORE passing them to the synth
    void noteOn  (int midiNote);
    void noteOff (int midiNote);
    void clearHeldNotes();

    void    setMode    (ArpMode m) { mode = m; }
    ArpMode getMode()        const { return mode; }

    void setNumSteps (int n) { numSteps = juce::jlimit (1, ARP_MAX_STEPS, n); }
    int  getNumSteps() const { return numSteps; }

    ArpStep&       getStep (int i)       { return steps[i]; }
    const ArpStep& getStep (int i) const { return steps[i]; }
    int  getCurrentStep() const          { return stepIndex; }

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& v);

private:
    std::array<ArpStep, ARP_MAX_STEPS> steps {};
    int     numSteps = 8;
    ArpMode mode     = ArpMode::Up;
    bool    enabled  = false;
    bool    playing  = false;

    // Keyboard-held notes, sorted ascending
    std::array<int, ARP_MAX_NOTES> heldNotes {};
    int heldNoteCount = 0;

    double sampleRate     = 44100.0;
    float  bpm            = 120.0f;
    double samplesPerStep = 0.0;

    // Timeline-based scheduling (same pattern as ProSequencer)
    int64_t currentSamplePos = 0;
    double  nextStepSample   = 0.0;
    double  nextGridSample   = 0.0;

    int  stepIndex     = 0;
    int  noteIndex     = 0;      // position within heldNotes for current mode
    int  noteDirection = 1;      // +1 / -1 for UpDown bounce

    // Active notes (multiple for Chord mode)
    std::array<int, ARP_MAX_NOTES> activeNotes {};
    int    activeNoteCount   = 0;
    double noteOffCountdown  = 0.0;
    bool   tieActive         = false;

    mutable uint32_t rngState = 54321u;

    void  updateSamplesPerStep();
    void  scheduleNextStep (double fromGridSample, int afterSampleOffset);
    void  triggerStep      (juce::MidiBuffer& midi, int sampleOffset);
    void  sendAllNoteOffs  (juce::MidiBuffer& midi, int sampleOffset);
    int   selectNoteIndex  ();
    float fastRand01       () const;
    void  sortHeldNotes    ();
};
