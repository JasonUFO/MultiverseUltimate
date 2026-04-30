#pragma once
#include "Voice.h"
#include "FMVoice.h"
#include <JuceHeader.h>
#include <array>

constexpr int MAX_VOICES = 16;
constexpr int MAX_FM_VOICES = 8;

enum class SynthMode
{
    Classic,
    FM
};

enum class VoiceMode
{
    Poly,
    Mono,
    Legato
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
    void setOversamplingMode(Filter::OversamplingMode mode);

    // Per-oscillator controls (index 0-2)
    void setOscillatorType(int index, OscillatorType type);
    void setOscillatorLevel(int index, float level);
    void setOscillatorDetune(int index, float detuneSemitones);
    void setOscillatorWaveform(int index, WaveformType wf);
    void setOscillatorWavePosition(int index, float pos);

    // Unison controls
    void setUnisonVoices(int n);
    void setUnisonDetune(float semitones);
    void setUnisonWidth(float w);

    // Voice mode / portamento
    void setVoiceMode(VoiceMode m);
    void setPortamento(float seconds);
    void setPortaAlways(bool always);

    // Wavetable file loading
    bool loadWavetableFile(int oscIndex, const juce::File& file);
    juce::String getWavetableFilePath(int oscIndex) const;

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

    // Process single sample (for backward compatibility)
    float process();

    // Process buffer for efficiency - returns output sample count
    int processBuffer(juce::AudioBuffer<float>& buffer, int numSamples);

    int getActiveVoiceCount() const;

    // Getters for state persistence
    void getEnvelopeParams(float& a, float& d, float& s, float& r) const;
    WaveformType getWaveform() const;

    // Per-oscillator getters
    OscillatorType getOscillatorType(int index) const;
    float getOscillatorLevel(int index) const;
    float getOscillatorDetune(int index) const;
    WaveformType getOscillatorWaveform(int index) const;
    float getOscillatorWavePosition(int index) const;
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
        float unisonDetuneOffset = 0.0f;
        float panLeft  = 1.0f;
        float panRight = 1.0f;
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

    // Per-oscillator state (3 oscillators)
    struct OscSettings
    {
        OscillatorType type = OscillatorType::Classic;
        WaveformType classicWaveform = WaveformType::Saw;
        float level = 1.0f;
        float detuneSemitones = 0.0f;
        float wavePosition = 0.0f; // 0..1 for wavetable scan
    };
    std::array<OscSettings, 3> oscSettings;

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

    // Voice mode / portamento
    VoiceMode voiceMode = VoiceMode::Poly;
    float portaTime = 0.0f;
    bool portaAlways = false;
    float glideCurrentSemitone = 69.0f;
    float glideTargetSemitone  = 69.0f;
    float glideCoeff = 1.0f;

    // Mono note stack (held keys, no heap alloc)
    int monoNoteStack[16] = {};
    int monoNoteCount = 0;
    float monoVelocity = 0.5f;

    // Unison state
    int   unisonVoiceCount       = 1;
    float unisonDetuneSemitones  = 0.2f;
    float unisonWidthAmount      = 1.0f;

    // Wavetable file loading
    juce::AudioFormatManager formatManager;
    juce::String wavetableFilePaths[3];
};