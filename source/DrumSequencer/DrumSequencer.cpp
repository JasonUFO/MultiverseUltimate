#include "DrumSequencer.h"
#include <algorithm>

DrumVoice::DrumVoice() {}

void DrumVoice::prepare (double sr)
{
    sampleRate = (sr > 0.0 && sr <= 1000000.0) ? sr : 44100.0;
}

void DrumVoice::trigger (const DrumTrack& track, float velocity)
{
    if (track.sampleBuffer.getNumSamples() == 0)
        return;

    sampleData = track.sampleBuffer.getReadPointer (0);
    sampleLength = track.sampleBuffer.getNumSamples();
    position = 0.0;
    speed = sampleRate / track.sampleRate;
    volume = velocity * track.volume;
    playing = true;
}

void DrumVoice::release()
{
    playing = false;
}

float DrumVoice::process()
{
    if (!playing || sampleData == nullptr || sampleLength == 0)
        return 0.0f;

    int pos = static_cast<int> (position);

    if (pos >= sampleLength - 1)
    {
        playing = false;
        return 0.0f;
    }

    int nextPos = std::min (pos + 1, sampleLength - 1);
    float frac = static_cast<float> (position - pos);
    float s1 = sampleData[pos];
    float s2 = sampleData[nextPos];
    float sample = s1 + frac * (s2 - s1);

    position += speed;
    return sample * volume;
}

void DrumVoice::forceStop()
{
    playing = false;
    position = 0.0;
    sampleData = nullptr;
}

DrumSequencer::DrumSequencer()
{
    for (int i = 0; i < DRUM_TRACK_COUNT; ++i)
    {
        tracks[i].name = "Track " + juce::String (i + 1);
    }
    formatManager.registerBasicFormats();
}

DrumSequencer::~DrumSequencer() {}

void DrumSequencer::prepare (double sr, int spb)
{
    sampleRate = (sr > 0.0 && sr <= 1000000.0) ? sr : 44100.0;
    samplesPerBlock = (spb > 0) ? spb : 512;

    for (auto& v : voices)
        v.voice.prepare (sampleRate);

    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        trackFX[t].prepare (sampleRate, samplesPerBlock);
        trackBufs[t].setSize (2, samplesPerBlock);
    }

    updateSamplesPerStep();
}

void DrumSequencer::process (juce::AudioBuffer<float>& mainBuffer, int numSamples)
{
    // Always clear per-track buffers so bus routing sees silence when stopped
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
        trackBufs[t].clear();

    if (!playing.load())
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter >= currentEffectiveSamplesPerStep)
        {
            sampleCounter -= currentEffectiveSamplesPerStep;
            handleStep (currentStep.load());
            int nextStep = (currentStep.load() + 1) % DRUM_STEPS;
            currentStep.store (nextStep);

            bool nextIsOdd = (nextStep % 2) != 0;
            double factor = nextIsOdd ? (1.0 + swing * 0.33) : (1.0 - swing * 0.33);
            currentEffectiveSamplesPerStep = juce::jmax (1.0, samplesPerStep * factor);
        }

        for (auto& av : voices)
        {
            if (!av.inUse) continue;

            if (av.voice.isActive())
            {
                float s = av.voice.process();
                int t = av.trackIndex;
                if (t >= 0 && t < DRUM_TRACK_COUNT)
                {
                    trackBufs[t].getWritePointer (0)[i] += s;
                    trackBufs[t].getWritePointer (1)[i] += s;
                    float absS = std::abs (s);
                    float prev = trackLevels[t].load (std::memory_order_relaxed);
                    if (absS > prev)
                        trackLevels[t].store (absS, std::memory_order_relaxed);
                }
            }
            else
            {
                av.inUse = false;
            }
        }

        for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
        {
            float lv = trackLevels[t].load (std::memory_order_relaxed);
            if (lv > 0.0f)
                trackLevels[t].store (lv * 0.9999f, std::memory_order_relaxed);
        }

        ++sampleCounter;
    }

    // Apply per-track FX then mix bus-0 tracks into mainBuffer
    auto* mainL = mainBuffer.getWritePointer (0);
    auto* mainR = mainBuffer.getNumChannels() > 1 ? mainBuffer.getWritePointer (1) : mainL;

    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        trackFX[t].processBlock (trackBufs[t], numSamples);

        if (trackOutputBus[t] == 0)
        {
            const auto* tL = trackBufs[t].getReadPointer (0);
            const auto* tR = trackBufs[t].getReadPointer (1);
            for (int i = 0; i < numSamples; ++i)
            {
                mainL[i] += tL[i];
                mainR[i] += tR[i];
            }
        }
    }
}

void DrumSequencer::start()
{
    currentStep.store (0);
    sampleCounter = samplesPerStep;
    currentEffectiveSamplesPerStep = samplesPerStep;
    playing.store (true);
}

