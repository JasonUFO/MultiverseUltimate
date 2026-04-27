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
    int sourceIndex = static_cast<int>(source);
    if (sourceIndex >= 0 && sourceIndex < MAX_MOD_SOURCES)
    {
        return sourceValues[sourceIndex].value.load(std::memory_order_relaxed);
    }
    return 0.0f;
}

void ModulationMatrix::setModulationValue(ModSourceType source, int index, float value)
{
    int sourceIndex = static_cast<int>(source);
    if (sourceIndex >= 0 && sourceIndex < MAX_MOD_SOURCES)
    {
        sourceValues[sourceIndex].value.store(value, std::memory_order_relaxed);
    }
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
    {
        return sourceValues[index].type;
    }
    return ModSourceType::LFO1;
}

ModTargetType ModulationMatrix::getTargetType(int index) const
{
    if (index >= 0 && index < MAX_MOD_TARGETS)
    {
        return static_cast<ModTargetType>(index);
    }
    return ModTargetType::FilterCutoff;
}

void ModulationMatrix::setLFORate(int lfoIndex, float rateHz)
{
    if (lfoIndex < 0 || lfoIndex > 3) return;
    rateHz = std::clamp(rateHz, 0.01f, 100.0f);
    switch (lfoIndex)
    {
        case 0: lfo1Rate = rateHz; break;
        case 1: lfo2Rate = rateHz; break;
        case 2: lfo3Rate = rateHz; break;
        case 3: lfo4Rate = rateHz; break;
    }
}

float ModulationMatrix::getLFORate(int lfoIndex) const
{
    if (lfoIndex < 0 || lfoIndex > 3) return 1.0f;
    switch (lfoIndex)
    {
        case 0: return lfo1Rate;
        case 1: return lfo2Rate;
        case 2: return lfo3Rate;
        case 3: return lfo4Rate;
    }
    return 1.0f;
}

void ModulationMatrix::advanceLFOs()
{
    constexpr double twoPi = 6.283185307179586;
    double sr = sampleRate > 0 ? sampleRate : 44100.0;

    float currPhase1 = lfo1Phase.load(std::memory_order_relaxed);
    float val = static_cast<float>(std::sin(currPhase1));
    sourceValues[static_cast<int>(ModSourceType::LFO1)].value.store(val, std::memory_order_relaxed);
    float newPhase1 = currPhase1 + static_cast<float>(twoPi * lfo1Rate / sr);
    lfo1Phase.store(newPhase1 >= twoPi ? newPhase1 - twoPi : newPhase1, std::memory_order_relaxed);

    float currPhase2 = lfo2Phase.load(std::memory_order_relaxed);
    val = static_cast<float>(std::sin(currPhase2));
    sourceValues[static_cast<int>(ModSourceType::LFO2)].value.store(val, std::memory_order_relaxed);
    float newPhase2 = currPhase2 + static_cast<float>(twoPi * lfo2Rate / sr);
    lfo2Phase.store(newPhase2 >= twoPi ? newPhase2 - twoPi : newPhase2, std::memory_order_relaxed);

    float currPhase3 = lfo3Phase.load(std::memory_order_relaxed);
    val = static_cast<float>(std::sin(currPhase3));
    sourceValues[static_cast<int>(ModSourceType::LFO3)].value.store(val, std::memory_order_relaxed);
    float newPhase3 = currPhase3 + static_cast<float>(twoPi * lfo3Rate / sr);
    lfo3Phase.store(newPhase3 >= twoPi ? newPhase3 - twoPi : newPhase3, std::memory_order_relaxed);

    float currPhase4 = lfo4Phase.load(std::memory_order_relaxed);
    val = static_cast<float>(std::sin(currPhase4));
    sourceValues[static_cast<int>(ModSourceType::LFO4)].value.store(val, std::memory_order_relaxed);
    float newPhase4 = currPhase4 + static_cast<float>(twoPi * lfo4Rate / sr);
    lfo4Phase.store(newPhase4 >= twoPi ? newPhase4 - twoPi : newPhase4, std::memory_order_relaxed);
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
    v.setProperty("lfo1Rate", lfo1Rate, nullptr);
    v.setProperty("lfo2Rate", lfo2Rate, nullptr);
    v.setProperty("lfo3Rate", lfo3Rate, nullptr);
    v.setProperty("lfo4Rate", lfo4Rate, nullptr);

    auto conns = juce::ValueTree("Connections");
    {
        juce::ScopedLock lock(connectionLock);
        for (const auto& c : connections)
        {
            juce::ValueTree conn("Conn");
            conn.setProperty("source", static_cast<int>(c.source), nullptr);
            conn.setProperty("target", static_cast<int>(c.target), nullptr);
            conn.setProperty("amount", c.amount, nullptr);
            conn.setProperty("enabled", c.enabled, nullptr);
            conn.setProperty("sourceIndex", c.sourceIndex, nullptr);
            conn.setProperty("targetIndex", c.targetIndex, nullptr);
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

    if (state.hasProperty("lfo1Rate")) lfo1Rate = (float)state.getProperty("lfo1Rate");
    if (state.hasProperty("lfo2Rate")) lfo2Rate = (float)state.getProperty("lfo2Rate");
    if (state.hasProperty("lfo3Rate")) lfo3Rate = (float)state.getProperty("lfo3Rate");
    if (state.hasProperty("lfo4Rate")) lfo4Rate = (float)state.getProperty("lfo4Rate");

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
                    c.source = static_cast<ModSourceType>((int)conn.getProperty("source"));
                    c.target = static_cast<ModTargetType>((int)conn.getProperty("target"));
                    c.amount = (float)conn.getProperty("amount");
                    c.enabled = conn.getProperty("enabled");
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
