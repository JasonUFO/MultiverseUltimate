#include "PresetBrowserPanel.h"
#include "../PluginProcessor.h"
#include "../CyberpunkTheme.h"

namespace
{
    const juce::Colour bgBase   { 0xff171720 };
    const juce::Colour bgRaised { 0xff1e1e2c };
    const juce::Colour bgDeep   { 0xff111119 };
    const juce::Colour shadowDark  { 0xff0b0b12 };
    const juce::Colour shadowLight { 0xff252535 };
    const juce::Colour accentBlue  { 0xff5b8def };
    const juce::Colour accentPurple{ 0xffc084fc };
    const juce::Colour textPrimary  { 0xffe4e8f0 };
    const juce::Colour textSecondary{ 0xff7a8499 };
    const juce::Colour textMuted    { 0xff3d4358 };

    void drawNeumorphicRect (juce::Graphics& g, juce::Rectangle<float> b,
                             float cornerRadius, float offset)
    {
        juce::Path p;
        p.addRoundedRectangle (b, cornerRadius);
        const int blur = juce::roundToInt (offset * 2.0f);
        {
            juce::DropShadow ds { shadowDark.withAlpha (0.65f), blur, { (int)offset, (int)offset } };
            ds.drawForPath (g, p);
        }
        {
            juce::DropShadow ds { shadowLight.withAlpha (0.45f), blur, { -(int)offset, -(int)offset } };
            ds.drawForPath (g, p);
        }
    }
}

//==============================================================================
PresetBrowserPanel::PresetBrowserPanel(PluginProcessor& p)
    : processorRef(p)
{
    // Search editor
    searchEditor.setTextToShowWhenEmpty ("Search presets...", textMuted);
    searchEditor.setFont (juce::Font (12.0f, juce::Font::plain));
    searchEditor.addListener (this);
    addAndMakeVisible (searchEditor);

    // Category buttons
    categoryButtons = { &catAll, &catInit, &catBass, &catLead, &catPad, &catDrums, &catFX };
    for (auto* btn : categoryButtons)
    {
        btn->setClickingTogglesState (true);
        btn->setRadioGroupId (101);
        btn->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn->setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        btn->setColour (juce::TextButton::textColourOffId, textSecondary);
        btn->setColour (juce::TextButton::textColourOnId, textPrimary);
        btn->addListener (this);
        addAndMakeVisible (btn);
    }
    catAll.setToggleState (true, juce::dontSendNotification);

    // Bank selector
    auto banks = processorRef.getPresetManager().getBankNames();
    bankSelector.addItemList (banks, 1);
    bankSelector.setSelectedItemIndex (processorRef.getPresetManager().getCurrentBank(), juce::dontSendNotification);
    bankSelector.addListener (this);
    addAndMakeVisible (bankSelector);

    // Name + save
    nameLabel.setColour (juce::Label::textColourId, textSecondary);
    addAndMakeVisible (nameLabel);

    nameEditor.setTextToShowWhenEmpty ("Preset name...", textMuted);
    nameEditor.setFont (juce::Font (12.0f, juce::Font::plain));
    addAndMakeVisible (nameEditor);

    saveButton.addListener (this);
    loadButton.addListener (this);
    deleteButton.addListener (this);
    importButton.addListener (this);
    exportButton.addListener (this);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (loadButton);
    addAndMakeVisible (deleteButton);
    addAndMakeVisible (importButton);
    addAndMakeVisible (exportButton);

    presetList.setRowHeight (32);
    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetList.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (presetList);

    countLabel.setFont (juce::Font (10.0f, juce::Font::plain));
    countLabel.setColour (juce::Label::textColourId, textSecondary);
    countLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (countLabel);

    rebuildFilter();
}

//==============================================================================
void PresetBrowserPanel::paint (juce::Graphics& g)
{
    g.fillAll (bgBase);
}

