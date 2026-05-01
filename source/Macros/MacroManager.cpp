#include "MacroManager.h"

MacroManager::MacroManager()
{
    for (int i = 0; i < NUM_MACROS; ++i)
        names[i] = "Macro " + juce::String(i + 1);
}

void MacroManager::addAssignment(int macroIdx, const juce::String& paramID,
                                  float minVal, float maxVal)
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return;
    juce::ScopedLock sl(lock);
    for (int i = 0; i < NUM_MACROS; ++i)
        targets[i].erase(std::remove_if(targets[i].begin(), targets[i].end(),
            [&](const MacroTarget& t) { return t.paramID == paramID; }),
            targets[i].end());
    targets[macroIdx].push_back({ paramID, minVal, maxVal });
}

void MacroManager::removeAssignment(int macroIdx, const juce::String& paramID)
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return;
    juce::ScopedLock sl(lock);
    auto& v = targets[macroIdx];
    v.erase(std::remove_if(v.begin(), v.end(),
        [&](const MacroTarget& t) { return t.paramID == paramID; }),
        v.end());
}

bool MacroManager::isAssigned(int macroIdx, const juce::String& paramID) const
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return false;
    juce::ScopedLock sl(lock);
    for (const auto& t : targets[macroIdx])
        if (t.paramID == paramID)
            return true;
    return false;
}

int MacroManager::getAssignmentCount(int macroIdx) const
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return 0;
    juce::ScopedLock sl(lock);
    return static_cast<int>(targets[macroIdx].size());
}

std::vector<MacroTarget> MacroManager::getTargets(int macroIdx) const
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return {};
    juce::ScopedLock sl(lock);
    return targets[macroIdx];
}

void MacroManager::setName(int macroIdx, const juce::String& name)
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return;
    juce::ScopedLock sl(lock);
    names[macroIdx] = name;
}

juce::String MacroManager::getName(int macroIdx) const
{
    if (macroIdx < 0 || macroIdx >= NUM_MACROS)
        return {};
    juce::ScopedLock sl(lock);
    return names[macroIdx];
}

int MacroManager::getMacroIndexForParam(const juce::String& paramID) const
{
    juce::ScopedLock sl(lock);
    for (int i = 0; i < NUM_MACROS; ++i)
        for (const auto& t : targets[i])
            if (t.paramID == paramID)
                return i;
    return -1;
}

juce::ValueTree MacroManager::getState() const
{
    juce::ScopedLock sl(lock);
    juce::ValueTree root("MacroManager");
    for (int i = 0; i < NUM_MACROS; ++i)
    {
        juce::ValueTree macroNode("Macro");
        macroNode.setProperty("index", i, nullptr);
        macroNode.setProperty("name",  names[i], nullptr);
        for (const auto& t : targets[i])
        {
            juce::ValueTree tNode("Target");
            tNode.setProperty("paramID", t.paramID, nullptr);
            tNode.setProperty("minVal",  t.minVal,  nullptr);
            tNode.setProperty("maxVal",  t.maxVal,  nullptr);
            macroNode.appendChild(tNode, nullptr);
        }
        root.appendChild(macroNode, nullptr);
    }
    return root;
}

void MacroManager::setState(const juce::ValueTree& tree)
{
    if (!tree.hasType("MacroManager"))
        return;
    juce::ScopedLock sl(lock);
    for (int i = 0; i < NUM_MACROS; ++i)
        targets[i].clear();
    for (const auto& macroNode : tree)
    {
        if (!macroNode.hasType("Macro"))
            continue;
        const int idx = static_cast<int>(macroNode.getProperty("index", -1));
        if (idx < 0 || idx >= NUM_MACROS)
            continue;
        names[idx] = macroNode.getProperty("name",
            "Macro " + juce::String(idx + 1)).toString();
        for (const auto& tNode : macroNode)
        {
            if (!tNode.hasType("Target"))
                continue;
            MacroTarget t;
            t.paramID = tNode.getProperty("paramID", "").toString();
            t.minVal  = static_cast<float>(tNode.getProperty("minVal", 0.0f));
            t.maxVal  = static_cast<float>(tNode.getProperty("maxVal", 1.0f));
            if (t.paramID.isNotEmpty())
                targets[idx].push_back(t);
        }
    }
}
