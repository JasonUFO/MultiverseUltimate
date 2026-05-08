#include "Preset.h"

Preset::Preset() {}

Preset::Preset(const juce::String& newName) : name(newName) {}

juce::String Preset::getName() const { return name; }
void Preset::setName(const juce::String& n) { name = n; }

juce::String Preset::getCategory() const { return category; }
void Preset::setCategory(const juce::String& c) { category = c; }

juce::String Preset::getAuthor() const { return author; }
void Preset::setAuthor(const juce::String& a) { author = a; }

juce::String Preset::getDescription() const { return description; }
void Preset::setDescription(const juce::String& d) { description = d; }

juce::StringArray Preset::getTags() const { return tags; }
void Preset::setTags(const juce::StringArray& t) { tags = t; }

juce::String Preset::getTagsString() const { return tags.joinIntoString(","); }

void Preset::setTagsFromString(const juce::String& commaSeparated)
{
    tags.clear();
    if (commaSeparated.isNotEmpty())
    {
        auto parts = juce::StringArray::fromTokens(commaSeparated, ",", "");
        for (auto& p : parts)
        {
            p = p.trim().toLowerCase();
            if (p.isNotEmpty() && !tags.contains(p))
                tags.add(p);
        }
    }
}

juce::StringArray Preset::getCharacters() const { return characters; }
void Preset::setCharacters(const juce::StringArray& c) { characters = c; }

juce::String Preset::getCharactersString() const { return characters.joinIntoString(","); }

void Preset::setCharactersFromString(const juce::String& commaSeparated)
{
    characters.clear();
    if (commaSeparated.isNotEmpty())
    {
        auto parts = juce::StringArray::fromTokens(commaSeparated, ",", "");
        for (auto& p : parts)
        {
            p = p.trim().toLowerCase();
            if (p.isNotEmpty() && !characters.contains(p))
                characters.add(p);
        }
    }
}

void Preset::setParameter(int index, float value)
{
    if (index < 0) return;
    if (index >= (int)parameters.size())
        parameters.resize(static_cast<size_t>(index) + 1);
    parameters[static_cast<size_t>(index)] = value;
}

float Preset::getParameter(int index) const
{
    if (index >= 0 && index < (int)parameters.size())
        return parameters[static_cast<size_t>(index)];
    return 0.0f;
}

void Preset::serialize(juce::XmlElement& xml) const
{
    xml.setAttribute("name", name);
    xml.setAttribute("category", category);
    xml.setAttribute("author", author);
    xml.setAttribute("description", description);
    xml.setAttribute("tags", getTagsString());
    xml.setAttribute("characters", getCharactersString());

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
    description = xml.getStringAttribute("description", "");
    setTagsFromString(xml.getStringAttribute("tags", ""));
    setCharactersFromString(xml.getStringAttribute("characters", ""));

    // Extract #hashtags from description
    if (description.contains("#"))
    {
        auto words = juce::StringArray::fromTokens(description, " ,.;:!?\n\r\t", "'");
        for (auto& w : words)
        {
            if (w.startsWith("#") && w.length() > 1)
            {
                auto tag = w.substring(1).toLowerCase();
                if (tag.isNotEmpty() && !tags.contains(tag))
                    tags.add(tag);
            }
        }
    }

    parameters.clear();
    for (int i = 0; xml.hasAttribute("param" + juce::String(i)); i++)
    {
        parameters.push_back((float)xml.getDoubleAttribute("param" + juce::String(i), 0.0));
    }

    return true;
}