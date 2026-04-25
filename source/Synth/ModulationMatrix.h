#pragma once
#include <array>
#include <vector>
#include <functional>
#include <cstdint>

constexpr int MAX_MOD_SOURCES = 16;
constexpr int MAX_MOD_TARGETS = 16;
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
    Random
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
    EffectMix
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

    const std::vector<ModConnection>& getConnections() const { return connections; }
    std::vector<ModConnection> getActiveConnectionsForTarget(ModTargetType target, int index) const;

    ModSourceType getSourceType(int index) const;
    ModTargetType getTargetType(int index) const;

    int getSourceCount() const { return MAX_MOD_SOURCES; }
    int getTargetCount() const { return MAX_MOD_TARGETS; }

    static constexpr const char* getSourceName(ModSourceType type)
    {
        switch (type)
        {
            case ModSourceType::LFO1: return "LFO 1";
            case ModSourceType::LFO2: return "LFO 2";
            case ModSourceType::LFO3: return "LFO 3";
            case ModSourceType::LFO4: return "LFO 4";
            case ModSourceType::Envelope: return "Envelope";
            case ModSourceType::Velocity: return "Velocity";
            case ModSourceType::NoteNumber: return "Note Number";
            case ModSourceType::Aftertouch: return "Aftertouch";
            case ModSourceType::ModWheel: return "Mod Wheel";
            case ModSourceType::PitchBend: return "Pitch Bend";
            case ModSourceType::Random: return "Random";
        }
        return "";
    }

    static constexpr const char* getTargetName(ModTargetType type)
    {
        switch (type)
        {
            case ModTargetType::OscillatorPitch: return "OSC Pitch";
            case ModTargetType::OscillatorLevel: return "OSC Level";
            case ModTargetType::OscillatorWaveform: return "OSC Wave";
            case ModTargetType::FilterCutoff: return "Filter Cutoff";
            case ModTargetType::FilterResonance: return "Filter Resonance";
            case ModTargetType::AmpVolume: return "Volume";
            case ModTargetType::AmpPan: return "Pan";
            case ModTargetType::LFO1Rate: return "LFO 1 Rate";
            case ModTargetType::LFO2Rate: return "LFO 2 Rate";
            case ModTargetType::LFO3Rate: return "LFO 3 Rate";
            case ModTargetType::LFO4Rate: return "LFO 4 Rate";
            case ModTargetType::EffectParam1: return "Effect Param 1";
            case ModTargetType::EffectParam2: return "Effect Param 2";
            case ModTargetType::EffectParam3: return "Effect Param 3";
            case ModTargetType::EffectMix: return "Effect Mix";
        }
        return "";
    }

private:
    struct SourceValue
    {
        ModSourceType type;
        int index = 0;
        float value = 0.0f;
        float phase = 0.0f;
        bool keyDown = false;
    };

    std::vector<ModConnection> connections;
    std::array<SourceValue, MAX_MOD_SOURCES> sourceValues;

    double sampleRate = 44100.0;
    int samplesPerBlock = 512;

    float lfo1Phase = 0.0f;
    float lfo2Phase = 0.0f;
    float lfo3Phase = 0.0f;
    float lfo4Phase = 0.0f;

    int nextConnectionId = 0;
};