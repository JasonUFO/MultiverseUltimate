#pragma once
#include "SamplerZone.h"
#include <vector>

class MvSamplerVoice
{
public:
    void prepare (double sampleRate);
    void noteOn (const SamplerZone* zone, int midiNote, float velocity);
    void noteOff();
    void forceStop();
    bool isActive() const noexcept { return active; }
    float process();
    int getMidiNote() const noexcept { return midiNote; }

private:
    float getSampleAt (double pos) const noexcept;
    float processTimestretch();

    const SamplerZone* currentZone = nullptr;
    double readPos = 0.0;
    double playbackRate = 1.0;
    bool pingPongForward = true;

    // Timestretch (WSOLA) state
    bool timestretchActive = false;
    double pitchRate = 1.0;   // pitch ratio (for resampling)
    double timeRate = 1.0;    // speed ratio (for WSOLA grain placement)

    // WSOLA buffers (pre-allocated in noteOn, no alloc in process)
    std::vector<float> grainBuffer;
    std::vector<float> outputBuffer;
    std::vector<float> hannWindow;
    int grainSize = 0;        // samples per grain
    int hopSynthesis = 0;     // output hop size
    int hopAnalysis = 0;      // input hop size
    int outputReadPos = 0;    // read position in output ring
    int outputWritePos = 0;   // write position in output ring
    int grainsReady = 0;      // number of complete grains in output
    static constexpr int WSOLA_SEARCH = 512;  // cross-correlation search window
    static constexpr int OUTPUT_RING_SIZE = 8192; // output ring buffer size

    // Time-based ADSR (increments per sample, computed in noteOn)
    enum class EnvStage { Idle, Attack, Decay, Sustain, Release };
    EnvStage envStage = EnvStage::Idle;
    float envValue = 0.0f;
    float attackInc = 0.0f;
    float decayDec = 0.0f;
    float sustainLevel = 0.7f;
    float releaseDec = 0.0f;

    int midiNote = -1;
    float velocity = 0.0f;
    bool active = false;
    double sampleRate = 44100.0;
};