void DrumSequencer::stop()
{
    playing.store (false);
    currentStep.store (0);
    sampleCounter = 0.0;

    for (auto& av : voices)
    {
        av.voice.forceStop();
        av.inUse = false;
    }
}

void DrumSequencer::setBPM (float newBPM)
{
    bpm = juce::jlimit (40.0f, 240.0f, newBPM);
    updateSamplesPerStep();
}

void DrumSequencer::updateSamplesPerStep()
{
    double beatsPerSecond = bpm / 60.0;
    double stepsPerSecond = beatsPerSecond * quantStepsPerBeat;
    samplesPerStep = sampleRate / stepsPerSecond;
    currentEffectiveSamplesPerStep = samplesPerStep;
}

void DrumSequencer::handleStep (int step)
{
    for (int track = 0; track < DRUM_TRACK_COUNT; ++track)
    {
        auto& trackData = currentPattern.tracks[track][step];
        if (trackData.active)
        {
            triggerTrack (track, trackData.velocity);
        }
    }
}

void DrumSequencer::triggerTrack (int track, float velocity)
{
    if (tracks[track].sampleBuffer.getNumSamples() == 0)
        return;

    if (tracks[track].muted)
        return;

    if (anySoloActive() && !tracks[track].solo)
        return;

    ActiveVoice* freeVoice = nullptr;
    for (auto& av : voices)
    {
        if (!av.inUse)
        {
            freeVoice = &av;
            break;
        }
    }

    if (freeVoice == nullptr)
    {
        for (auto& av : voices)
        {
            if (av.trackIndex == track)
            {
                freeVoice = &av;
                break;
            }
        }
    }

    if (freeVoice == nullptr)
    {
        for (auto& av : voices)
        {
            freeVoice = &av;
            break;
        }
    }

    if (freeVoice != nullptr)
    {
        freeVoice->voice.trigger (tracks[track], velocity);
        freeVoice->trackIndex = track;
        freeVoice->inUse = true;
    }
}

bool DrumSequencer::anySoloActive() const
{
    for (const auto& t : tracks)
        if (t.solo) return true;
    return false;
}

void DrumSequencer::setStepActive (int track, int step, bool active)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT && step >= 0 && step < DRUM_STEPS)
        currentPattern.tracks[track][step].active = active;
}

bool DrumSequencer::getStepActive (int track, int step) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT && step >= 0 && step < DRUM_STEPS)
        return currentPattern.tracks[track][step].active;
    return false;
}

void DrumSequencer::setStepVelocity (int track, int step, float velocity)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT && step >= 0 && step < DRUM_STEPS)
        currentPattern.tracks[track][step].velocity = juce::jlimit (0.0f, 1.0f, velocity);
}

float DrumSequencer::getStepVelocity (int track, int step) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT && step >= 0 && step < DRUM_STEPS)
        return currentPattern.tracks[track][step].velocity;
    return 1.0f;
}

void DrumSequencer::savePattern (int slot)
{
    if (slot >= 0 && slot < MAX_DRUM_PATTERNS)
        savedPatterns[slot] = currentPattern;
}

void DrumSequencer::loadPattern (int slot)
{
    if (slot >= 0 && slot < MAX_DRUM_PATTERNS)
    {
        currentPattern = savedPatterns[slot];
        currentPatternSlot = slot;
    }
}

void DrumSequencer::setTrackName (int track, const juce::String& name)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        tracks[track].name = name;
}

juce::String DrumSequencer::getTrackName (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return tracks[track].name;
    return {};
}

void DrumSequencer::setTrackVolume (int track, float volume)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        tracks[track].volume = juce::jlimit (0.0f, 1.0f, volume);
}

float DrumSequencer::getTrackVolume (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return tracks[track].volume;
    return 1.0f;
}

void DrumSequencer::setTrackMuted (int track, bool muted)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        tracks[track].muted = muted;
}

bool DrumSequencer::getTrackMuted (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return tracks[track].muted;
    return false;
}

void DrumSequencer::setTrackSolo (int track, bool solo)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        tracks[track].solo = solo;
}

bool DrumSequencer::getTrackSolo (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return tracks[track].solo;
    return false;
}

void DrumSequencer::setTrackRootNote (int track, int note)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        tracks[track].rootNote = juce::jlimit (0, 127, note);
}

int DrumSequencer::getTrackRootNote (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return tracks[track].rootNote;
    return 36;
}

void DrumSequencer::syncToStep (int step)
{
    int targetStep = (step % DRUM_STEPS + DRUM_STEPS) % DRUM_STEPS;
    currentStep.store(targetStep);
    sampleCounter = samplesPerStep;
}

