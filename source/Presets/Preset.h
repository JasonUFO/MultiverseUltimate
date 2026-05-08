#pragma once
#include <JuceHeader.h>

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

    juce::String getDescription() const;
    void setDescription(const juce::String& desc);

    juce::StringArray getTags() const;
    void setTags(const juce::StringArray& t);
    juce::String getTagsString() const;
    void setTagsFromString(const juce::String& commaSeparated);

    juce::StringArray getCharacters() const;
    void setCharacters(const juce::StringArray& c);
    juce::String getCharactersString() const;
    void setCharactersFromString(const juce::String& commaSeparated);

    void setParameter(int index, float value);
    float getParameter(int index) const;

    void serialize(juce::XmlElement& xml) const;
    bool deserialize(const juce::XmlElement& xml);

private:
    juce::String name = "Init";
    juce::String category = "Basic";
    juce::String author = "MultiphaseAudio";
    juce::String description;
    juce::StringArray tags;
    juce::StringArray characters;
    std::vector<float> parameters;
};