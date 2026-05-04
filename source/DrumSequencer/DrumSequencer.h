#pragma once
#include <JuceHeader.h>
#include "../Layers/LayerEffectChain.h"

constexpr int DRUM_TRACK_COUNT = 8;
constexpr int DRUM_STEPS = 16;
constexpr int MAX_DRUM_PATTERNS = 8;

struct DrumStep
{
    bool active = false;
    float velocity = 1.0f;
};

struct DrumPattern
{
    std::array<std::array<DrumStep, DRUM_STEPS>, DRUM_TRACK_COUNT> tracks {};
};

struct DrumTrack
{
    juce::String name = "Track";
    juce::AudioBuffer<float> sampleBuffer;
    double sampleRate = 44100.0;
    int rootNote = 36;
    float volume = 1.0f;
    bool muted = false;
    bool solo = false;
};

class DrumVoice
{
public:
    DrumVoice();
    void prepare (double sr);
    void trigger (const DrumTrack& track, float velocity);
    void release();
    float process();
    bool isActive() const { return playing; }
    void forceStop();

private:
    double sampleRate = 44100.0;
    double position = 0.0;
    double speed = 1.0;
    float volume = 1.0f;
    bool playing = false;
    const float* sampleData = nullptr;
    int sampleLength = 0;
};

class DrumSequencer
{
public:
    DrumSequencer();
    ~DrumSequencer();

    void prepare (double sampleRate, int samplesPerBlock);
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

    void start();
    void stop();
    bool isPlaying() const { return playing.load(); }

    void setBPM (float newBPM);
    float getBPM() const { return bpm; }

    void setStepActive (int track, int step, bool active);
    bool getStepActive (int track, int step) const;
    void setStepVelocity (int track, int step, float velocity);
    float getStepVelocity (int track, int step) const;

    int getCurrentStep() const { return currentStep.load(); }

    void savePattern (int slot);
    void loadPattern (int slot);
    int getCurrentPatternSlot() const { return currentPatternSlot; }

    void setTrackName (int track, const juce::String& name);
    juce::String getTrackName (int track) const;
    void setTrackVolume (int track, float volume);
    float getTrackVolume (int track) const;
    void setTrackMuted (int track, bool muted);
    bool getTrackMuted (int track) const;
    void setTrackSolo (int track, bool solo);
    bool getTrackSolo (int track) const;
    void setTrackRootNote (int track, int note);
    int getTrackRootNote (int track) const;

    void syncToStep (int step);
    void syncToDAWPosition (double ppqStepPos);

    bool loadSample (int track, const juce::File& file);
    void clearTrackSample (int track);

    float getTrackLevel (int track) const;

    DrumPattern& getCurrentPattern() { return currentPattern; }

    // Swing (0.0 = no swing, 1.0 = max ~33% delay on odd steps)
    void setSwing (float s);
    float getSwing() const { return swing; }

    // Step resolution (steps per beat: 1=1/4, 2=1/8, 4=1/16, 8=1/32)
    void setStepsPerBeat (float spb);
    float getStepsPerBeat() const { return quantStepsPerBeat; }

    // Copy / paste current pattern
    void copyCurrentPattern();
    void pasteToCurrentPattern();
    bool hasPatternInClipboard() const { return clipboardValid; }

    // Sample buffer access for waveform preview (UI thread only)
    const juce::AudioBuffer<float>& getTrackSampleBuffer (int track) const;

    // Per-track FX chain
    LayerEffectChain& getTrackFX (int track);

    // Per-track output bus (0 = main mix, 9-16 = individual drum buses)
    void setTrackOutputBus (int track, int bus);
    int  getTrackOutputBus (int track) const;

    // Per-track rendered buffer (valid after process(); audio thread writes, processor reads)
    const juce::AudioBuffer<float>& getTrackBuffer (int track) const;

    // State persistence
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

 private:
    struct ActiveVoice
    {
        DrumVoice voice;
        int trackIndex = 0;
        bool inUse = false;
    };

    void triggerTrack (int track, float velocity);
    void updateSamplesPerStep();
    void handleStep (int step);
    bool anySoloActive() const;
    float mixVoices (float sample, int track);

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;
    float bpm = 120.0f;
    double samplesPerStep = 0.0;
    double sampleCounter = 0.0;

    std::atomic<int> currentStep { 0 };
    std::atomic<bool> playing { false };

    DrumPattern currentPattern;
    std::array<DrumPattern, MAX_DRUM_PATTERNS> savedPatterns {};
    int currentPatternSlot = 0;

    std::array<DrumTrack, DRUM_TRACK_COUNT> tracks;
    std::array<ActiveVoice, DRUM_TRACK_COUNT * 4> voices;

    juce::AudioFormatManager formatManager;

    std::atomic<float> trackLevels[DRUM_TRACK_COUNT] = {};

    // Per-track FX, output bus routing, and rendered buffers
    std::array<LayerEffectChain, DRUM_TRACK_COUNT> trackFX;
    int trackOutputBus[DRUM_TRACK_COUNT] = {};          // 0=main, 9-16=individual buses
    std::array<juce::AudioBuffer<float>, DRUM_TRACK_COUNT> trackBufs;

    float swing = 0.0f;
    float quantStepsPerBeat = 4.0f;
    double currentEffectiveSamplesPerStep = 0.0;

    DrumPattern clipboardPattern;
    bool clipboardValid = false;
};