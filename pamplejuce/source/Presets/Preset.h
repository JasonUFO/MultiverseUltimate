#pragma once
#include <juce_core/juce_core.h>

class Preset
{
public:
    Preset();
    explicit Preset(const juce::String& name);
    
    juce::String getName() const;
    void setName(const juce::String& name);
    
    juce::String getCategory() const;
    void setCategory(const juce::String& category);
    
    juce::String getAuthor() const;
    void setAuthor(const juce::String& author);
    
    void setParameter(int index, float value);
    float getParameter(int index) const;
    
    void serialize(juce::XmlElement& xml) const;
    bool deserialize(const juce::XmlElement& xml);
    
private:
    juce::String name = "Init";
    juce::String category = "Basic";
    juce::String author = "MultiphaseAudio";
    std::vector<float> parameters;
};