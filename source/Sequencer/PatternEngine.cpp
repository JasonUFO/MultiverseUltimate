#include "PatternEngine.h"
#include <cmath>
#include <algorithm>

PatternEngine::PatternEngine()
{
    for (int s = 0; s < PATTERN_MAX_STEPS; ++s)
    {
        steps[s].active = (s < 4);
        steps[s].note = 60;
        steps[s].velocity = 0.8f;
        steps[s].gate = 0.5f;
        steps[s].probability = 1.0f;
        steps[s].ratchet = 1;
    }

    activeNote = -1;
    noteOffCountdown = 0.0;
    ratchetCount = 0;
    stepIndex = 0;
    ratchetSubPhase = 0.0;
    currentVelocity = 0.8f;
    currentGate = 0.5f;
    nextStepSample = 0.0;
    nextGridSample = 0.0;

    steps[0].active = true;
    steps[0].note = 36;
    steps[4].active = true;
    steps[4].note = 36;
    steps[8].active = true;
    steps[8].note = 36;
    steps[12].active = true;
    steps[12].note = 36;
}

void PatternEngine::prepare (double sr, float bpmIn)
{
    sampleRate = sr;
    bpm = bpmIn;
    updateSamplesPerStep();

    currentSamplePos = 0;
    activeNote = -1;
    noteOffCountdown = 0.0;
    ratchetCount = 0;
    stepIndex = 0;
    ratchetSubPhase = 0.0;
    tieActive = false;

    nextGridSample = 0.0;
    scheduleNextStep (0.0, -1);
}

void PatternEngine::process (juce::MidiBuffer& midi, int numSamples)
{
    if (!playing || !enabled) return;

    for (int i = 0; i < numSamples; ++i)
    {
        const int64_t absPos = currentSamplePos + i;

        if (noteOffCountdown > 0.0)
        {
            noteOffCountdown -= 1.0;
            if (noteOffCountdown <= 0.0 && activeNote >= 0 && !tieActive)
            {
                midi.addEvent (juce::MidiMessage::noteOff (1, activeNote), i);
                activeNote = -1;
            }
        }

        if (ratchetCount > 0)
        {
            ratchetSubPhase -= 1.0;
            if (ratchetSubPhase <= 0.0)
            {
                const int step = stepIndex;
                if (step >= 0 && step < numSteps && steps[step].active)
                {
                    if (fastRand01() <= steps[step].probability)
                    {
                        const int note = generateNote (step);
                        const float vel = steps[step].velocity;
                        midi.addEvent (juce::MidiMessage::noteOn (1, note, vel).withTimeStamp (absPos), i);
                        activeNote = note;
                        currentVelocity = vel;
                        currentGate = steps[step].gate;
                        noteOffCountdown = samplesPerStep * currentGate;
                        if (steps[step].tie)
                            tieActive = true;
                    }
                }
                ratchetCount -= 1;
                if (ratchetCount > 0)
                    ratchetSubPhase = samplesPerStep / static_cast<double> (steps[step].ratchet);
            }
        }

        if (absPos >= nextStepSample && stepIndex < numSteps)
        {
            const int step = stepIndex;
            const auto& st = steps[step];

            if (st.active && fastRand01() <= st.probability)
            {
                const int note = generateNote (step);
                const float vel = st.velocity;

                midi.addEvent (juce::MidiMessage::noteOn (1, note, vel).withTimeStamp (absPos), i);
                activeNote = note;
                currentVelocity = vel;
                currentGate = st.gate;
                noteOffCountdown = samplesPerStep * currentGate;

                tieActive = false;
                if (st.tie)
                    tieActive = true;

                ratchetCount = st.ratchet;
                ratchetSubPhase = samplesPerStep / static_cast<double> (st.ratchet);
            }
            else
            {
                ratchetCount = 0;
            }

            stepIndex = (stepIndex + 1) % numSteps;
            scheduleNextStep (nextGridSample, i);
        }
    }

    currentSamplePos += numSamples;
}

void PatternEngine::start()
{
    playing = true;
    if (numSteps > 0)
    {
        nextGridSample = 0.0;
        scheduleNextStep (0.0, -1);
    }
}

