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
    void setFilterType(Filter::FilterType t);

    // Sub oscillator
    void setSubOscEnabled(bool e);
    void setSubOscLevel(float l);
    void setSubOscWaveform(WaveformType wf);

    // Noise oscillator
    void setNoiseOscEnabled(bool e);
    void setNoiseOscLevel(float l);
    void setNoiseOscColor(float hz);

    // Per-oscillator controls (index 0-7)
    void setOscillatorType(int index, OscillatorType type);
    void setOscillatorLevel(int index, float level);
    void setOscillatorDetune(int index, float detuneSemitones);
    void setOscillatorWaveform(int index, WaveformType wf);
    void setOscillatorWavePosition(int index, float pos);
    void setOscillatorShapeType(int index, OscShapeType st);
    void setOscillatorShapeAmount(int index, float amt);
    void setOscillatorSelfOsc(int index, float feedback);
    void setOscillatorPhaseDistAmount(int index, float amt);
    void setOscCount(int n);

    // Unison controls
    void setUnisonVoices(int n);
    void setUnisonDetune(float semitones);
    void setUnisonWidth(float w);

    // Unison spread modes
    enum class UnisonSpreadMode { Stacked, Chord, Random };
    void setUnisonSpreadMode(UnisonSpreadMode m) { unisonSpreadMode = m; }

    // Voice mode / portamento
    void setVoiceMode(VoiceMode m);
    void setPortamento(float seconds);
    void setPortaAlways(bool always);

    // CPU voice limiting (1–MAX_VOICES for classic, 1–MAX_FM_VOICES for FM)
    void setVoiceLimit(int limit);
    int  getVoiceLimit() const { return voiceLimit; }

    // Wavetable file loading
    bool loadWavetableFile(int oscIndex, const juce::File& file);
    juce::String getWavetableFilePath(int oscIndex) const;

    // Wavetable editor access — voice 0 is the master for editing; distributeWavetable copies to all voices
    WavetableOscillator& getWavetableOscillator(int oscIndex) { return voices[0].voice.getWavetableOsc(juce::jlimit(0, 7, oscIndex)); }
    void distributeWavetable(int oscIndex);

    // MPE (MIDI Polyphonic Expression) — Lower Zone: master ch 1, member ch 2–15
    static constexpr float MPE_PITCH_BEND_RANGE = 48.0f;  // semitones, standard MPE
    void setMPEEnabled(bool enabled);
    void noteOnMPE(int channel, int midiNote, float velocity);
    void noteOffMPE(int channel, int midiNote);
    void setMPEPitchBend(int channel, float semitones);
    void setMPEPressure(int channel, float pressure);   // 0..1
    void setMPESlide(int channel, float slide);         // -1..+1 (CC74 centred at 63)

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
    OscShapeType getOscillatorShapeType(int index) const;
    float getOscillatorShapeAmount(int index) const;
    float getOscillatorSelfOsc(int index) const;
    float getOscillatorPhaseDistAmount(int index) const;
    int getOscCount() const { return oscCount; }
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
        int midiChannel = 0;  // 0 = unassigned; 2–15 for MPE member channels
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

    // Per-oscillator state (up to 8 oscillators)
    struct OscSettings
    {
        OscillatorType type = OscillatorType::Classic;
        WaveformType classicWaveform = WaveformType::Saw;
        float level = 1.0f;
        float detuneSemitones = 0.0f;
        float wavePosition = 0.0f;
        OscShapeType shapeType = OscShapeType::Off;
        float shapeAmount = 0.0f;
        float selfOscFeedback = 0.0f;
        float phaseDistAmount = 0.5f;
    };
    int oscCount = 3;
    std::array<OscSettings, 8> oscSettings;

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
    int voiceLimit = MAX_VOICES;

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
    UnisonSpreadMode unisonSpreadMode = UnisonSpreadMode::Stacked;

    // Sub / Noise osc settings (applied to all voices)
    bool  subOscEnabled  = false;
    float subOscLevel    = 0.5f;
    WaveformType subOscWaveform = WaveformType::Sine;
    bool  noiseOscEnabled = false;
    float noiseOscLevel   = 0.3f;
    float noiseOscColor   = 20000.0f;
    Filter::FilterType filterTypeParam = Filter::FilterType::LP;

    // Wavetable file loading
    juce::AudioFormatManager formatManager;
    juce::String wavetableFilePaths[8];

    // MPE per-channel state (index 0 = ch 1, …, 15 = ch 16)
    struct MpeChannelState
    {
        float pitchBendSemitones = 0.0f;
        float pressure = 0.0f;
        float slide = 0.0f;
    };
    MpeChannelState mpeChannels[16];
    bool mpeEnabled = false;
};