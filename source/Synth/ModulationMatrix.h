#pragma once
#include <array>
#include <vector>
#include <functional>
#include <cstdint>
#include <atomic>
#include "JuceHeader.h"

constexpr int MAX_MOD_SOURCES = 21;
constexpr int MAX_MOD_TARGETS = 26;
constexpr int MAX_MOD_CONNECTIONS = 32;

enum class ModSourceType : uint8_t
{
    LFO1,
    LFO2,
    LFO3,
    LFO4,
    Envelope,
    Velocity,
    NoteNumber,
    Aftertouch,
    ModWheel,
    PitchBend,
    Random,
    EnvelopeFollower,
    MPEPressure,
    MPESlide,
    SequencerStep,
    LFO5,
    LFO6,
    LFO7,
    LFO8,
    Envelope2,
    Envelope3
};

enum class ModTargetType : uint8_t
{
    OscillatorPitch,
    OscillatorLevel,
    OscillatorWaveform,
    FilterCutoff,
    FilterResonance,
    AmpVolume,
    AmpPan,
    LFO1Rate,
    LFO2Rate,
    LFO3Rate,
    LFO4Rate,
    EffectParam1,
    EffectParam2,
    EffectParam3,
    EffectMix,
    GranularPosition,
    GranularDensity,
    GranularGrainSize,
    GranularSpray,
    GranularPitchScatter,
    OscShapeAmount,
    OscPhaseDistAmount,
    LFO5Rate,
    LFO6Rate,
    LFO7Rate,
    LFO8Rate
};

enum class LFOShape : uint8_t
{
    Sine,
    Triangle,
    Saw,
    Square,
    SampleAndHold
};

struct ModConnection
{
    ModSourceType source = ModSourceType::LFO1;
    ModTargetType target = ModTargetType::FilterCutoff;
    float amount = 0.0f;
    bool enabled = true;
    int sourceIndex = 0;
    int targetIndex = 0;
};

class ModulationMatrix
{
public:
    ModulationMatrix();

    int addConnection(ModSourceType source, ModTargetType target, float amount);
    void removeConnection(int connectionId);
    void updateConnection(int connectionId, float amount);
    void updateConnectionSource(int connectionId, ModSourceType source, int index);
    void updateConnectionTarget(int connectionId, ModTargetType target, int index);
    void setConnectionEnabled(int connectionId, bool enabled);

    float getModulationValue(ModSourceType source, int index);
    void setModulationValue(ModSourceType source, int index, float value);

    void prepare(double sampleRate, int samplesPerBlock);
    void prepareForBlock();

    // LFO control (index 0-7 for LFO1-LFO8)
    void setLFORate(int lfoIndex, float rateHz);
    float getLFORate(int lfoIndex) const;
    void setLFOShape(int lfoIndex, LFOShape shape);
    LFOShape getLFOShape(int lfoIndex) const;

    // Advance all LFOs by given number of samples and update sourceValues
    void advanceLFOs(int numSamples = 1);

    // Compute modulation sums for all targets based on current source values
    // outSums must point to array of size at least MAX_MOD_TARGETS
    void computeModulationSums(float* outSums) const;

    // Deprecated: returns copy with lock protection
    std::vector<ModConnection> getConnections() const;
    std::vector<ModConnection> getActiveConnectionsForTarget(ModTargetType target, int index) const;

