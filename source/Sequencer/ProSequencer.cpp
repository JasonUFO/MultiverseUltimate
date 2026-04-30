#include "ProSequencer.h"
#include <cmath>
#include <algorithm>

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
    nextStepSample.fill (0.0);
    nextGridSample.fill (0.0);

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

    currentSamplePos = 0;
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
    stepIndex.fill (0);

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
    {
        nextGridSample[lane] = 0.0;
        scheduleNextStep (lane, 0.0, -1);
    }
}

void ProSequencer::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing) return;

    for (int i = 0; i < numSamples; ++i)
    {
        const int64_t absPos = currentSamplePos + i;

        // --- Gate-based noteOff and ratchet re-trigger ---
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

        // --- Groove-aware step triggers (per lane, independent) ---
        for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
        {
            if (absPos >= static_cast<int64_t> (nextStepSample[lane]))
            {
                // Kill any held note and cancel gate countdown
                sendNoteOff (midi, lane, i);
                noteOffCountdown[lane] = 0.0;
                ratchetCount[lane]     = 0;

                triggerStep (midi, lane, i);
                stepIndex[lane] = getNextStep (lane);

                // Advance the pure grid by one step, then apply groove to find next trigger
                nextGridSample[lane] += samplesPerStep;
                scheduleNextStep (lane, nextGridSample[lane], i);
            }
        }
    }

    currentSamplePos += numSamples;
}

void ProSequencer::start()
{
    currentSamplePos = 0;
    stepIndex.fill (0);
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
    playing = true;

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
    {
        nextGridSample[lane] = 0.0;
        scheduleNextStep (lane, 0.0, -1);
    }
}

void ProSequencer::stop()
{
    playing = false;
    activeNote.fill (-1);
    noteOffCountdown.fill (0.0);
    ratchetCount.fill (0);
}

void ProSequencer::setBPM (float newBpm)
{
    bpm = newBpm;
    updateSamplesPerStep();
}

void ProSequencer::setSwingAmount (float swing)
{
    swingAmount = juce::jlimit (0.0f, 1.0f, swing);
}

void ProSequencer::syncToDAWPosition (double ppqStepPos)
{
    // ppqStepPos = PPQ * 4  (1 unit = one 16th note)
    double intPart  = 0.0;
    double fracPart = std::modf (ppqStepPos, &intPart);

    double samplesUntilNext = (1.0 - fracPart) * samplesPerStep;
    if (samplesUntilNext >= samplesPerStep)
        samplesUntilNext = 0.0;

    int baseStep = static_cast<int> (intPart) + (fracPart > 1.0e-9 ? 1 : 0);

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
    {
        stepIndex[lane] = baseStep % lanes[lane].numSteps;

        double gridPos = static_cast<double> (currentSamplePos) + samplesUntilNext;
        nextGridSample[lane] = gridPos;
        scheduleNextStep (lane, gridPos, -1);
    }
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
void ProSequencer::setStepMicroTiming (int l, int s, float v) { if (l>=0&&l<PRO_SEQ_LANES&&s>=0&&s<PRO_SEQ_STEPS) lanes[l].steps[s].microTiming = juce::jlimit (-1.0f, 1.0f, v); }

// ---------------------------------------------------------------------------
// State persistence
// ---------------------------------------------------------------------------

juce::MidiFile ProSequencer::exportMidi() const
{
    juce::MidiFile midiFile;
    const int ticksPerBeat = 480;
    midiFile.setTicksPerQuarterNote (ticksPerBeat);

    const int ticksPerStep  = ticksPerBeat / 4;
    const int microsPerBeat = static_cast<int> (60000000.0 / static_cast<double> (bpm));

    for (int lane = 0; lane < PRO_SEQ_LANES; ++lane)
    {
        const auto& l = lanes[lane];
        bool hasActive = false;
        for (int s = 0; s < l.numSteps; ++s)
            if (l.steps[s].active) { hasActive = true; break; }
        if (!hasActive) continue;

        juce::MidiMessageSequence track;
        track.addEvent (juce::MidiMessage::tempoMetaEvent (microsPerBeat), 0);

        for (int s = 0; s < l.numSteps; ++s)
        {
            const auto& st = l.steps[s];
            if (!st.active) continue;

            const double startTick = static_cast<double> (s * ticksPerStep);
            const double endTick   = startTick + static_cast<double> (ticksPerStep) * st.gate - 1.0;
            const int vel = juce::jlimit (1, 127, static_cast<int> (st.velocity * 127.0f));

            track.addEvent (juce::MidiMessage::noteOn  (1, st.note, static_cast<uint8_t> (vel)), startTick);
            track.addEvent (juce::MidiMessage::noteOff (1, st.note),                              endTick);
        }
        track.updateMatchedPairs();
        midiFile.addTrack (track);
    }

    return midiFile;
}

// ---------------------------------------------------------------------------

juce::ValueTree ProSequencer::getState() const
{
    juce::ValueTree root ("ProSequencer");
    root.setProperty ("swingAmount", swingAmount, nullptr);

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
            stepNode.setProperty ("microTiming", st.microTiming,  nullptr);
            laneNode.appendChild (stepNode, nullptr);
        }

        root.appendChild (laneNode, nullptr);
    }

    return root;
}

void ProSequencer::setState (const juce::ValueTree& state)
{
    if (!state.hasType ("ProSequencer")) return;

    swingAmount = juce::jlimit (0.0f, 1.0f, (float) state.getProperty ("swingAmount", 0.0f));

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
            st.microTiming = juce::jlimit (-1.0f, 1.0f, (float) stepNode.getProperty ("microTiming", 0.0f));
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

void ProSequencer::scheduleNextStep (int lane, double fromGridSample, int afterSampleOffset)
{
    const int ns = stepIndex[lane];

    // Swing: delay odd-indexed steps by swingAmount * 50% of a step
    const double swingOffset = (ns % 2 == 1)
                               ? static_cast<double> (swingAmount) * samplesPerStep * 0.5
                               : 0.0;

    // Microtiming: per-step nudge, capped at ±50% of a step
    const double microOffset = static_cast<double> (lanes[lane].steps[ns].microTiming)
                               * samplesPerStep * 0.5;

    double scheduled = fromGridSample + swingOffset + microOffset;

    if (afterSampleOffset >= 0)
    {
        // When called from inside process(), must fire strictly after the current sample
        const double minNext = static_cast<double> (currentSamplePos) + afterSampleOffset + 1.0;
        scheduled = std::max (scheduled, minNext);
    }
    else
    {
        // When called outside process() (sync / start / prepare), don't schedule in the past
        scheduled = std::max (scheduled, static_cast<double> (currentSamplePos));
    }

    nextStepSample[lane] = scheduled;
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

    if (!step.active)                    return;
    if (fastRand01() > step.probability) return;

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
