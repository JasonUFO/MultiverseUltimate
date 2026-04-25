#include "ModulationMatrix.h"

ModulationMatrix::ModulationMatrix()
{
    for (int i = 0; i < MAX_MOD_SOURCES; ++i)
    {
        sourceValues[i].type = static_cast<ModSourceType>(i);
        sourceValues[i].index = 0;
        sourceValues[i].value = 0.0f;
        sourceValues[i].phase = 0.0f;
    }
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
        return sourceValues[sourceIndex].value;
    }
    return 0.0f;
}

void ModulationMatrix::setModulationValue(ModSourceType source, int index, float value)
{
    int sourceIndex = static_cast<int>(source);
    if (sourceIndex >= 0 && sourceIndex < MAX_MOD_SOURCES)
    {
        sourceValues[sourceIndex].value = value;
    }
}

void ModulationMatrix::prepare(double sampleRate_, int samplesPerBlock_)
{
    sampleRate = sampleRate_;
    samplesPerBlock = samplesPerBlock_;
}

std::vector<ModConnection> ModulationMatrix::getActiveConnectionsForTarget(ModTargetType target, int index) const
{
    std::vector<ModConnection> result;
    for (const auto& conn : connections)
        if (conn.target == target && conn.targetIndex == index && conn.enabled)
            result.push_back(conn);
    return result;
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