#pragma once
#include "Voice.h"
#include "FMVoice.h"
#include <array>

constexpr int MAX_VOICES = 16;
constexpr int MAX_FM_VOICES = 8;

enum class SynthMode
{
    Classic,
    FM
};

class SynthEngine
{
public:
    SynthEngine();

    void prepare(double sampleRate, int samplesPerBlock);

    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void allNotesOff();

    void setMasterVolume(float volume);
    void setPitchBend(float cents);

    // Classic mode controls
    void setEnvelopeParams(float a, float d, float s, float r);
    void setFilterParams(float cutoff, float resonance);
    void setWaveform(WaveformType type);

    // FM mode controls
    void setSynthMode(SynthMode mode);
    SynthMode getSynthMode() const { return synthMode; }
    void setFMAlgorithm(int index);
    void setFMOperatorParams(int opIndex,
                             float ratio,
                             float level,
                             float feedback,
                             float attack,
                             float decay,
                             float sustain,
                             float release);

    float process();

    int getActiveVoiceCount() const;

private:
    struct VoiceInfo
    {
        Voice voice;
        bool inUse = false;
        int lastUseTime = 0;
    };

    struct FMVoiceInfo
    {
        FMVoice voice;
        bool inUse = false;
        int lastUseTime = 0;
    };

    VoiceInfo* findFreeVoice();
    VoiceInfo* findVoiceForNote(int midiNote);
    FMVoiceInfo* findFreeFMVoice();
    FMVoiceInfo* findFMVoiceForNote(int midiNote);

    std::array<VoiceInfo, MAX_VOICES> voices;
    std::array<FMVoiceInfo, MAX_FM_VOICES> fmVoices;

    SynthMode synthMode = SynthMode::Classic;
    int fmAlgorithmIndex = 0;

    float masterVolume = 0.5f;
    float sampleRate = 44100.0f;
    double pitchBend = 0.0;
    int voiceCounter = 0;
};