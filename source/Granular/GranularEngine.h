#pragma once
#include <JuceHeader.h>
#include "GranularVoice.h"

static constexpr int MAX_GRANULAR_VOICES = 16;

class GranularEngine
{
public:
    GranularEngine();

    void prepare (double sampleRate, int samplesPerBlock);

    void noteOn  (int midiNote, float velocity);
    void noteOff (int midiNote);
    void allNotesOff();

    // Main process: adds granular audio into buffer (stereo, 2 channels)
    void processBuffer (juce::AudioBuffer<float>& buffer, int numSamples);

    // File loading — call from message thread only
    bool loadSourceFile (const juce::File& file);
    juce::String getSourceFilePath() const { return sourceFilePath; }

    juce::ValueTree getState() const;
    void setState (const juce::ValueTree& tree);

    // Per-block parameter setters (called from processBlock, audio-thread safe)
    void setPosition      (float v) noexcept { position.store(v);       }
    void setGrainSize     (float v) noexcept { grainSize.store(v);      }
    void setSpray         (float v) noexcept { spray.store(v);          }
    void setDensity       (float v) noexcept { density.store(v);        }
    void setPitchScatter  (float v) noexcept { pitchScatter.store(v);   }
    void setEnvShape      (int   v) noexcept { envShape.store(v);       }
    void setReverse       (bool  v) noexcept { reverse.store(v);        }
    void setStereoSpread  (float v) noexcept { stereoSpread.store(v);   }
    void setAttack        (float v) noexcept { attack.store(v);         }
    void setDecay         (float v) noexcept { decay.store(v);          }
    void setSustain       (float v) noexcept { sustain.store(v);        }
    void setRelease       (float v) noexcept { release.store(v);        }
    void setMasterVolume  (float v) noexcept { masterVolume.store(v);   }

private:
    GranularVoice voices[MAX_GRANULAR_VOICES];

    double sampleRate   = 44100.0;
    int    blockSize    = 512;

    // Source buffer (pre-allocated; populated in prepare() or on file load)
    juce::AudioBuffer<float> sourceBuffer;
    juce::CriticalSection    sourceLock;
    juce::String             sourceFilePath;

    void buildDefaultSourceBuffer();

    // Atomic params (written audio-thread from setters, read in processBuffer)
    std::atomic<float> position      { 0.0f };
    std::atomic<float> grainSize     { 0.08f };
    std::atomic<float> spray         { 0.1f };
    std::atomic<float> density       { 12.0f };
    std::atomic<float> pitchScatter  { 0.0f };
    std::atomic<int>   envShape      { 1 };    // Hann default
    std::atomic<bool>  reverse       { false };
    std::atomic<float> stereoSpread  { 0.5f };
    std::atomic<float> attack        { 0.01f };
    std::atomic<float> decay         { 0.1f };
    std::atomic<float> sustain       { 0.7f };
    std::atomic<float> release       { 0.5f };
    std::atomic<float> masterVolume  { 0.8f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GranularEngine)
};
