#pragma once
#include <JuceHeader.h>
#include "../MultiverseFlatTheme.h"

class CollapsibleSection : public juce::Component
{
public:
    CollapsibleSection(const juce::String& title, bool expanded = true)
        : sectionTitle(title), isExpanded(expanded)
    {
        headerButton.setButtonText(title);
        headerButton.setColour(juce::TextButton::buttonColourId, juce::Colour());
        headerButton.setColour(juce::TextButton::textColourOffId, MultiverseFlatTheme::textPrimary());
        headerButton.setColour(juce::TextButton::textColourOnId, MultiverseFlatTheme::accentCyan());
        headerButton.setClickingTogglesState(false);
        headerButton.onClick = [this] { toggleExpanded(); };
        addAndMakeVisible(headerButton);
    }

    void setContent(std::unique_ptr<juce::Component> content)
    {
        contentComponent = std::move(content);
        if (contentComponent)
        {
            addAndMakeVisible(*contentComponent);
            contentComponent->setVisible(isExpanded);
        }
    }

    juce::Component* getContent() const { return contentComponent.get(); }

    bool getExpanded() const { return isExpanded; }

    void setExpanded(bool expanded, bool animate = true)
    {
        if (isExpanded == expanded) return;
        isExpanded = expanded;
        if (contentComponent)
            contentComponent->setVisible(isExpanded);
        headerButton.repaint();
        if (onStateChanged) onStateChanged();
        resized();
    }

    void toggleExpanded() { setExpanded(!isExpanded); }

    int getHeaderHeight() const { return 24; }

    int getDesiredHeight() const
    {
        if (!isExpanded) return getHeaderHeight();
        return getHeaderHeight() + contentHeight;
    }

    void setContentHeight(int h) { contentHeight = h; }

    std::function<void()> onStateChanged;

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        // Draw section card background
        MultiverseFlatTheme::drawCard(g, b, 8.0f, false);

        // Disclosure triangle
        const float triX = 10.0f;
        const float triY = (float)getHeaderHeight() * 0.5f;
        const float triSize = 6.0f;

        juce::Path triangle;
        if (isExpanded)
        {
            // Down-pointing triangle (expanded)
            triangle.addTriangle(triX - triSize * 0.5f, triY - triSize * 0.4f,
                                  triX + triSize * 0.5f, triY - triSize * 0.4f,
                                  triX, triY + triSize * 0.6f);
        }
        else
        {
            // Right-pointing triangle (collapsed)
            triangle.addTriangle(triX - triSize * 0.4f, triY - triSize * 0.5f,
                                  triX - triSize * 0.4f, triY + triSize * 0.5f,
                                  triX + triSize * 0.6f, triY);
        }
        g.setColour(isExpanded ? MultiverseFlatTheme::accentCyan() : MultiverseFlatTheme::textMuted());
        g.fillPath(triangle);

        // Title text
        g.setColour(MultiverseFlatTheme::textLabel());
        g.setFont(MultiverseFlatTheme::headerFont());
        g.drawText(sectionTitle, static_cast<int>(triX + triSize + 4), 0,
                   getWidth() - static_cast<int>(triX + triSize + 8), getHeaderHeight(),
                   juce::Justification::centredLeft, false);

        // Divider line below header
        MultiverseFlatTheme::drawDivider(g, static_cast<float>(getHeaderHeight()),
                                          8.0f, static_cast<float>(getWidth() - 8));
    }

    void resized() override
    {
        headerButton.setBounds(getLocalBounds().withHeight(getHeaderHeight()));

        if (contentComponent && isExpanded)
        {
            contentComponent->setBounds(getLocalBounds()
                                         .withTop(getHeaderHeight())
                                         .reduced(8, 0)
                                         .withTrimmedBottom(4));
        }
    }

private:
    juce::String sectionTitle;
    bool isExpanded;
    int contentHeight = 0;
    juce::TextButton headerButton;
    std::unique_ptr<juce::Component> contentComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CollapsibleSection)
};