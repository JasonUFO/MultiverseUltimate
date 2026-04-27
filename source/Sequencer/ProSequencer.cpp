#include "ProSequencer.h"
#include <cmath>

ProSequencer::ProSequencer()
{
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
    stepIndex.fill (0);
    ratchetSubPhase.fill (0.0);
    currentNote.fill (60);
    currentVelocity.fill (0.8f);
    currentGate.fill (0.5f);

    // Test pattern: lane 0, quarter notes (16th-note steps 0, 4, 8, 12), MIDI note 36
    for (int s : {0, 4, 8, 12})
    {
        lanes[0].steps[s].active   = true;
        lanes[0].steps[s].note     = 36;
        lanes[0].steps[s].velocity = 0.9f;
        lanes[0].steps[s].gate     = 0.8f;
    }
}

void ProSequencer::prepare (double sr, float bpmIn)
{
    sampleRate = sr;
    bpm        = bpmIn;
    updateSamplesPerStep();

    // Fire on the very first sample when start() has not been overridden by sync
    sampleCounter = 0.0;

    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
    stepIndex.fill (0);
}

void ProSequencer::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing) return;

    for (int i = 0; i < numSamples; ++i)
    {
        // --- Gate-based noteOff and ratchet re-trigger (per lane) ---
        for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
        {
            if (noteOffCountdown[lane] > 0.0)
            {
                noteOffCountdown[lane] -= 1.0;

                if (noteOffCountdown[lane] <= 0.0)
                {
                    sendNoteOff (midi, lane, i);

                    if (ratchetCount[lane] > 1)
                    {
                        --ratchetCount[lane];
                        retriggerNote (midi, lane, i);
                        noteOffCountdown[lane] = ratchetSubPhase[lane] * static_cast<double> (currentGate[lane]);
                    }
                }
            }
        }

        // --- Step clock (count-down; fires when counter reaches 0) ---
        sampleCounter -= 1.0;

        if (sampleCounter <= 0.0)
        {
            sampleCounter += samplesPerStep;

            for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
            {
                // Cancel any note that hasn't hit its gate deadline yet
                sendNoteOff (midi, lane, i);
                noteOffCountdown[lane] = 0.0;
                ratchetCount[lane]     = 0;

                // Play the current step, then advance
                triggerStep (midi, lane, i);
                stepIndex[lane] = getNextStep (lane);
            }
        }
    }
}

void ProSequencer::start()
{
    sampleCounter = 0.0;   // fires on first sample
    stepIndex.fill (0);
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
    playing = true;
}

void ProSequencer::stop()
{
    playing = false;
    // activeNote held-state is left so that allNotesOff logic in the processor
    // (synthEngine.allNotesOff) covers any ringing voices.
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
}

void ProSequencer::setBPM (float newBpm)
{
    bpm = newBpm;
    updateSamplesPerStep();
}

void ProSequencer::syncToDAWPosition (double ppqStepPos)
{
    // ppqStepPos = PPQ * 4  (1 unit = one 16th note)
    double intPart  = 0.0;
    double fracPart = std::modf (ppqStepPos, &intPart);

    // Samples until the next step boundary
    double samplesUntilNext = (1.0 - fracPart) * samplesPerStep;

    // If we are right at a boundary (frac ≈ 0) treat as "fire now"
    if (samplesUntilNext >= samplesPerStep)
        samplesUntilNext = 0.0;

    sampleCounter = samplesUntilNext;

    // The step that fires at the next boundary
    int baseStep = static_cast<int> (intPart) + (fracPart > 1.0e-9 ? 1 : 0);

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
        stepIndex[lane] = baseStep % lanes[lane].numSteps;
}

// ---------------------------------------------------------------------------
// Step accessors
// ---------------------------------------------------------------------------

void ProSequencer::setStepActive      (int l, int s, bool  v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].active      = v; }
void ProSequencer::setStepNote        (int l, int s, int   v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].note        = v; }
void ProSequencer::setStepVelocity    (int l, int s, float v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].velocity    = v; }
void ProSequencer::setStepGate        (int l, int s, float v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].gate        = v; }
void ProSequencer::setStepProbability (int l, int s, float v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].probability = v; }
void ProSequencer::setStepRatchet     (int l, int s, int   v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].ratchet     = juce::jmax (1, v); }

// ---------------------------------------------------------------------------
// State persistence
// ---------------------------------------------------------------------------