void PatternEngine::stop()
{
    playing = false;
    activeNote = -1;
    noteOffCountdown = 0.0;
    tieActive = false;
}

void PatternEngine::stopWithMidi (juce::MidiBuffer& midi)
{
    playing = false;
    if (activeNote >= 0)
    {
        midi.addEvent (juce::MidiMessage::noteOff (1, activeNote), 0);
    }
    activeNote = -1;
    noteOffCountdown = 0.0;
    tieActive = false;
}

void PatternEngine::setBPM (float newBpm)
{
    bpm = newBpm;
    updateSamplesPerStep();
}

void PatternEngine::setSwingAmount (float swing)
{
    swingAmount = juce::jlimit (0.0f, 1.0f, swing);
    if (playing)
    {
        nextGridSample = 0.0;
        scheduleNextStep (0.0, -1);
    }
}

void PatternEngine::syncToDAWPosition (double ppqStepPos)
{
    if (!playing || samplesPerStep <= 0.0) return;

    const double gridPos = ppqStepPos * 4.0;
    const int newStep = static_cast<int> (gridPos) % numSteps;
    const double subStep = gridPos - static_cast<int> (gridPos);

    nextGridSample = (static_cast<double> (newStep) + subStep) * samplesPerStep;
    stepIndex = newStep;

    const int step = stepIndex;
    if (step >= 0 && step < numSteps && steps[step].active)
    {
        const double grooveOffset = steps[step].microTiming * samplesPerStep * 0.5;
        const double swingOffset = swingAmount * samplesPerStep * 0.5;
        nextStepSample = currentSamplePos + nextGridSample + grooveOffset + swingOffset;
    }
    else
    {
        scheduleNextStep (nextGridSample, -1);
    }
}

void PatternEngine::noteOn (int midiNote)
{
    if (midiNote < 0 || midiNote > 127) return;

    auto it = std::find (inputNotes.begin(), inputNotes.end(), midiNote);
    if (it == inputNotes.end())
    {
        inputNotes.push_back (midiNote);
        std::sort (inputNotes.begin(), inputNotes.end());
    }

    if (mode == PatternMode::Arp || mode == PatternMode::Phrase)
    {
        if (!enabled)
        {
            enabled = true;
            playing = true;
        }
    }
}

void PatternEngine::noteOff (int midiNote)
{
    auto it = std::find (inputNotes.begin(), inputNotes.end(), midiNote);
    if (it != inputNotes.end())
        inputNotes.erase (it);

    if (mode == PatternMode::Arp || mode == PatternMode::Phrase)
    {
        if (inputNotes.empty())
        {
            enabled = false;
            playing = false;
        }
        else
        {
            noteIndex = selectNoteIndex();
        }
    }
}

void PatternEngine::clearInputNotes()
{
    inputNotes.clear();
    enabled = false;
    playing = false;
    activeNote = -1;
    noteOffCountdown = 0.0;
}

int PatternEngine::generateNote (int stepIdx)
{
    if (stepIdx < 0 || stepIdx >= PATTERN_MAX_STEPS)
        return 60;

    const auto& st = steps[stepIdx];

    switch (mode)
    {
        case PatternMode::Sequencer:
            return st.note;

        case PatternMode::Arp:
        case PatternMode::Phrase:
        {
            if (inputNotes.empty())
                return 60;

            const int idx = selectNoteIndex();
            int baseNote = inputNotes[idx % inputNotes.size()];

            baseNote += st.noteOffset + (st.octave * 12);
            return juce::jlimit (0, 127, baseNote);
        }
    }

    return 60;
}

int PatternEngine::selectNoteIndex()
{
    if (inputNotes.empty())
        return 0;

    switch (noteOrder)
    {
        case NoteOrder::Up:
            noteIndex = (noteIndex + noteDirection);
            if (noteIndex >= static_cast<int> (inputNotes.size()))
                noteIndex = 0;
            noteDirection = 1;
            break;

        case NoteOrder::Down:
            noteIndex = (noteIndex - noteDirection);
            if (noteIndex < 0)
                noteIndex = static_cast<int> (inputNotes.size()) - 1;
            noteDirection = 1;
            break;

        case NoteOrder::Random:
            noteIndex = static_cast<int> (fastRand01() * static_cast<float> (inputNotes.size()));
            break;

        case NoteOrder::Chord:
            noteIndex = 0;
            break;
    }

    return noteIndex;
}

