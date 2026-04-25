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

    updateSamplesPerStep();
}

void DrumSequencer::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (!playing.load())
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        if (sampleCounter >= samplesPerStep)
        {
            sampleCounter -= samplesPerStep;
            handleStep (currentStep.load());
            currentStep.store ((currentStep.load() + 1) % DRUM_STEPS);
        }

        float mixL = 0.0f, mixR = 0.0f;
        int activeCount = 0;

        for (auto& av : voices)
        {
            if (!av.inUse)
                continue;

            if (av.voice.isActive())
            {
                float s = av.voice.process();
                mixL += s;
                mixR += s;
                ++activeCount;
            }
            else
            {
                av.inUse = false;
            }
        }

        if (activeCount > 1)
        {
            mixL /= static_cast<float> (activeCount);
            mixR /= static_cast<float> (activeCount);
        }

        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : left;
        if (left != nullptr) left[i] = mixL;
        if (right != nullptr) right[i] = mixR;

        ++sampleCounter;
    }
}

void DrumSequencer::start()
{
    currentStep.store (0);
    sampleCounter = 0.0;
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
    double stepsPerBeat = 4.0;
    double stepsPerSecond = beatsPerSecond * stepsPerBeat;
    samplesPerStep = sampleRate / stepsPerSecond;
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