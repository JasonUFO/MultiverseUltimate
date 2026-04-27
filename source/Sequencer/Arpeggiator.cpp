#include "Arpeggiator.h"
#include <algorithm>
#include <cmath>

Arpeggiator::Arpeggiator()
{
    activeNotes.fill (-1);
    heldNotes.fill (0);
}

void Arpeggiator::prepare (double sr, float bpmIn)
{
    sampleRate = sr;
    bpm        = bpmIn;
    updateSamplesPerStep();

    currentSamplePos = 0;
    nextGridSample   = 0.0;
    scheduleNextStep (0.0, -1);
}

void Arpeggiator::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing) return;

    for (int i = 0; i < numSamples; ++i)
    {
        const int64_t absPos = currentSamplePos + i;

        // Gate-based NoteOff countdown
        if (noteOffCountdown > 0.0)
        {
            noteOffCountdown -= 1.0;
            if (noteOffCountdown <= 0.0)
                sendAllNoteOffs (midi, i);
        }

        // Step trigger
        if (absPos >= static_cast<int64_t> (nextStepSample))
        {
            triggerStep (midi, i);

            stepIndex = (stepIndex + 1) % numSteps;

            nextGridSample += samplesPerStep;
            scheduleNextStep (nextGridSample, i);
        }
    }

    currentSamplePos += numSamples;
}

void Arpeggiator::start()
{
    currentSamplePos = 0;
    stepIndex        = 0;
    noteIndex        = 0;
    noteDirection    = 1;
    activeNoteCount  = 0;
    noteOffCountdown = 0.0;
    tieActive        = false;
    playing          = true;

    nextGridSample = 0.0;
    scheduleNextStep (0.0, -1);
}

void Arpeggiator::stop()
{
    playing          = false;
    noteOffCountdown = 0.0;
    tieActive        = false;
    activeNoteCount  = 0;
}

void Arpeggiator::setEnabled (bool e)
{
    enabled = e;
    if (!e)
    {
        stop();
        clearHeldNotes();
    }
}

void Arpeggiator::setBPM (float newBpm)
{
    bpm = newBpm;
    updateSamplesPerStep();
}

void Arpeggiator::syncToDAWPosition (double ppqStepPos)
{
    double intPart  = 0.0;
    double fracPart = std::modf (ppqStepPos, &intPart);

    double samplesUntilNext = (1.0 - fracPart) * samplesPerStep;
    if (samplesUntilNext >= samplesPerStep)
        samplesUntilNext = 0.0;

    int baseStep = static_cast<int> (intPart) + (fracPart > 1.0e-9 ? 1 : 0);
    stepIndex    = baseStep % numSteps;

    double gridPos = static_cast<double> (currentSamplePos) + samplesUntilNext;
    nextGridSample = gridPos;
    scheduleNextStep (gridPos, -1);
}

void Arpeggiator::noteOn (int midiNote)
{
    if (heldNoteCount >= ARP_MAX_NOTES) return;
    for (int i = 0; i < heldNoteCount; ++i)
        if (heldNotes[i] == midiNote) return;  // already held

    heldNotes[heldNoteCount++] = midiNote;
    sortHeldNotes();
}

void Arpeggiator::noteOff (int midiNote)
{
    for (int i = 0; i < heldNoteCount; ++i)
    {
        if (heldNotes[i] == midiNote)
        {
            for (int j = i; j < heldNoteCount - 1; ++j)
                heldNotes[j] = heldNotes[j + 1];
            --heldNoteCount;

            if (heldNoteCount > 0)
                noteIndex = noteIndex % heldNoteCount;
            else
                noteIndex = 0;

            return;
        }
    }
}

void Arpeggiator::clearHeldNotes()
{
    heldNoteCount = 0;
    noteIndex     = 0;
}

// ---------------------------------------------------------------------------
// State persistence
// ---------------------------------------------------------------------------

juce::ValueTree Arpeggiator::getState() const
{
    juce::ValueTree root ("Arpeggiator");
    root.setProperty ("enabled",  enabled,    nullptr);
    root.setProperty ("mode",     (int) mode, nullptr);
    root.setProperty ("numSteps", numSteps,   nullptr);

    for (int s = 0; s < ARP_MAX_STEPS; ++s)
    {
        const auto& st = steps[s];
        juce::ValueTree stepNode ("Step");
        stepNode.setProperty ("index",      s,             nullptr);
        stepNode.setProperty ("active",     st.active,     nullptr);
        stepNode.setProperty ("noteOffset", st.noteOffset, nullptr);
        stepNode.setProperty ("octave",     st.octave,     nullptr);
        stepNode.setProperty ("velocity",   st.velocity,   nullptr);
        stepNode.setProperty ("gate",       st.gate,       nullptr);
        stepNode.setProperty ("tie",        st.tie,        nullptr);
        root.appendChild (stepNode, nullptr);
    }

    return root;
}