juce::ValueTree ProSequencer::getState() const
{
    juce::ValueTree root ("ProSequencer");

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
    {
        juce::ValueTree laneNode ("Lane");
        laneNode.setProperty ("index",    lane,                        nullptr);
        laneNode.setProperty ("numSteps", lanes[lane].numSteps,        nullptr);
        laneNode.setProperty ("playMode", (int) lanes[lane].playMode,  nullptr);

        for (int s = 0; s < PRO_SEQ_STEPS; ++s)
        {
            const auto& st = lanes[lane].steps[s];
            juce::ValueTree stepNode ("Step");
            stepNode.setProperty ("index",       s,               nullptr);
            stepNode.setProperty ("active",      st.active,       nullptr);
            stepNode.setProperty ("note",        st.note,         nullptr);
            stepNode.setProperty ("velocity",    st.velocity,     nullptr);
            stepNode.setProperty ("gate",        st.gate,         nullptr);
            stepNode.setProperty ("probability", st.probability,  nullptr);
            stepNode.setProperty ("ratchet",     st.ratchet,      nullptr);
            laneNode.appendChild (stepNode, nullptr);
        }

        root.appendChild (laneNode, nullptr);
    }

    return root;
}

void ProSequencer::setState (const juce::ValueTree& state)
{
    if (!state.hasType ("ProSequencer")) return;

    for (auto laneNode : state)
    {
        if (!laneNode.hasType ("Lane")) continue;
        int lane = (int) laneNode.getProperty ("index", -1);
        if (lane < 0 || lane >= PRO_SEQ_LANES) continue;

        lanes[lane].numSteps = (int) laneNode.getProperty ("numSteps", 16);
        lanes[lane].playMode = static_cast<ProPlayMode> ((int) laneNode.getProperty ("playMode", 0));

        for (auto stepNode : laneNode)
        {
            if (!stepNode.hasType ("Step")) continue;
            int s = (int) stepNode.getProperty ("index", -1);
            if (s < 0 || s >= PRO_SEQ_STEPS) continue;

            auto& st       = lanes[lane].steps[s];
            st.active      = (bool)(int)  stepNode.getProperty ("active",      false);
            st.note        = (int)         stepNode.getProperty ("note",        60);
            st.velocity    = (float)        stepNode.getProperty ("velocity",    0.8f);
            st.gate        = (float)        stepNode.getProperty ("gate",        0.5f);
            st.probability = (float)        stepNode.getProperty ("probability", 1.0f);
            st.ratchet     = juce::jmax (1, (int) stepNode.getProperty ("ratchet", 1));
        }
    }
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

void ProSequencer::updateSamplesPerStep()
{
    // 16th notes: 4 steps per beat
    samplesPerStep = (sampleRate * 60.0) / (static_cast<double> (bpm) * 4.0);
}

float ProSequencer::fastRand01() const
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState & 0x7fffffffu) / static_cast<float> (0x80000000u);
}

int ProSequencer::getNextStep (int lane) const
{
    const auto& l   = lanes[lane];
    const int   cur = stepIndex[lane];

    switch (l.playMode)
    {
        case ProPlayMode::Forward:
            return (cur + 1) % l.numSteps;

        case ProPlayMode::Reverse:
            return (cur - 1 + l.numSteps) % l.numSteps;

        case ProPlayMode::Random:
            return static_cast<int> (fastRand01() * static_cast<float> (l.numSteps)) % l.numSteps;
    }

    return (cur + 1) % l.numSteps;
}

void ProSequencer::triggerStep (juce::MidiBuffer& midi, int lane, int sampleOffset)
{
    const auto& step = lanes[lane].steps[stepIndex[lane]];

    if (!step.active)                      return;
    if (fastRand01() > step.probability)   return;

    const int    R    = juce::jmax (1, step.ratchet);
    const double sub  = samplesPerStep / static_cast<double> (R);
    const float  gate = juce::jlimit (0.01f, 0.99f, step.gate);

    currentNote[lane]     = step.note;
    currentVelocity[lane] = step.velocity;
    currentGate[lane]     = gate;
    ratchetCount[lane]    = R;
    ratchetSubPhase[lane] = sub;

    const auto vel = static_cast<uint8_t> (juce::jlimit (1, 127, static_cast<int> (step.velocity * 127.0f)));
    midi.addEvent (juce::MidiMessage::noteOn (lane + 1, step.note, vel), sampleOffset);
    activeNote[lane]       = step.note;
    noteOffCountdown[lane] = sub * static_cast<double> (gate);
}

void ProSequencer::sendNoteOff (juce::MidiBuffer& midi, int lane, int sampleOffset)
{
    if (activeNote[lane] >= 0)
    {
        midi.addEvent (juce::MidiMessage::noteOff (lane + 1, activeNote[lane]), sampleOffset);
        activeNote[lane] = -1;
    }
}

void ProSequencer::retriggerNote (juce::MidiBuffer& midi, int lane, int sampleOffset)
{
    const auto vel = static_cast<uint8_t> (juce::jlimit (1, 127, static_cast<int> (currentVelocity[lane] * 127.0f)));
    midi.addEvent (juce::MidiMessage::noteOn (lane + 1, currentNote[lane], vel), sampleOffset);
    activeNote[lane] = currentNote[lane];
}
