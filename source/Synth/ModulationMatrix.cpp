#include "ModulationMatrix.h"
#include <cmath>
#include <algorithm>
#include <memory>
#include "JuceHeader.h"

ModulationMatrix::ModulationMatrix()
{
    for (int i = 0; i < MAX_MOD_SOURCES; ++i)
    {
        sourceValues[i].type = static_cast<ModSourceType>(i);
        sourceValues[i].index = 0;
        sourceValues[i].value.store(0.0f, std::memory_order_relaxed);
        sourceValues[i].phase = 0.0f;
    }

    for (int i = 0; i < 8; ++i)
    {
        lfoPhase[i].store(0.0f, std::memory_order_relaxed);
        lfoRate[i]    = 1.0f;
        lfoShape[i]   = LFOShape::Sine;
        lfoSHValue[i] = 0.0f;
        // Default custom table: sine
        for (int j = 0; j < 256; ++j)
            lfoCustomTable[i][j] = std::sin(2.0f * 3.14159265f * (float)j / 256.0f);
    }

    for (int b = 0; b < 2; ++b)
        for (int t = 0; t < MAX_MOD_TARGETS; ++t)
            modSumsBuffer[b][t] = 0.0f;
}

int ModulationMatrix::addConnection(ModSourceType source, ModTargetType target, float amount)
{
    if (connections.size() >= MAX_MOD_CONNECTIONS)
        return -1;

    ModConnection conn;
    conn.source = source;
    conn.target = target;
    conn.amount = amount;
    conn.enabled = true;
    conn.sourceIndex = 0;
    conn.targetIndex = 0;

    connections.push_back(conn);
    return nextConnectionId++;
}

void ModulationMatrix::removeConnection(int connectionId)
{
    if (connectionId >= 0 && connectionId < static_cast<int>(connections.size()))
    {
        connections.erase(connections.begin() + connectionId);
    }
}

void ModulationMatrix::updateConnection(int connectionId, float amount)
{
    if (connectionId >= 0 && connectionId < static_cast<int>(connections.size()))
    {
        connections[connectionId].amount = amount;
    }
}

void ModulationMatrix::updateConnectionSource(int connectionId, ModSourceType source, int index)
{
    if (connectionId >= 0 && connectionId < static_cast<int>(connections.size()))
    {
        connections[connectionId].source = source;
        connections[connectionId].sourceIndex = index;
    }
}

void ModulationMatrix::updateConnectionTarget(int connectionId, ModTargetType target, int index)
{
    if (connectionId >= 0 && connectionId < static_cast<int>(connections.size()))
    {
        connections[connectionId].target = target;
        connections[connectionId].targetIndex = index;
    }
}

void ModulationMatrix::setConnectionEnabled(int connectionId, bool enabled)
{
    if (connectionId >= 0 && connectionId < static_cast<int>(connections.size()))
    {
        connections[connectionId].enabled = enabled;
    }
}

float ModulationMatrix::getModulationValue(ModSourceType source, int index)
{
    int srcIdx = static_cast<int>(source);
    if (srcIdx >= 0 && srcIdx < MAX_MOD_SOURCES)
        return sourceValues[srcIdx].value.load(std::memory_order_relaxed);
    return 0.0f;
}

void ModulationMatrix::setModulationValue(ModSourceType source, int index, float value)
{
    int srcIdx = static_cast<int>(source);
    if (srcIdx >= 0 && srcIdx < MAX_MOD_SOURCES)
        sourceValues[srcIdx].value.store(value, std::memory_order_relaxed);
}

void ModulationMatrix::prepare(double sampleRate_, int samplesPerBlock_)
{
    sampleRate = sampleRate_;
    samplesPerBlock = samplesPerBlock_;

    for (int b = 0; b < 2; ++b)
        for (int t = 0; t < MAX_MOD_TARGETS; ++t)
            modSumsBuffer[b][t] = 0.0f;
    currentModSumsBuffer.store(0, std::memory_order_relaxed);
}

void ModulationMatrix::prepareForBlock()
{
    swapModSumsBuffers();
}

void ModulationMatrix::swapModSumsBuffers()
{
    int prev = currentModSumsBuffer.load(std::memory_order_relaxed);
    int next = 1 - prev;
    for (int t = 0; t < MAX_MOD_TARGETS; ++t)
        modSumsBuffer[next][t] = 0.0f;
    currentModSumsBuffer.store(next, std::memory_order_relaxed);
}