void Arpeggiator::setState (const juce::ValueTree& v)
{
    if (!v.hasType ("Arpeggiator")) return;

    enabled  = (bool)(int) v.getProperty ("enabled",  false);
    mode     = static_cast<ArpMode> ((int) v.getProperty ("mode",     0));
    numSteps = juce::jlimit (1, ARP_MAX_STEPS, (int) v.getProperty ("numSteps", 8));

    for (auto stepNode : v)
    {
        if (!stepNode.hasType ("Step")) continue;
        int s = (int) stepNode.getProperty ("index", -1);
        if (s < 0 || s >= ARP_MAX_STEPS) continue;

        auto& st      = steps[s];
        st.active     = (bool)(int) stepNode.getProperty ("active",     true);
        st.noteOffset = juce::jlimit (-12, 12,    (int)   stepNode.getProperty ("noteOffset", 0));
        st.octave     = juce::jlimit (-2,  2,     (int)   stepNode.getProperty ("octave",     0));
        st.velocity   = juce::jlimit (0.0f, 1.0f, (float) stepNode.getProperty ("velocity",   0.8f));
        st.gate       = juce::jlimit (0.01f, 0.99f, (float) stepNode.getProperty ("gate",     0.5f));
        st.tie        = (bool)(int) stepNode.getProperty ("tie", false);
    }
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

void Arpeggiator::updateSamplesPerStep()
{
    // 16th-note steps: 4 per beat
    samplesPerStep = (sampleRate * 60.0) / (static_cast<double> (bpm) * 4.0);
}

void Arpeggiator::scheduleNextStep (double fromGridSample, int afterSampleOffset)
{
    double scheduled = fromGridSample;

    if (afterSampleOffset >= 0)
    {
        // Called inside process(): must fire strictly after current sample
        const double minNext = static_cast<double> (currentSamplePos) + afterSampleOffset + 1.0;
        scheduled = std::max (scheduled, minNext);
    }
    else
    {
        scheduled = std::max (scheduled, static_cast<double> (currentSamplePos));
    }

    nextStepSample = scheduled;
}

void Arpeggiator::triggerStep (juce::MidiBuffer& midi, int sampleOffset)
{
    const auto& step = steps[stepIndex];

    // Tie: hold current note through this step, cancel pending gate cutoff
    if (step.tie && step.active && activeNoteCount > 0)
    {
        noteOffCountdown = 0.0;
        tieActive = true;
        return;
    }

    // Release whatever is currently held (gated or tied)
    sendAllNoteOffs (midi, sampleOffset);
    tieActive = false;

    if (!step.active || heldNoteCount == 0)
        return;

    if (mode == ArpMode::Chord)
    {
        for (int i = 0; i < heldNoteCount; ++i)
        {
            int note = juce::jlimit (0, 127,
                heldNotes[i] + step.noteOffset + step.octave * 12);
            const auto vel = static_cast<uint8_t> (
                juce::jlimit (1, 127, static_cast<int> (step.velocity * 127.0f)));
            midi.addEvent (juce::MidiMessage::noteOn (1, note, vel), sampleOffset);
            activeNotes[activeNoteCount++] = note;
        }
    }
    else
    {
        int idx  = selectNoteIndex();
        int note = juce::jlimit (0, 127,
            heldNotes[idx] + step.noteOffset + step.octave * 12);
        const auto vel = static_cast<uint8_t> (
            juce::jlimit (1, 127, static_cast<int> (step.velocity * 127.0f)));
        midi.addEvent (juce::MidiMessage::noteOn (1, note, vel), sampleOffset);
        activeNotes[0]  = note;
        activeNoteCount = 1;
    }

    noteOffCountdown = samplesPerStep
                       * static_cast<double> (juce::jlimit (0.01f, 0.99f, step.gate));
}

void Arpeggiator::sendAllNoteOffs (juce::MidiBuffer& midi, int sampleOffset)
{
    for (int i = 0; i < activeNoteCount; ++i)
        if (activeNotes[i] >= 0)
            midi.addEvent (juce::MidiMessage::noteOff (1, activeNotes[i]), sampleOffset);
    activeNoteCount  = 0;
    noteOffCountdown = 0.0;
}

int Arpeggiator::selectNoteIndex()
{
    if (heldNoteCount == 0) return 0;

    switch (mode)
    {
        case ArpMode::Up:
        {
            if (noteIndex >= heldNoteCount) noteIndex = 0;
            return noteIndex++;
        }

        case ArpMode::Down:
        {
            // Map ascending counter to descending index
            if (noteIndex >= heldNoteCount) noteIndex = 0;
            int idx = heldNoteCount - 1 - noteIndex;
            if (++noteIndex >= heldNoteCount) noteIndex = 0;
            return idx;
        }

        case ArpMode::UpDown:
        {
            if (heldNoteCount == 1) { noteIndex = 0; return 0; }
            int idx = juce::jlimit (0, heldNoteCount - 1, noteIndex);
            noteIndex += noteDirection;
            if (noteIndex >= heldNoteCount) { noteDirection = -1; noteIndex = heldNoteCount - 2; }
            else if (noteIndex < 0)         { noteDirection =  1; noteIndex = 1; }
            return idx;
        }

        case ArpMode::Random:
            return static_cast<int> (fastRand01() * static_cast<float> (heldNoteCount))
                   % heldNoteCount;

        case ArpMode::Chord:
            return 0; // handled in triggerStep directly

        default:
            return 0;
    }
}

float Arpeggiator::fastRand01() const
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState & 0x7fffffffu) / static_cast<float> (0x80000000u);
}

void Arpeggiator::sortHeldNotes()
{
    std::sort (heldNotes.begin(), heldNotes.begin() + heldNoteCount);
}
