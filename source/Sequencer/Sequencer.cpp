#include "Sequencer.h"
#include <algorithm>

Sequencer::Sequencer()
{
    arpNotes.fill (-1);
    updateSamplesPerStep();
}

void Sequencer::prepare (double sr, float bpmVal)
{
    sampleRate = sr;
    bpm = bpmVal;
    updateSamplesPerStep();
    sampleCounter = 0.0;
    noteOffCounter = 0.0;
    currentStep.store (0);
}

void Sequencer::updateSamplesPerStep()
{
    samplesPerStep = sampleRate * 60.0 / (static_cast<double> (bpm) * 4.0);
}

void Sequencer::setBPM (float bpmVal)
{
    bpm = bpmVal;
    updateSamplesPerStep();
}

void Sequencer::setNumSteps (int n)
{
    currentPattern.numSteps = juce::jlimit (1, MAX_STEPS, n);
}

void Sequencer::start()
{
    sampleCounter = 0.0;
    noteOffCounter = 0.0;
    currentStep.store (0);
    playing.store (true);
}

void Sequencer::stop()
{
    playing.store (false);
    lastNoteOn = -1;
    noteOffCounter = 0.0;
}

uint32_t Sequencer::fastRand()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

int Sequencer::getNextStepIndex (int current) const
{
    const int numSteps = currentPattern.numSteps;

    switch (playMode)
    {
        case PlayMode::Forward:
            return (current + 1) % numSteps;

        case PlayMode::Reverse:
            return (current - 1 + numSteps) % numSteps;

        case PlayMode::Random:
        {
            uint32_t r = const_cast<Sequencer*> (this)->fastRand();
            return static_cast<int> (r % static_cast<uint32_t> (numSteps));
        }
    }

    return (current + 1) % numSteps;
}

void Sequencer::triggerNoteOn (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity)
{
    int midiVel = static_cast<int> (juce::jlimit (0.0f, 127.0f, velocity * 127.0f));
    if (midiVel == 0) midiVel = 1;
    midi.addEvent (juce::MidiMessage::noteOn (1, note, static_cast<uint8_t> (midiVel)), sampleOffset);
}

void Sequencer::scheduleNoteOff (juce::MidiBuffer& midi, int sampleOffset)
{
    if (lastNoteOn >= 0)
    {
        midi.addEvent (juce::MidiMessage::noteOff (1, lastNoteOn), sampleOffset);
        lastNoteOn = -1;
    }
}

void Sequencer::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing.load())
        return;

    for (int s = 0; s < numSamples; ++s)
    {
        if (noteOffCounter > 0.0)
        {
            noteOffCounter -= 1.0;
            if (noteOffCounter <= 0.0)
                scheduleNoteOff (midi, s);
        }

        if (sampleCounter <= 0.0)
        {
            int step = currentStep.load();

            if (mode == SequencerMode::Sequencer)
            {
                const auto& st = currentPattern.steps[step];
                if (st.active)
                {
                    triggerNoteOn (midi, s, st.noteNumber, st.velocity);
                    noteOffCounter = samplesPerStep * static_cast<double> (st.gate);
                    lastNoteOn = st.noteNumber;
                }
            }
            else
            {
                if (arpNoteCount > 0)
                {
                    int note = arpNotes[arpIndex % arpNoteCount];
                    triggerNoteOn (midi, s, note, 0.8f);
                    noteOffCounter = samplesPerStep * 0.5;
                    lastNoteOn = note;
                    arpIndex = (arpIndex + 1) % arpNoteCount;
                }
            }

            int nextStep = getNextStepIndex (step);
            currentStep.store (nextStep);
            sampleCounter += samplesPerStep;
        }
        sampleCounter -= 1.0;
    }
}

void Sequencer::setStepActive (int step, bool active)
{
    if (step >= 0 && step < MAX_STEPS)
        currentPattern.steps[step].active = active;
}

void Sequencer::setStepNote (int step, int noteNumber)
{
    if (step >= 0 && step < MAX_STEPS)
        currentPattern.steps[step].noteNumber = juce::jlimit (0, 127, noteNumber);
}

void Sequencer::setStepVelocity (int step, float velocity)
{
    if (step >= 0 && step < MAX_STEPS)
        currentPattern.steps[step].velocity = juce::jlimit (0.0f, 1.0f, velocity);
}

