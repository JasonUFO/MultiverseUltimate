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
    int getFMAlgorithm() const { return fmAlgorithmIndex; }
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

    // Getters for state persistence
    void getEnvelopeParams(float& a, float& d, float& s, float& r) const;
    WaveformType getWaveform() const;
    void getFMOperatorParams(int opIndex,
                             float& ratio,
                             float& level,
                             float& feedback,
                             float& attack,
                             float& decay,
                             float& sustain,
                             float& release) const;

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

    // Envelope parameters (Classic mode)
    float envAttack = 0.01f;
    float envDecay = 0.1f;
    float envSustain = 0.7f;
    float envRelease = 0.3f;

    // Waveform for classic oscillators
    WaveformType waveform = WaveformType::Saw;

    // FM operator parameters (4 operators)
    struct FMOperatorSettings
    {
        float ratio = 1.0f;
        float level = 1.0f;
        float feedback = 0.0f;
        float attack = 0.01f;
        float decay = 0.1f;
        float sustain = 0.7f;
        float release = 0.3f;
    };
    std::array<FMOperatorSettings, 4> fmOpParams;

    float masterVolume = 0.5f;
    float sampleRate = 44100.0f;
    double pitchBend = 0.0;
    int voiceCounter = 0;
};