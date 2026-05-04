#pragma once
#include <JuceHeader.h>

enum class LoopMode
{
    None,
    Forward,
    PingPong
};

struct SamplerZone
{
    juce::String name;
    juce::String filePath;

    // Key and velocity range
    int loNote = 0;
    int hiNote = 127;
    int loVel = 0;    // 0-127
    int hiVel = 127;
    int rootNote = 60;

    // Audio data
    juce::AudioBuffer<float> audioData;
    double fileSampleRate = 44100.0;

    // Pitch and speed (independent controls)
    float tuning = 0.0f;   // semitones offset, ±24
    float speed  = 1.0f;   // playback speed multiplier, 0.25–4.0

    // Loop settings (sample positions within audioData)
    int loopStart = 0;
    int loopEnd = 0;           // 0 means use full buffer length
    int crossfadeLength = 0;
    LoopMode loopMode = LoopMode::None;

    int getEffectiveLoopEnd() const noexcept
    {
        int total = audioData.getNumSamples();
        return (loopEnd > 0 && loopEnd <= total) ? loopEnd : total;
    }

    bool matchesNote (int midiNote, int velocity) const noexcept
    {
        return midiNote >= loNote && midiNote <= hiNote
            && velocity >= loVel && velocity <= hiVel;
    }
};
