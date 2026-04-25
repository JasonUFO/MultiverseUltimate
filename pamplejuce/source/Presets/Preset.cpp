#include "Preset.h"

Preset::Preset() {}

Preset::Preset(const juce::String& newName) : name(newName) {}

juce::String Preset::getName() const { return name; }
void Preset::setName(const juce::String& n) { name = n; }

juce::String Preset::getCategory() const { return category; }
void Preset::setCategory(const juce::String& c) { category = c; }

juce::String Preset::getAuthor() const { return author; }
void Preset::setAuthor(const juce::String& a) { author = a; }

void Preset::setParameter(int index, float value)
{
    if (index >= (int)parameters.size())
        parameters.resize(index + 1);
    parameters[index] = value;
}

float Preset::getParameter(int index) const
{
    if (index < (int)parameters.size())
        return parameters[index];
    return 0.0f;
}

void Preset::serialize(juce::XmlElement& xml) const
{
    xml.setAttribute("name", name);
    xml.setAttribute("category", category);
    xml.setAttribute("author", author);
    
    for (int i = 0; i < (int)parameters.size(); i++)
    {
        xml.setAttribute("param" + juce::String(i), parameters[i]);
    }
}

bool Preset::deserialize(const juce::XmlElement& xml)
{
    name = xml.getStringAttribute("name", "Init");
    category = xml.getStringAttribute("category", "Basic");
    author = xml.getStringAttribute("author", "MultiphaseAudio");
    
    parameters.clear();
    for (int i = 0; xml.hasAttribute("param" + juce::String(i)); i++)
    {
        parameters.push_back((float)xml.getDoubleAttribute("param" + juce::String(i), 0.0));
    }
    
    return true;
}