void PatternEngine::updateSamplesPerStep()
{
    const double beatsPerStep = 0.25;
    samplesPerStep = (sampleRate * 60.0 / bpm) * beatsPerStep;
}

void PatternEngine::scheduleNextStep (double fromGridSample, int afterSampleOffset)
{
    if (numSteps <= 0 || samplesPerStep <= 0.0) return;

    const int step = stepIndex;
    const double grooveOffset = (step >= 0 && step < PATTERN_MAX_STEPS)
        ? steps[step].microTiming * samplesPerStep * 0.5
        : 0.0;
    const double swingOffset = swingAmount * samplesPerStep * 0.5;

    nextGridSample = fromGridSample + samplesPerStep;
    nextStepSample = currentSamplePos + nextGridSample + grooveOffset + swingOffset;
}

float PatternEngine::fastRand01() const
{
    uint32_t x = rngState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rngState = x;
    return (x & 0xFFFFFFu) / 16777216.0f;
}

juce::ValueTree PatternEngine::getState() const
{
    juce::ValueTree root ("PatternEngine");
    root.setProperty ("numSteps", numSteps, nullptr);
    root.setProperty ("mode", static_cast<int> (mode), nullptr);
    root.setProperty ("noteOrder", static_cast<int> (noteOrder), nullptr);
    root.setProperty ("enabled", enabled, nullptr);
    root.setProperty ("swingAmount", swingAmount, nullptr);

    juce::ValueTree stepsNode ("Steps");
    for (int s = 0; s < numSteps; ++s)
    {
        juce::ValueTree stepNode ("Step");
        stepNode.setProperty ("active", steps[s].active, nullptr);
        stepNode.setProperty ("note", steps[s].note, nullptr);
        stepNode.setProperty ("noteOffset", steps[s].noteOffset, nullptr);
        stepNode.setProperty ("octave", steps[s].octave, nullptr);
        stepNode.setProperty ("velocity", steps[s].velocity, nullptr);
        stepNode.setProperty ("gate", steps[s].gate, nullptr);
        stepNode.setProperty ("probability", steps[s].probability, nullptr);
        stepNode.setProperty ("ratchet", steps[s].ratchet, nullptr);
        stepNode.setProperty ("microTiming", steps[s].microTiming, nullptr);
        stepNode.setProperty ("tie", steps[s].tie, nullptr);
        stepsNode.appendChild (stepNode, nullptr);
    }
    root.appendChild (stepsNode, nullptr);

    return root;
}

void PatternEngine::setState (const juce::ValueTree& state)
{
    if (!state.hasType ("PatternEngine")) return;

    numSteps = state.getProperty ("numSteps", 16);
    mode = static_cast<PatternMode> ((int)state.getProperty ("mode", 0));
    noteOrder = static_cast<NoteOrder> ((int)state.getProperty ("noteOrder", 0));
    enabled = state.getProperty ("enabled", false);
    swingAmount = (float)state.getProperty ("swingAmount", 0.0);

    auto stepsNode = state.getChildWithName ("Steps");
    if (stepsNode.isValid())
    {
        for (int s = 0; s < stepsNode.getNumChildren() && s < PATTERN_MAX_STEPS; ++s)
        {
            auto stepNode = stepsNode.getChild (s);
            steps[s].active = stepNode.getProperty ("active", true);
            steps[s].note = stepNode.getProperty ("note", 60);
            steps[s].noteOffset = stepNode.getProperty ("noteOffset", 0);
            steps[s].octave = stepNode.getProperty ("octave", 0);
            steps[s].velocity = stepNode.getProperty ("velocity", 0.8f);
            steps[s].gate = stepNode.getProperty ("gate", 0.5f);
            steps[s].probability = stepNode.getProperty ("probability", 1.0f);
            steps[s].ratchet = stepNode.getProperty ("ratchet", 1);
            steps[s].microTiming = stepNode.getProperty ("microTiming", 0.0f);
            steps[s].tie = stepNode.getProperty ("tie", false);
        }
    }
}