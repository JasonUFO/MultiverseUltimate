#pragma once
#include <JuceHeader.h>

class FactoryPresets
{
public:
    // Writes 100 factory preset files to factoryDir (Factory/).
    // Category subdirs (Init/Bass/Lead/Pad/Drums/FX) must already exist.
    static void writeToDirectory(const juce::File& factoryDir);
};