void DrumSequencer::syncToDAWPosition (double ppqStepPos)
{
    const int stepIndex = ((static_cast<int>(std::floor(ppqStepPos)) % DRUM_STEPS) + DRUM_STEPS) % DRUM_STEPS;
    const double phase = ppqStepPos - std::floor(ppqStepPos);

    if (phase < 1e-6)
    {
        currentStep.store(stepIndex);
        sampleCounter = samplesPerStep;
    }
    else
    {
        currentStep.store((stepIndex + 1) % DRUM_STEPS);
        sampleCounter = phase * samplesPerStep;
    }
}

bool DrumSequencer::loadSample (int track, const juce::File& file)
{
    if (track < 0 || track >= DRUM_TRACK_COUNT)
        return false;

    if (!file.existsAsFile())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    auto numSamples = static_cast<int> (reader->lengthInSamples);
    tracks[track].sampleBuffer.setSize (1, numSamples);
    reader->read (&tracks[track].sampleBuffer, 0, numSamples, 0, true, false);
    tracks[track].sampleRate = reader->sampleRate;
    tracks[track].name = file.getFileNameWithoutExtension();

    return true;
}

void DrumSequencer::clearTrackSample (int track)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
    {
        tracks[track].sampleBuffer.clear();
        tracks[track].sampleRate = 44100.0;
    }
}

float DrumSequencer::getTrackLevel (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return trackLevels[track].load();
    return 0.0f;
}

void DrumSequencer::setSwing (float s)
{
    swing = juce::jlimit (0.0f, 1.0f, s);
}

void DrumSequencer::setStepsPerBeat (float spb)
{
    quantStepsPerBeat = juce::jlimit (0.25f, 16.0f, spb);
    updateSamplesPerStep();
}

void DrumSequencer::copyCurrentPattern()
{
    clipboardPattern = currentPattern;
    clipboardValid = true;
}

void DrumSequencer::pasteToCurrentPattern()
{
    if (clipboardValid)
        currentPattern = clipboardPattern;
}

const juce::AudioBuffer<float>& DrumSequencer::getTrackSampleBuffer (int track) const
{
    static const juce::AudioBuffer<float> empty;
    if (track < 0 || track >= DRUM_TRACK_COUNT) return empty;
    return tracks[track].sampleBuffer;
}

LayerEffectChain& DrumSequencer::getTrackFX (int track)
{
    return trackFX[juce::jlimit (0, DRUM_TRACK_COUNT - 1, track)];
}

void DrumSequencer::setTrackOutputBus (int track, int bus)
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        trackOutputBus[track] = bus;
}

int DrumSequencer::getTrackOutputBus (int track) const
{
    if (track >= 0 && track < DRUM_TRACK_COUNT)
        return trackOutputBus[track];
    return 0;
}

const juce::AudioBuffer<float>& DrumSequencer::getTrackBuffer (int track) const
{
    static const juce::AudioBuffer<float> empty;
    if (track < 0 || track >= DRUM_TRACK_COUNT) return empty;
    return trackBufs[track];
}

juce::ValueTree DrumSequencer::getState() const
{
    juce::ValueTree v("DrumSequencer");
    v.setProperty("bpm", bpm, nullptr);
    v.setProperty("currentPatternSlot", currentPatternSlot, nullptr);
    v.setProperty("sampleRate", sampleRate, nullptr);
    v.setProperty("swing", swing, nullptr);
    v.setProperty("stepsPerBeat", quantStepsPerBeat, nullptr);

    // Tracks
    juce::ValueTree tracksNode("Tracks");
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        juce::ValueTree trackNode("Track");
        trackNode.setProperty("index", t, nullptr);
        trackNode.setProperty("name", tracks[t].name, nullptr);
        trackNode.setProperty("volume", tracks[t].volume, nullptr);
        trackNode.setProperty("rootNote", tracks[t].rootNote, nullptr);
        trackNode.setProperty("muted", tracks[t].muted, nullptr);
        trackNode.setProperty("solo", tracks[t].solo, nullptr);
        trackNode.setProperty("outputBus", trackOutputBus[t], nullptr);
        trackNode.appendChild(trackFX[t].getState(), nullptr);
        tracksNode.appendChild(trackNode, nullptr);
    }
    v.appendChild(tracksNode, nullptr);

    // Current pattern steps
    juce::ValueTree patternNode("CurrentPattern");
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
    {
        for (int s = 0; s < DRUM_STEPS; ++s)
        {
            const auto& step = currentPattern.tracks[t][s];
            juce::ValueTree stepNode("Step");
            stepNode.setProperty("track", t, nullptr);
            stepNode.setProperty("step", s, nullptr);
            stepNode.setProperty("active", step.active, nullptr);
            stepNode.setProperty("velocity", step.velocity, nullptr);
            patternNode.appendChild(stepNode, nullptr);
        }
    }
    v.appendChild(patternNode, nullptr);

    // Saved patterns
    juce::ValueTree savedNode("SavedPatterns");
    for (int slot = 0; slot < MAX_DRUM_PATTERNS; ++slot)
    {
        juce::ValueTree patNode("Pattern");
        patNode.setProperty("slot", slot, nullptr);
        for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
        {
            for (int s = 0; s < DRUM_STEPS; ++s)
            {
                const auto& step = savedPatterns[slot].tracks[t][s];
                juce::ValueTree stepNode("Step");
                stepNode.setProperty("track", t, nullptr);
                stepNode.setProperty("step", s, nullptr);
                stepNode.setProperty("active", step.active, nullptr);
                stepNode.setProperty("velocity", step.velocity, nullptr);
                patNode.appendChild(stepNode, nullptr);
            }
        }
        savedNode.appendChild(patNode, nullptr);
    }
    v.appendChild(savedNode, nullptr);

    return v;
}

