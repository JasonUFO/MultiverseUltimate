#include "PresetBrowserPanel.h"
#include "../PluginProcessor.h"

PresetBrowserPanel::PresetBrowserPanel(PluginProcessor& p)
    : processorRef(p)
{
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(nameLabel);

    nameEditor.setTextToShowWhenEmpty("Preset name...", juce::Colours::grey);
    nameEditor.setFont(juce::Font(13.0f, juce::Font::plain));
    addAndMakeVisible(nameEditor);

    saveButton.addListener(this);
    loadButton.addListener(this);
    deleteButton.addListener(this);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(deleteButton);

    presetList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1e1e1e));
    presetList.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff444444));
    presetList.setRowHeight(22);
    addAndMakeVisible(presetList);

    refresh();
}

void PresetBrowserPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    g.setColour(juce::Colour(0xff555555));
    g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
}

void PresetBrowserPanel::resized()
{
    auto area = getLocalBounds().reduced(6, 4);

    // Top row: label + name editor + save button
    auto topRow = area.removeFromTop(26);
    nameLabel.setBounds(topRow.removeFromLeft(44));
    saveButton.setBounds(topRow.removeFromRight(60).reduced(2, 1));
    topRow.removeFromLeft(4);
    nameEditor.setBounds(topRow.reduced(0, 2));

    area.removeFromTop(4);

    // Remaining: list on left, load/delete on right
    auto btnCol = area.removeFromRight(64);
    loadButton.setBounds  (btnCol.removeFromTop(26).reduced(2, 1));
    btnCol.removeFromTop(4);
    deleteButton.setBounds(btnCol.removeFromTop(26).reduced(2, 1));

    area.removeFromRight(4);
    presetList.setBounds(area);
}

int PresetBrowserPanel::getNumRows()
{
    return processorRef.getPresetManager().getPresetCount();
}

void PresetBrowserPanel::paintListBoxItem(int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (selected)
        g.fillAll(juce::Colour(0xffcc6600));
    else if (row % 2 == 0)
        g.fillAll(juce::Colour(0xff252525));
    else
        g.fillAll(juce::Colour(0xff1e1e1e));

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(13.0f, juce::Font::plain));
    auto names = processorRef.getPresetManager().getPresetNames();
    if (row < names.size())
        g.drawText(names[row], 6, 0, w - 6, h, juce::Justification::centredLeft);
}

void PresetBrowserPanel::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (processorRef.loadPresetAtIndex(row))
        refresh();
}

void PresetBrowserPanel::buttonClicked(juce::Button* b)
{
    if (b == &saveButton)   saveCurrentPreset();
    if (b == &loadButton)   loadSelectedPreset();
    if (b == &deleteButton) deleteSelectedPreset();
}

void PresetBrowserPanel::saveCurrentPreset()
{
    auto name = nameEditor.getText().trim();
    if (name.isEmpty())
        return;
    processorRef.saveNamedPreset(name);
    nameEditor.clear();
    refresh();
}

void PresetBrowserPanel::loadSelectedPreset()
{
    int row = presetList.getSelectedRow();
    if (row >= 0)
    {
        processorRef.loadPresetAtIndex(row);
        refresh();
    }
}

void PresetBrowserPanel::deleteSelectedPreset()
{
    int row = presetList.getSelectedRow();
    if (row >= 0)
    {
        processorRef.getPresetManager().deletePreset(row);
        refresh();
    }
}

void PresetBrowserPanel::refresh()
{
    presetList.updateContent();
    presetList.repaint();
}
