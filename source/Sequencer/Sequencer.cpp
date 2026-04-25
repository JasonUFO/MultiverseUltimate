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
    currentStep.store (0);
}

void Sequencer::updateSamplesPerStep()
{
    // Duration of one 16th note
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
    currentStep.store (0);
    playing.store (true);
}

void Sequencer::stop()
{
    playing.store (false);
    lastNoteOn = -1;
}

void Sequencer::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing.load())
        return;

    for (int s = 0; s < numSamples; ++s)
    {
        if (sampleCounter <= 0.0)
        {
            triggerStep (midi, s);
            sampleCounter += samplesPerStep;
        }
        sampleCounter -= 1.0;
    }
}

void Sequencer::triggerStep (juce::MidiBuffer& midi, int sampleOffset)
{
    if (lastNoteOn >= 0)
    {
        midi.addEvent (juce::MidiMessage::noteOff (1, lastNoteOn), sampleOffset);
        lastNoteOn = -1;
    }

    int step = currentStep.load();

    if (mode == SequencerMode::Sequencer)
    {
        const auto& s = currentPattern.steps[step];
        if (s.active)
        {
            auto vel = static_cast<uint8_t> (juce::jlimit (1, 127, static_cast<int> (s.velocity * 127.0f)));
            midi.addEvent (juce::MidiMessage::noteOn (1, s.noteNumber, vel), sampleOffset);
            lastNoteOn = s.noteNumber;
        }
    }
    else // Arpeggiator
    {
        if (arpNoteCount > 0)
        {
            int note = arpNotes[arpIndex % arpNoteCount];
            midi.addEvent (juce::MidiMessage::noteOn (1, note, (uint8_t) 100), sampleOffset);
            lastNoteOn = note;
            arpIndex = (arpIndex + 1) % arpNoteCount;
        }
    }

    currentStep.store ((step + 1) % currentPattern.numSteps);
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
        std::sort (arpNotes.begin(), arpNotes.begin() + arpNoteCount);
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

juce::MidiFile Sequencer::exportMidi() const
{
    juce::MidiFile midiFile;
    const int ticksPerBeat = 480;
    midiFile.setTicksPerQuarterNote (ticksPerBeat);

    juce::MidiMessageSequence track;

    auto microsPerBeat = static_cast<int> (60000000.0 / static_cast<double> (bpm));
    track.addEvent (juce::MidiMessage::tempoMetaEvent (microsPerBeat), 0);

    const int ticksPerStep = ticksPerBeat / 4; // one 16th note

    for (int i = 0; i < currentPattern.numSteps; ++i)
    {
        const auto& step = currentPattern.steps[i];
        if (!step.active)
            continue;

        double startTick = static_cast<double> (i * ticksPerStep);
        double endTick = startTick + static_cast<double> (ticksPerStep) - 1.0;
        auto vel = static_cast<uint8_t> (juce::jlimit (1, 127, static_cast<int> (step.velocity * 127.0f)));

        track.addEvent (juce::MidiMessage::noteOn (1, step.noteNumber, vel), startTick);
        track.addEvent (juce::MidiMessage::noteOff (1, step.noteNumber), endTick);
    }

    track.updateMatchedPairs();
    midiFile.addTrack (track);

    return midiFile;
}