void DrumSequencer::setState(const juce::ValueTree& state)
{
    if (!state.hasType("DrumSequencer"))
        return;

    if (state.hasProperty("bpm"))
        setBPM((float)state.getProperty("bpm"));
    if (state.hasProperty("currentPatternSlot"))
        currentPatternSlot = (int)state.getProperty("currentPatternSlot");
    if (state.hasProperty("swing"))
        swing = juce::jlimit(0.0f, 1.0f, (float)state.getProperty("swing"));
    if (state.hasProperty("stepsPerBeat"))
        setStepsPerBeat((float)state.getProperty("stepsPerBeat"));

    // Load tracks
    auto tracksNode = state.getChildWithName("Tracks");
    if (tracksNode.isValid())
    {
        for (auto trackNode : tracksNode)
        {
            if (trackNode.hasType("Track"))
            {
                int idx = (int)trackNode.getProperty("index");
                if (idx >= 0 && idx < DRUM_TRACK_COUNT)
                {
                    tracks[idx].name = trackNode.getProperty("name").toString();
                    tracks[idx].volume = (float)trackNode.getProperty("volume");
                    tracks[idx].rootNote = (int)trackNode.getProperty("rootNote");
                    tracks[idx].muted = (bool)trackNode.getProperty("muted", false);
                    tracks[idx].solo = (bool)trackNode.getProperty("solo", false);
                    trackOutputBus[idx] = (int)trackNode.getProperty("outputBus", 0);
                    auto fxState = trackNode.getChildWithName("LayerFX");
                    if (fxState.isValid()) trackFX[idx].setState(fxState);
                }
            }
        }
    }

    // Clear current pattern before loading
    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
        for (int s = 0; s < DRUM_STEPS; ++s)
        {
            currentPattern.tracks[t][s].active = false;
            currentPattern.tracks[t][s].velocity = 1.0f;
        }
    auto patternNode = state.getChildWithName("CurrentPattern");
    if (patternNode.isValid())
    {
        for (auto stepNode : patternNode)
        {
            if (stepNode.hasType("Step"))
            {
                int t = (int)stepNode.getProperty("track");
                int s = (int)stepNode.getProperty("step");
                if (t >= 0 && t < DRUM_TRACK_COUNT && s >= 0 && s < DRUM_STEPS)
                {
                    currentPattern.tracks[t][s].active = (bool)stepNode.getProperty("active");
                    currentPattern.tracks[t][s].velocity = (float)stepNode.getProperty("velocity");
                }
            }
        }
    }

    // Load saved patterns
    auto savedNode = state.getChildWithName("SavedPatterns");
    if (savedNode.isValid())
    {
        for (auto patNode : savedNode)
        {
            if (patNode.hasType("Pattern"))
            {
                int slot = (int)patNode.getProperty("slot");
                if (slot >= 0 && slot < MAX_DRUM_PATTERNS)
                {
                    auto& pat = savedPatterns[slot];
                    for (int t = 0; t < DRUM_TRACK_COUNT; ++t)
                        for (int s = 0; s < DRUM_STEPS; ++s)
                        {
                            pat.tracks[t][s].active = false;
                            pat.tracks[t][s].velocity = 1.0f;
                        }
                    for (auto stepNode : patNode)
                    {
                        if (stepNode.hasType("Step"))
                        {
                            int t = (int)stepNode.getProperty("track");
                            int s = (int)stepNode.getProperty("step");
                            if (t >= 0 && t < DRUM_TRACK_COUNT && s >= 0 && s < DRUM_STEPS)
                            {
                                pat.tracks[t][s].active = (bool)stepNode.getProperty("active");
                                pat.tracks[t][s].velocity = (float)stepNode.getProperty("velocity");
                            }
                        }
                    }
                }
            }
        }
    }
}