std::vector<ModConnection> ModulationMatrix::getActiveConnectionsForTarget(ModTargetType target, int index) const
{
    std::vector<ModConnection> result;
    juce::ScopedLock lock(connectionLock);
    for (const auto& conn : connections)
        if (conn.target == target && conn.targetIndex == index && conn.enabled)
            result.push_back(conn);
    return result;
}

std::vector<ModConnection> ModulationMatrix::getConnections() const
{
    juce::ScopedLock lock(connectionLock);
    return connections;
}

ModSourceType ModulationMatrix::getSourceType(int index) const
{
    if (index >= 0 && index < MAX_MOD_SOURCES)
        return sourceValues[index].type;
    return ModSourceType::LFO1;
}

ModTargetType ModulationMatrix::getTargetType(int index) const
{
    if (index >= 0 && index < MAX_MOD_TARGETS)
        return static_cast<ModTargetType>(index);
    return ModTargetType::FilterCutoff;
}

void ModulationMatrix::setLFORate(int lfoIndex, float rateHz)
{
    if (lfoIndex < 0 || lfoIndex >= 8) return;
    lfoRate[lfoIndex] = std::clamp(rateHz, 0.01f, 100.0f);
}

float ModulationMatrix::getLFORate(int lfoIndex) const
{
    if (lfoIndex < 0 || lfoIndex >= 8) return 1.0f;
    return lfoRate[lfoIndex];
}

void ModulationMatrix::setLFOShape(int lfoIndex, LFOShape shape)
{
    if (lfoIndex < 0 || lfoIndex >= 8) return;
    lfoShape[lfoIndex] = shape;
}

LFOShape ModulationMatrix::getLFOShape(int lfoIndex) const
{
    if (lfoIndex < 0 || lfoIndex >= 8) return LFOShape::Sine;
    return lfoShape[lfoIndex];
}

void ModulationMatrix::setCustomTable(int lfoIndex, const std::array<float, 256>& table)
{
    if (lfoIndex < 0 || lfoIndex >= 8) return;
    lfoCustomTable[lfoIndex] = table;
}

std::array<float, 256> ModulationMatrix::getCustomTable(int lfoIndex) const
{
    if (lfoIndex < 0 || lfoIndex >= 8) return lfoCustomTable[0];
    return lfoCustomTable[lfoIndex];
}

void ModulationMatrix::advanceLFOs(int numSamples)
{
    constexpr float pi    = 3.14159265358979f;
    constexpr float twoPi = 6.28318530717959f;
    const float sr = static_cast<float>(sampleRate > 0 ? sampleRate : 44100.0);
    const float delta = twoPi * static_cast<float>(numSamples) / sr;

    // LFO1-LFO4 write to sourceValues[0-3]
    // LFO5-LFO8 write to sourceValues[15-18]
    for (int i = 0; i < 8; ++i)
    {
        float p = lfoPhase[i].load(std::memory_order_relaxed);
        float inc = delta * lfoRate[i];
        float newP = p + inc;
        bool wrapped = (newP >= twoPi);
        if (wrapped) newP -= twoPi;
        lfoPhase[i].store(newP, std::memory_order_relaxed);

        float value = 0.0f;
        switch (lfoShape[i])
        {
            case LFOShape::Triangle:
                value = 1.0f - 2.0f * std::abs(newP / pi - 1.0f);
                break;
            case LFOShape::Saw:
                value = newP / pi - 1.0f;
                break;
            case LFOShape::Square:
                value = newP < pi ? 1.0f : -1.0f;
                break;
            case LFOShape::SampleAndHold:
                if (wrapped)
                    lfoSHValue[i] = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;
                value = lfoSHValue[i];
                break;
            case LFOShape::Custom:
            {
                float normPhase = newP / twoPi;
                float tablePos  = normPhase * 255.0f;
                int   idx0      = (int)tablePos & 255;
                int   idx1      = (idx0 + 1) & 255;
                float frac      = tablePos - (float)(int)tablePos;
                value = lfoCustomTable[i][idx0]
                      + frac * (lfoCustomTable[i][idx1] - lfoCustomTable[i][idx0]);
                break;
            }
            default:
                value = std::sin(newP);
                break;
        }

        const int srcIdx = (i < 4) ? i : (11 + i);  // 0-3 → 0-3, 4-7 → 15-18
        sourceValues[srcIdx].value.store(value, std::memory_order_relaxed);
    }
}