void Sequencer::setStepGate (int step, float gate)
{
    if (step >= 0 && step < MAX_STEPS)
        currentPattern.steps[step].gate = juce::jlimit (0.0f, 1.0f, gate);
}

Step Sequencer::getStep (int step) const
{
    if (step >= 0 && step < MAX_STEPS)
        return currentPattern.steps[step];
    return {};
}

void Sequencer::savePattern (int slot)
{
    if (slot >= 0 && slot < MAX_PATTERNS)
    {
        savedPatterns[slot] = currentPattern;
        currentPatternSlot = slot;
    }
}

void Sequencer::loadPattern (int slot)
{
    if (slot >= 0 && slot < MAX_PATTERNS)
    {
        currentPattern = savedPatterns[slot];
        currentPatternSlot = slot;
    }
}

void Sequencer::addArpNote (int noteNumber)
{
    for (int i = 0; i < arpNoteCount; ++i)
        if (arpNotes[i] == noteNumber) return;

    if (arpNoteCount < 16)
    {
        arpNotes[arpNoteCount++] = noteNumber;
        for (int i = arpNoteCount - 1; i > 0; --i)
        {
            if (arpNotes[i] < arpNotes[i - 1])
                std::swap (arpNotes[i], arpNotes[i - 1]);
            else
                break;
        }
    }
}

void Sequencer::removeArpNote (int noteNumber)
{
    for (int i = 0; i < arpNoteCount; ++i)
    {
        if (arpNotes[i] == noteNumber)
        {
            for (int j = i; j < arpNoteCount - 1; ++j)
                arpNotes[j] = arpNotes[j + 1];
            --arpNoteCount;
            if (arpNoteCount > 0)
                arpIndex = arpIndex % arpNoteCount;
            else
                arpIndex = 0;
            return;
        }
    }
}

void Sequencer::clearArpNotes()
{
    arpNoteCount = 0;
    arpIndex = 0;
}

void Sequencer::syncToStep (int step)
{
    int targetStep = step % currentPattern.numSteps;
    if (targetStep < 0)
        targetStep += currentPattern.numSteps;
    currentStep.store(targetStep);
    sampleCounter = 0.0;
}

juce::MidiFile Sequencer::exportMidi() const
{
    juce::MidiFile midiFile;
    const int ticksPerBeat = 480;
    midiFile.setTicksPerQuarterNote (ticksPerBeat);

    juce::MidiMessageSequence track;

    auto microsPerBeat = static_cast<int> (60000000.0 / static_cast<double> (bpm));
    track.addEvent (juce::MidiMessage::tempoMetaEvent (microsPerBeat), 0);

    const int ticksPerStep = ticksPerBeat / 4;

    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        const auto& s = currentPattern.steps[i];
        if (!s.active)
            continue;

        double startTick = static_cast<double> (i * ticksPerStep);
        double endTick = startTick + static_cast<double> (ticksPerStep * s.gate) - 1.0;
        int midiVel = static_cast<int> (juce::jlimit (1, 127, static_cast<int> (s.velocity * 127.0f)));

        track.addEvent (juce::MidiMessage::noteOn (1, s.noteNumber, static_cast<uint8_t> (midiVel)), startTick);
        track.addEvent (juce::MidiMessage::noteOff (1, s.noteNumber), endTick);
    }

    track.updateMatchedPairs();
    midiFile.addTrack (track);

    return midiFile;
}