//==============================================================================
void PresetBrowserPanel::resized()
{
    auto area = getLocalBounds().reduced (6, 6);

    // Search bar at top (count label right-aligned inside it)
    auto searchBar = area.removeFromTop (28);
    countLabel.setBounds (searchBar.removeFromRight (72).reduced (0, 6));
    searchEditor.setBounds (searchBar);
    area.removeFromTop (6);

    // Category pills (horizontal row below search)
    auto catRow = area.removeFromTop (26);
    const int pillWidth = 34;
    const int pillPad = 2;
    int cx = catRow.getX();
    for (auto* btn : categoryButtons)
    {
        btn->setBounds (cx, catRow.getY() + 2, pillWidth, catRow.getHeight() - 6);
        cx += pillWidth + pillPad;
    }
    area.removeFromTop (8);

    // Bank selector row
    auto bankRow = area.removeFromTop (26);
    juce::Label bankLabel ("Bank:", "Bank:");
    bankLabel.setColour (juce::Label::textColourId, textSecondary);
    bankLabel.setBounds (bankRow.removeFromLeft (44));
    addAndMakeVisible (bankLabel);
    bankSelector.setBounds (bankRow.reduced (0, 2));
    area.removeFromTop (6);

    // Name + save row
    auto topRow = area.removeFromTop (26);
    nameLabel.setBounds (topRow.removeFromLeft (44));
    saveButton.setBounds (topRow.removeFromRight (56).reduced (2, 1));
    topRow.removeFromLeft (4);
    nameEditor.setBounds (topRow.reduced (0, 2));
    area.removeFromTop (6);

    // Preset list on left, action buttons on right
    auto btnCol = area.removeFromRight (64);
    loadButton.setBounds   (btnCol.removeFromTop (26).reduced (2, 1));
    btnCol.removeFromTop (4);
    deleteButton.setBounds (btnCol.removeFromTop (26).reduced (2, 1));
    btnCol.removeFromTop (4);
    importButton.setBounds (btnCol.removeFromTop (26).reduced (2, 1));
    btnCol.removeFromTop (4);
    exportButton.setBounds (btnCol.removeFromTop (26).reduced (2, 1));

    area.removeFromRight (4);
    presetList.setBounds (area);
}

//==============================================================================
void PresetBrowserPanel::rebuildFilter()
{
    filteredPresetIndices.clear();
    auto& pm = processorRef.getPresetManager();
    int total = pm.getPresetCount();
    auto allNames = pm.getPresetNames();
    auto searchText = searchEditor.getText().toLowerCase();

    // Determine active category from toggle state
    juce::String activeCat = "All";
    if (!catAll.getToggleState())
    {
        if      (catInit.getToggleState())  activeCat = "Init";
        else if (catBass.getToggleState())  activeCat = "Bass";
        else if (catLead.getToggleState())  activeCat = "Lead";
        else if (catPad.getToggleState())   activeCat = "Pad";
        else if (catDrums.getToggleState()) activeCat = "Drums";
        else if (catFX.getToggleState())    activeCat = "FX";
    }

    for (int i = 0; i < total; ++i)
    {
        juce::String cat  = pm.getPresetCategory(i);
        juce::String name = (i < allNames.size()) ? allNames[i] : juce::String{};

        bool matchCategory = (activeCat == "All") || (cat == activeCat);
        bool matchSearch   = searchText.isEmpty() || name.toLowerCase().contains(searchText);

        if (matchCategory && matchSearch)
            filteredPresetIndices.add(i);
    }

    // Show filtered count (and total when filtered)
    juce::String countText = juce::String(filteredPresetIndices.size());
    if (activeCat != "All" || searchText.isNotEmpty())
        countText += " / " + juce::String(total);
    countText += " presets";
    countLabel.setText(countText, juce::dontSendNotification);

    presetList.updateContent();
    presetList.repaint();
}

//==============================================================================
void PresetBrowserPanel::setActiveCategory (juce::TextButton* cat)
{
    for (auto* btn : categoryButtons)
        btn->setToggleState (btn == cat, juce::dontSendNotification);
    rebuildFilter();
}

//==============================================================================
int PresetBrowserPanel::getNumRows()
{
    return filteredPresetIndices.size();
}

