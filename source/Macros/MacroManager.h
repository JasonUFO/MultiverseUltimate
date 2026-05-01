#pragma once
#include <JuceHeader.h>

struct MacroTarget
{
    juce::String paramID;
    float        minVal = 0.0f;
    float        maxVal = 1.0f;
};

class MacroManager
{
public:
    static constexpr int NUM_MACROS = 8;

    MacroManager();

    // Assign paramID to a macro. Removes any prior assignment from other macros.
    void addAssignment(int macroIdx, const juce::String& paramID,
                       float minVal = 0.0f, float maxVal = 1.0f);
    void removeAssignment(int macroIdx, const juce::String& paramID);
    bool isAssigned(int macroIdx, const juce::String& paramID) const;
    int  getAssignmentCount(int macroIdx) const;

    // Returns a copy (thread-safe snapshot).
    std::vector<MacroTarget> getTargets(int macroIdx) const;

    void         setName(int macroIdx, const juce::String& name);
    juce::String getName(int macroIdx) const;

    // Returns the macro index (0-based) this param is assigned to, or -1.
    int getMacroIndexForParam(const juce::String& paramID) const;

    juce::ValueTree getState() const;
    void            setState(const juce::ValueTree& tree);

private:
    std::array<std::vector<MacroTarget>, NUM_MACROS> targets;
    std::array<juce::String, NUM_MACROS>             names;
    mutable juce::CriticalSection                    lock;
};