void ModulationMatrix::computeModulationSums(float* outSums) const
{
    int bufIdx = currentModSumsBuffer.load(std::memory_order_relaxed);
    const float* src = modSumsBuffer[bufIdx];
    for (int t = 0; t < MAX_MOD_TARGETS; ++t)
        outSums[t] = src[t];

    std::vector<ModConnection> connSnapshot;
    {
        juce::ScopedLock lock(connectionLock);
        connSnapshot = connections;
    }

    for (const auto& conn : connSnapshot)
    {
        if (!conn.enabled) continue;
        int srcIdx = static_cast<int>(conn.source);
        if (srcIdx < 0 || srcIdx >= MAX_MOD_SOURCES) continue;
        float srcVal = sourceValues[srcIdx].value.load(std::memory_order_relaxed);
        int tgtIdx = static_cast<int>(conn.target);
        if (tgtIdx < 0 || tgtIdx >= MAX_MOD_TARGETS) continue;
        outSums[tgtIdx] += srcVal * conn.amount;
    }
}

juce::ValueTree ModulationMatrix::getState() const
{
    juce::ValueTree v("ModulationMatrix");

    for (int i = 0; i < 8; ++i)
    {
        v.setProperty("lfo" + juce::String(i + 1) + "Rate",  lfoRate[i],       nullptr);
        v.setProperty("lfo" + juce::String(i + 1) + "Shape", (int)lfoShape[i], nullptr);

        // Custom table: 256 floats as comma-separated string
        juce::String tableStr;
        for (int j = 0; j < 256; ++j)
        {
            if (j > 0) tableStr += ",";
            tableStr += juce::String(lfoCustomTable[i][j], 6);
        }
        v.setProperty("lfo" + juce::String(i + 1) + "CustomTable", tableStr, nullptr);
    }

    auto conns = juce::ValueTree("Connections");
    {
        juce::ScopedLock lock(connectionLock);
        for (const auto& c : connections)
        {
            juce::ValueTree conn("Conn");
            conn.setProperty("source",      static_cast<int>(c.source), nullptr);
            conn.setProperty("target",      static_cast<int>(c.target), nullptr);
            conn.setProperty("amount",      c.amount,                   nullptr);
            conn.setProperty("enabled",     c.enabled,                  nullptr);
            conn.setProperty("sourceIndex", c.sourceIndex,              nullptr);
            conn.setProperty("targetIndex", c.targetIndex,              nullptr);
            conns.appendChild(conn, nullptr);
        }
    }
    v.appendChild(conns, nullptr);
    return v;
}

void ModulationMatrix::setState(const juce::ValueTree& state)
{
    if (!state.hasType("ModulationMatrix"))
        return;

    // Backward compat: individual properties from old format
    if (state.hasProperty("lfo1Rate")) lfoRate[0] = (float)state.getProperty("lfo1Rate");
    if (state.hasProperty("lfo2Rate")) lfoRate[1] = (float)state.getProperty("lfo2Rate");
    if (state.hasProperty("lfo3Rate")) lfoRate[2] = (float)state.getProperty("lfo3Rate");
    if (state.hasProperty("lfo4Rate")) lfoRate[3] = (float)state.getProperty("lfo4Rate");

    for (int i = 0; i < 8; ++i)
    {
        const juce::String rk = "lfo" + juce::String(i + 1) + "Rate";
        const juce::String sk = "lfo" + juce::String(i + 1) + "Shape";
        const juce::String tk = "lfo" + juce::String(i + 1) + "CustomTable";
        if (state.hasProperty(rk))  lfoRate[i]  = (float)state.getProperty(rk);
        if (state.hasProperty(sk))  lfoShape[i] = static_cast<LFOShape>((int)state.getProperty(sk));
        if (state.hasProperty(tk))
        {
            juce::StringArray tokens;
            tokens.addTokens(state.getProperty(tk).toString(), ",", "");
            for (int j = 0; j < 256 && j < tokens.size(); ++j)
                lfoCustomTable[i][j] = tokens[j].getFloatValue();
        }
    }

    std::vector<ModConnection> newConnections;
    for (auto child : state)
    {
        if (child.hasType("Connections"))
        {
            for (auto conn : child)
            {
                if (conn.hasType("Conn"))
                {
                    ModConnection c;
                    c.source      = static_cast<ModSourceType>((int)conn.getProperty("source"));
                    c.target      = static_cast<ModTargetType>((int)conn.getProperty("target"));
                    c.amount      = (float)conn.getProperty("amount");
                    c.enabled     = conn.getProperty("enabled");
                    c.sourceIndex = (int)conn.getProperty("sourceIndex");
                    c.targetIndex = (int)conn.getProperty("targetIndex");
                    newConnections.push_back(c);
                }
            }
        }
    }

    {
        juce::ScopedLock lock(connectionLock);
        connections.clear();
        connections = std::move(newConnections);
    }
}