    // State persistence
    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& state);

    ModSourceType getSourceType(int index) const;
    ModTargetType getTargetType(int index) const;

    int getSourceCount() const { return MAX_MOD_SOURCES; }
    int getTargetCount() const { return MAX_MOD_TARGETS; }

    static constexpr const char* getSourceName(ModSourceType type)
    {
        switch (type)
        {
            case ModSourceType::LFO1:            return "LFO 1";
            case ModSourceType::LFO2:            return "LFO 2";
            case ModSourceType::LFO3:            return "LFO 3";
            case ModSourceType::LFO4:            return "LFO 4";
            case ModSourceType::Envelope:        return "Envelope";
            case ModSourceType::Velocity:        return "Velocity";
            case ModSourceType::NoteNumber:      return "Note Number";
            case ModSourceType::Aftertouch:      return "Aftertouch";
            case ModSourceType::ModWheel:        return "Mod Wheel";
            case ModSourceType::PitchBend:       return "Pitch Bend";
            case ModSourceType::Random:          return "Random";
            case ModSourceType::EnvelopeFollower:return "Env Follower";
            case ModSourceType::MPEPressure:     return "MPE Pressure";
            case ModSourceType::MPESlide:        return "MPE Slide";
            case ModSourceType::SequencerStep:   return "Seq Step";
            case ModSourceType::LFO5:            return "LFO 5";
            case ModSourceType::LFO6:            return "LFO 6";
            case ModSourceType::LFO7:            return "LFO 7";
            case ModSourceType::LFO8:            return "LFO 8";
            case ModSourceType::Envelope2:       return "Env 2";
            case ModSourceType::Envelope3:       return "Env 3";
        }
        return "";
    }

    static constexpr const char* getTargetName(ModTargetType type)
    {
        switch (type)
        {
            case ModTargetType::OscillatorPitch:     return "OSC Pitch";
            case ModTargetType::OscillatorLevel:     return "OSC Level";
            case ModTargetType::OscillatorWaveform:  return "OSC Wave";
            case ModTargetType::FilterCutoff:        return "Filter Cutoff";
            case ModTargetType::FilterResonance:     return "Filter Resonance";
            case ModTargetType::AmpVolume:           return "Volume";
            case ModTargetType::AmpPan:              return "Pan";
            case ModTargetType::LFO1Rate:            return "LFO 1 Rate";
            case ModTargetType::LFO2Rate:            return "LFO 2 Rate";
            case ModTargetType::LFO3Rate:            return "LFO 3 Rate";
            case ModTargetType::LFO4Rate:            return "LFO 4 Rate";
            case ModTargetType::EffectParam1:        return "Effect Param 1";
            case ModTargetType::EffectParam2:        return "Effect Param 2";
            case ModTargetType::EffectParam3:        return "Effect Param 3";
            case ModTargetType::EffectMix:           return "Effect Mix";
            case ModTargetType::GranularPosition:    return "Gran Position";
            case ModTargetType::GranularDensity:     return "Gran Density";
            case ModTargetType::GranularGrainSize:   return "Gran Grain Size";
            case ModTargetType::GranularSpray:       return "Gran Spray";
            case ModTargetType::GranularPitchScatter:return "Gran Pitch Scatter";
            case ModTargetType::OscShapeAmount:      return "OSC Shape Amt";
            case ModTargetType::OscPhaseDistAmount:  return "OSC PhaseDist Amt";
            case ModTargetType::LFO5Rate:            return "LFO 5 Rate";
            case ModTargetType::LFO6Rate:            return "LFO 6 Rate";
            case ModTargetType::LFO7Rate:            return "LFO 7 Rate";
            case ModTargetType::LFO8Rate:            return "LFO 8 Rate";
        }
        return "";
    }

private:
    struct SourceValue
    {
        ModSourceType type;
        int index = 0;
        std::atomic<float> value{0.0f};
        float phase = 0.0f;
        bool keyDown = false;
    };

    std::vector<ModConnection> connections;
    std::array<SourceValue, MAX_MOD_SOURCES> sourceValues;

    // Per-LFO state (indices 0-7 = LFO1-LFO8)
    std::array<std::atomic<float>, 8> lfoPhase;
    std::array<float, 8> lfoRate;
    std::array<LFOShape, 8> lfoShape;
    std::array<float, 8> lfoSHValue;

    int nextConnectionId = 0;

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;

    juce::CriticalSection connectionLock;

    float modSumsBuffer[2][MAX_MOD_TARGETS];
    std::atomic<int> currentModSumsBuffer{0};

    void swapModSumsBuffers();
};