juce::ValueTree Sequencer::getState() const
{
    juce::ValueTree v ("Sequencer");
    v.setProperty ("bpm", bpm, nullptr);
    v.setProperty ("mode", static_cast<int> (mode), nullptr);
    v.setProperty ("playMode", static_cast<int> (playMode), nullptr);
    v.setProperty ("currentPatternSlot", currentPatternSlot, nullptr);
    v.setProperty ("numSteps", currentPattern.numSteps, nullptr);

    juce::ValueTree pattern ("CurrentPattern");
    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        const auto& s = currentPattern.steps[i];
        juce::ValueTree stepNode ("Step");
        stepNode.setProperty ("step", i, nullptr);
        stepNode.setProperty ("active", s.active, nullptr);
        stepNode.setProperty ("noteNumber", s.noteNumber, nullptr);
        stepNode.setProperty ("velocity", s.velocity, nullptr);
        stepNode.setProperty ("gate", s.gate, nullptr);
        pattern.appendChild (stepNode, nullptr);
    }
    v.appendChild (pattern, nullptr);

    juce::ValueTree saved ("SavedPatterns");
    for (int slot = 0; slot < MAX_PATTERNS; ++slot)
    {
        const auto& pat = savedPatterns[slot];
        juce::ValueTree patNode ("Pattern");
        patNode.setProperty ("slot", slot, nullptr);
        for (int i = 0; i < pat.numSteps; ++i)
        {
            const auto& s = pat.steps[i];
            juce::ValueTree stepNode ("Step");
            stepNode.setProperty ("step", i, nullptr);
            stepNode.setProperty ("active", s.active, nullptr);
            stepNode.setProperty ("noteNumber", s.noteNumber, nullptr);
            stepNode.setProperty ("velocity", s.velocity, nullptr);
            stepNode.setProperty ("gate", s.gate, nullptr);
            patNode.appendChild (stepNode, nullptr);
        }
        saved.appendChild (patNode, nullptr);
    }
    v.appendChild (saved, nullptr);

    return v;
}

void Sequencer::setState (const juce::ValueTree& state)
{
    if (!state.hasType ("Sequencer"))
        return;

    if (state.hasProperty ("bpm"))
        setBPM (static_cast<float> (state.getProperty ("bpm")));
    if (state.hasProperty ("mode"))
        mode = static_cast<SequencerMode> (static_cast<int> (state.getProperty ("mode")));
    if (state.hasProperty ("playMode"))
        playMode = static_cast<PlayMode> (static_cast<int> (state.getProperty ("playMode")));
    if (state.hasProperty ("currentPatternSlot"))
        currentPatternSlot = static_cast<int> (state.getProperty ("currentPatternSlot"));
    if (state.hasProperty ("numSteps"))
        setNumSteps (static_cast<int> (state.getProperty ("numSteps")));

    auto patternNode = state.getChildWithName ("CurrentPattern");
    if (patternNode.isValid())
    {
        for (auto stepNode : patternNode)
        {
            if (stepNode.hasType ("Step"))
            {
                int idx = static_cast<int> (stepNode.getProperty ("step"));
                if (idx >= 0 && idx < MAX_STEPS)
                {
                    currentPattern.steps[idx].active = static_cast<bool> (stepNode.getProperty ("active"));
                    currentPattern.steps[idx].noteNumber = static_cast<int> (stepNode.getProperty ("noteNumber"));
                    currentPattern.steps[idx].velocity = static_cast<float> (stepNode.getProperty ("velocity"));
                    currentPattern.steps[idx].gate = static_cast<float> (stepNode.getProperty ("gate"));
                }
            }
        }
    }

    auto savedNode = state.getChildWithName ("SavedPatterns");
    if (savedNode.isValid())
    {
        for (auto patNode : savedNode)
        {
            if (patNode.hasType ("Pattern"))
            {
                int slot = static_cast<int> (patNode.getProperty ("slot"));
                if (slot >= 0 && slot < MAX_PATTERNS)
                {
                    auto& pat = savedPatterns[slot];
                    pat.numSteps = currentPattern.numSteps;
                    for (int i = 0; i < MAX_STEPS; ++i)
                    {
                        pat.steps[i].active = false;
                        pat.steps[i].noteNumber = 60;
                        pat.steps[i].velocity = 0.8f;
                        pat.steps[i].gate = 0.5f;
                    }
                    for (auto stepNode : patNode)
                    {
                        if (stepNode.hasType ("Step"))
                        {
                            int idx = static_cast<int> (stepNode.getProperty ("step"));
                            if (idx >= 0 && idx < MAX_STEPS)
                            {
                                pat.steps[idx].active = static_cast<bool> (stepNode.getProperty ("active"));
                                pat.steps[idx].noteNumber = static_cast<int> (stepNode.getProperty ("noteNumber"));
                                pat.steps[idx].velocity = static_cast<float> (stepNode.getProperty ("velocity"));
                                pat.steps[idx].gate = static_cast<float> (stepNode.getProperty ("gate"));
                            }
                        }
                    }
                }
            }
        }
    }

    clearArpNotes();
}