void PresetBrowserPanel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    auto bounds = juce::Rectangle<float> (4.0f, 1.0f, (float)w - 8.0f, (float)h - 2.0f);
    const float corner = 6.0f;

    if (selected)
    {
        // Active: accent border + glow + bright text
        g.setColour (accentBlue.withAlpha (0.25f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentBlue.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.5f);
    }
    else
    {
        // Neumorphic raised card
        drawNeumorphicRect (g, bounds, corner, 2.0f);
        g.setColour (bgRaised);
        g.fillRoundedRectangle (bounds, corner);
    }

    g.setColour (selected ? textPrimary : textSecondary);
    g.setFont (juce::Font (12.0f, juce::Font::plain));
    auto allNames = processorRef.getPresetManager().getPresetNames();
    int presetIndex = filteredPresetIndices[row];
    if (presetIndex >= 0 && presetIndex < allNames.size())
        g.drawText (allNames[presetIndex], (int)bounds.getX() + 10, 0, w - 20, h,
                    juce::Justification::centredLeft);
}

void PresetBrowserPanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int presetIndex = filteredPresetIndices[row];
        if (processorRef.loadPresetAtIndex (presetIndex))
            refresh();
    }
}

//==============================================================================
void PresetBrowserPanel::buttonClicked (juce::Button* b)
{
    if (b == &saveButton)   saveCurrentPreset();
    else if (b == &loadButton)   loadSelectedPreset();
    else if (b == &deleteButton) deleteSelectedPreset();
    else if (b == &importButton) importPreset();
    else if (b == &exportButton) exportPreset();
    else
    {
        // Category button clicked
        for (auto* btn : categoryButtons)
        {
            if (b == btn)
            {
                setActiveCategory (btn);
                return;
            }
        }
    }
}

//==============================================================================
void PresetBrowserPanel::comboBoxChanged (juce::ComboBox*)
{
    processorRef.getPresetManager().setCurrentBank (bankSelector.getSelectedItemIndex());
    rebuildFilter();
}

//==============================================================================
void PresetBrowserPanel::textEditorTextChanged (juce::TextEditor&)
{
    rebuildFilter();
}

void PresetBrowserPanel::textEditorReturnKeyPressed (juce::TextEditor&) {}
void PresetBrowserPanel::textEditorEscapeKeyPressed (juce::TextEditor& ed)
{
    ed.clear();
    rebuildFilter();
}

void PresetBrowserPanel::textEditorFocusLost (juce::TextEditor&) {}

//==============================================================================
void PresetBrowserPanel::saveCurrentPreset()
{
    if (processorRef.getPresetManager().isCurrentBankReadOnly())
        return;

    auto name = nameEditor.getText().trim();
    if (name.isEmpty())
        return;
    processorRef.saveNamedPreset (name);
    nameEditor.clear();
    rebuildFilter();
}

void PresetBrowserPanel::loadSelectedPreset()
{
    int row = presetList.getSelectedRow();
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int presetIndex = filteredPresetIndices[row];
        processorRef.loadPresetAtIndex (presetIndex);
        rebuildFilter();
    }
}

void PresetBrowserPanel::deleteSelectedPreset()
{
    int row = presetList.getSelectedRow();
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int presetIndex = filteredPresetIndices[row];
        processorRef.getPresetManager().deletePreset (presetIndex);
        rebuildFilter();
    }
}

void PresetBrowserPanel::refresh()
{
    rebuildFilter();
}

//==============================================================================
void PresetBrowserPanel::importPreset()
{
    juce::FileChooser chooser ("Import Preset", juce::File(), "*.mvpreset");
    chooser.launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();
            if (!results.isEmpty())
            {
                processorRef.getPresetManager().importPreset (results.getReference (0));
                rebuildFilter();
            }
        });
}

void PresetBrowserPanel::exportPreset()
{
    int row = presetList.getSelectedRow();
    if (row < 0) return;

    juce::FileChooser chooser ("Export Preset", juce::File(), "*.mvpreset");
    chooser.launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, row](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();
            if (!results.isEmpty())
            {
                int presetIndex = filteredPresetIndices[row];
                processorRef.getPresetManager().exportPreset (presetIndex, results.getReference (0));
            }
        });
}
