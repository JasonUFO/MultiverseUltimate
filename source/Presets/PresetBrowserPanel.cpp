#include "PresetBrowserPanel.h"
#include "../PluginProcessor.h"
#include "../MultiverseFlatTheme.h"

namespace
{
    // Use global theme colors via aliases for readability
    const auto& bgBase      = MultiverseFlatTheme::bgBase();
    const auto& bgRaised    = MultiverseFlatTheme::bgRaised();
    const auto& bgDeep      = MultiverseFlatTheme::bgDeep();
    const auto& accentBlue  = MultiverseFlatTheme::accentCyan();
    const auto& accentPurple= MultiverseFlatTheme::accentPurple();
    const auto& textPrimary = MultiverseFlatTheme::textPrimary();
    const auto& textSecondary=MultiverseFlatTheme::textSecondary();
    const auto& textMuted   = MultiverseFlatTheme::textMuted();
    const auto& borderLight = MultiverseFlatTheme::borderLight();
}

const juce::Colour PresetBrowserPanel::favColors[8] = {
    juce::Colour (0xFFFF3B30),  // Red
    juce::Colour (0xFFFF9500),  // Orange
    juce::Colour (0xFFFFCC00),  // Yellow
    juce::Colour (0xFF34C759),  // Green
    juce::Colour (0xFF00C7BE),  // Cyan
    juce::Colour (0xFF007AFF),  // Blue
    juce::Colour (0xFFAF52DE),  // Purple
    juce::Colour (0xFFFF2D55),  // Pink
};

//==============================================================================
PresetBrowserPanel::PresetBrowserPanel(PluginProcessor& p)
    : processorRef(p)
{
    // Search editor
    searchEditor.setTextToShowWhenEmpty ("Search presets... #tag", textMuted);
    searchEditor.setFont (juce::Font (12.0f, juce::Font::plain));
    searchEditor.addListener (this);
    addAndMakeVisible (searchEditor);

    // Category buttons (including virtual Favorites)
    categoryButtons = { &catAll, &catInit, &catBass, &catLead, &catPad, &catDrums, &catFX, &catFavorites };
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

    // Action buttons
    saveButton.addListener (this);
    importButton.addListener (this);
    exportButton.addListener (this);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (importButton);
    addAndMakeVisible (exportButton);

    // Auto-preview toggle
    autoPreviewButton.setToggleState (true, juce::dontSendNotification);
    autoPreviewButton.setColour (juce::ToggleButton::textColourId, textSecondary);
    autoPreviewButton.setColour (juce::ToggleButton::tickColourId, accentBlue);
    autoPreviewButton.addListener (this);
    addAndMakeVisible (autoPreviewButton);

    // Preset list
    presetList.setRowHeight (36);
    presetList.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetList.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (presetList);

    // Count label
    countLabel.setFont (juce::Font (10.0f, juce::Font::plain));
    countLabel.setColour (juce::Label::textColourId, textSecondary);
    countLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (countLabel);

    // Metadata detail strip
    authorLabel.setFont (juce::Font (10.0f, juce::Font::plain));
    authorLabel.setColour (juce::Label::textColourId, textSecondary);
    authorLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (authorLabel);

    descriptionLabel.setFont (juce::Font (10.0f, juce::Font::plain));
    descriptionLabel.setColour (juce::Label::textColourId, textSecondary);
    descriptionLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (descriptionLabel);

    tagsLabel.setFont (juce::Font (10.0f, juce::Font::plain));
    tagsLabel.setColour (juce::Label::textColourId, accentBlue);
    tagsLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (tagsLabel);

    // Tag filter area
    addAndMakeVisible (tagFilterArea);

    // Style action buttons
    for (auto* btn : { &saveButton, &importButton, &exportButton })
    {
        btn->setColour (juce::TextButton::buttonColourId, bgRaised);
        btn->setColour (juce::TextButton::textColourOffId, textSecondary);
    }

    rebuildFilter();
    startTimer (50);
}

//==============================================================================
void PresetBrowserPanel::paint (juce::Graphics& g)
{
    MultiverseFlatTheme::drawContentBackground(g, getLocalBounds().toFloat());

    // Tag filter area background
    if (activeTagFilters.size() > 0)
    {
        auto ta = tagFilterArea.getBounds().toFloat().reduced (2, 2);
        MultiverseFlatTheme::drawCard (g, ta, 6.0f, false, bgDeep);
    }

    // Metadata strip background
    auto metaArea = getLocalBounds().removeFromBottom (48).toFloat().reduced (6, 4);
    MultiverseFlatTheme::drawCard (g, metaArea, 8.0f);
}

//==============================================================================
void PresetBrowserPanel::resized()
{
    auto area = getLocalBounds().reduced (6, 6);

    // Search bar row (28px) — compact for 280px sidebar
    auto searchBar = area.removeFromTop (28);
    countLabel.setBounds (searchBar.removeFromRight (48).reduced (0, 6));
    autoPreviewButton.setBounds (searchBar.removeFromRight (36).reduced (2, 2));
    exportButton.setBounds  (searchBar.removeFromRight (28).reduced (2, 2));
    importButton.setBounds   (searchBar.removeFromRight (28).reduced (2, 2));
    saveButton.setBounds    (searchBar.removeFromRight (28).reduced (2, 2));
    searchEditor.setBounds (searchBar.reduced (0, 2));
    area.removeFromTop (6);

    // Category pills row (26px) — compact for 280px sidebar
    auto catRow = area.removeFromTop (26);
    const int pillWidth = 28;
    const int pillPad = 1;
    int cx = catRow.getX();
    for (auto* btn : categoryButtons)
    {
        int w = (btn == &catFavorites) ? 30 : pillWidth;
        btn->setBounds (cx, catRow.getY() + 2, w, catRow.getHeight() - 6);
        cx += w + pillPad;
    }
    area.removeFromTop (6);

    // Tag filter area (24px, shown when filters active)
    tagFilterArea.setBounds (area.removeFromTop (activeTagFilters.size() > 0 ? 24 : 0));
    if (activeTagFilters.size() > 0)
        area.removeFromTop (4);

    // Metadata detail strip at bottom (48px)
    auto metaArea = area.removeFromBottom (48);
    auto metaRow1 = metaArea.removeFromTop (18);
    authorLabel.setBounds (metaRow1.reduced (10, 2));
    auto metaRow2 = metaArea.removeFromTop (16);
    tagsLabel.setBounds (metaRow2.removeFromRight (metaRow2.getWidth() / 2).reduced (10, 0));
    descriptionLabel.setBounds (metaRow2.reduced (10, 0));

    // Preset list fills remaining space
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

    // Check for #tag in search text and extract
    juce::String textSearch = searchText;
    juce::StringArray searchTags;
    if (searchText.contains("#"))
    {
        auto words = juce::StringArray::fromTokens(searchText, " ", "");
        juce::StringArray remaining;
        for (auto& w : words)
        {
            if (w.startsWith("#") && w.length() > 1)
            {
                auto tag = w.substring(1);
                if (tag.isNotEmpty())
                    searchTags.add(tag);
                else
                    remaining.add(w);
            }
            else
            {
                remaining.add(w);
            }
        }
        textSearch = remaining.joinIntoString(" ").toLowerCase();
    }

    // Determine active category
    juce::String activeCat = "All";
    bool showFavorites = false;
    if (!catAll.getToggleState())
    {
        if      (catInit.getToggleState())     activeCat = "Init";
        else if (catBass.getToggleState())     activeCat = "Bass";
        else if (catLead.getToggleState())     activeCat = "Lead";
        else if (catPad.getToggleState())      activeCat = "Pad";
        else if (catDrums.getToggleState())    activeCat = "Drums";
        else if (catFX.getToggleState())       activeCat = "FX";
        else if (catFavorites.getToggleState()){ showFavorites = true; }
    }

    for (int i = 0; i < total; ++i)
    {
        const auto& meta = pm.getPresetMetadata(i);
        juce::String name = (i < allNames.size()) ? allNames[i] : juce::String{};

        // Category filter
        bool matchCategory = (activeCat == "All") || (meta.category == activeCat);
        if (showFavorites)
            matchCategory = pm.isFavorite(i);

        // Search text filter
        bool matchSearch = textSearch.isEmpty() ||
            name.toLowerCase().contains(textSearch) ||
            meta.author.toLowerCase().contains(textSearch) ||
            meta.description.toLowerCase().contains(textSearch);

        // Tag filters (AND logic — must match all active tags)
        bool matchTags = true;
        auto allActiveTags = activeTagFilters;
        for (auto& st : searchTags)
            if (!allActiveTags.contains(st))
                allActiveTags.add(st);

        if (allActiveTags.size() > 0)
        {
            for (const auto& tag : allActiveTags)
            {
                bool hasTag = false;
                for (const auto& pt : meta.tags)
                {
                    if (pt.toLowerCase() == tag.toLowerCase())
                    { hasTag = true; break; }
                }
                if (!hasTag) { matchTags = false; break; }
            }
        }

        if (matchCategory && matchSearch && matchTags)
            filteredPresetIndices.add(i);
    }

    // Update count label
    juce::String countText = juce::String(filteredPresetIndices.size());
    if (activeCat != "All" || searchText.isNotEmpty() || activeTagFilters.size() > 0 || showFavorites)
        countText += " / " + juce::String(total);
    countText += " presets";
    countLabel.setText(countText, juce::dontSendNotification);

    // Update tag filter area visibility
    tagFilterArea.setVisible(activeTagFilters.size() > 0);
    resized();

    presetList.updateContent();
    presetList.repaint();

    updateMetadataStrip();
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
        g.setColour (accentBlue.withAlpha (0.25f));
        g.fillRoundedRectangle (bounds, corner);
        g.setColour (accentBlue.withAlpha (0.6f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.5f);
    }
    else
    {
        MultiverseFlatTheme::drawCard(g, bounds, corner);
    }

    int presetIndex = filteredPresetIndices[row];
    auto& pm = processorRef.getPresetManager();
    auto allNames = pm.getPresetNames();

    // Favorite color dot
    int favCol = pm.getFavoriteColor(presetIndex);
    if (favCol >= 0)
    {
        g.setColour (favColors[favCol]);
        g.fillEllipse (10.0f, (float)h * 0.5f - 4.0f, 8.0f, 8.0f);
    }

    // Preset name
    g.setColour (selected ? textPrimary : textSecondary);
    g.setFont (juce::Font (12.0f, juce::Font::plain));
    juce::String name = (presetIndex >= 0 && presetIndex < allNames.size()) ? allNames[presetIndex] : juce::String{};
    g.drawText (name, 24, 0, w - 80, h, juce::Justification::centredLeft);

    // Category (right-aligned)
    const auto& meta = pm.getPresetMetadata(presetIndex);
    g.setColour (textMuted);
    g.setFont (juce::Font (9.0f, juce::Font::plain));
    g.drawText (meta.category, w - 70, 0, 46, h, juce::Justification::centredRight);

    // Author (far right)
    g.drawText (meta.author, w - 30, 0, 26, h, juce::Justification::centredRight);
}

void PresetBrowserPanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int presetIndex = filteredPresetIndices[row];
        processorRef.cancelPreviewNote();
        processorRef.loadPresetAtIndex (presetIndex);
        refresh();
    }
}

void PresetBrowserPanel::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    // Right-click context menu
    if (e.mods.isRightButtonDown() && row >= 0)
        showRightClickMenu(row);
    else
        updateMetadataStrip();
}

//==============================================================================
void PresetBrowserPanel::buttonClicked (juce::Button* b)
{
    if (b == &saveButton)        saveCurrentPreset();
    else if (b == &importButton)  importPreset();
    else if (b == &exportButton)  exportPreset();
    else if (b == &autoPreviewButton)
    {
        autoPreviewEnabled = autoPreviewButton.getToggleState();
        if (!autoPreviewEnabled)
            processorRef.cancelPreviewNote();
    }
    else
    {
        // Category button
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
    rebuildFilter();
}

//==============================================================================
void PresetBrowserPanel::textEditorTextChanged (juce::TextEditor&)
{
    rebuildFilter();
}

void PresetBrowserPanel::textEditorReturnKeyPressed (juce::TextEditor&)
{
    // If search contains #tag, extract and add as filter
    auto text = searchEditor.getText();
    if (text.contains("#"))
    {
        auto words = juce::StringArray::fromTokens(text, " ", "");
        for (auto& w : words)
        {
            if (w.startsWith("#") && w.length() > 1)
            {
                auto tag = w.substring(1).toLowerCase();
                if (tag.isNotEmpty() && !activeTagFilters.contains(tag))
                    activeTagFilters.add(tag);
            }
        }
        // Remove #tags from search text
        juce::StringArray remaining;
        for (auto& w : words)
            if (!w.startsWith("#"))
                remaining.add(w);
        searchEditor.setText (remaining.joinIntoString(" "), juce::dontSendNotification);
        rebuildFilter();
    }
}

void PresetBrowserPanel::textEditorEscapeKeyPressed (juce::TextEditor& ed)
{
    ed.clear();
    rebuildFilter();
}

void PresetBrowserPanel::textEditorFocusLost (juce::TextEditor&) {}

//==============================================================================
void PresetBrowserPanel::timerCallback()
{
    // Auto-preview: check if selected row changed
    if (!autoPreviewEnabled)
        return;

    int row = presetList.getSelectedRow();
    if (row != lastPreviewedRow && row >= 0 && row < filteredPresetIndices.size())
    {
        lastPreviewedRow = row;
        // Load preset for preview (but don't commit it)
        int presetIndex = filteredPresetIndices[row];
        // Trigger audio preview note
        processorRef.triggerPreviewNote();
    }
}

//==============================================================================
void PresetBrowserPanel::saveCurrentPreset()
{
    showSaveDialog();
}

void PresetBrowserPanel::loadSelectedPreset()
{
    int row = presetList.getSelectedRow();
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int presetIndex = filteredPresetIndices[row];
        processorRef.cancelPreviewNote();
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

void PresetBrowserPanel::focusSearchEditor()
{
    searchEditor.grabKeyboardFocus();
    searchEditor.selectAll();
}

int PresetBrowserPanel::getSelectedPresetIndex() const
{
    int row = presetList.getSelectedRow();
    if (row >= 0 && row < filteredPresetIndices.size())
        return filteredPresetIndices[row];
    return -1;
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

//==============================================================================
void PresetBrowserPanel::updateMetadataStrip()
{
    int row = presetList.getSelectedRow();
    if (row >= 0 && row < filteredPresetIndices.size())
    {
        int idx = filteredPresetIndices[row];
        const auto& meta = processorRef.getPresetManager().getPresetMetadata(idx);
        authorLabel.setText ("Author: " + meta.author, juce::dontSendNotification);
        descriptionLabel.setText (meta.description.isNotEmpty() ? meta.description : "No description", juce::dontSendNotification);
        tagsLabel.setText (meta.tags.size() > 0 ? ("#" + meta.tags.joinIntoString(" #")) : "", juce::dontSendNotification);
    }
    else
    {
        authorLabel.setText ("", juce::dontSendNotification);
        descriptionLabel.setText ("", juce::dontSendNotification);
        tagsLabel.setText ("", juce::dontSendNotification);
    }
}

//==============================================================================
void PresetBrowserPanel::addTagFilter(const juce::String& tag)
{
    if (!activeTagFilters.contains(tag))
    {
        activeTagFilters.add(tag);
        rebuildFilter();
    }
}

void PresetBrowserPanel::removeTagFilter(const juce::String& tag)
{
    activeTagFilters.removeString(tag);
    rebuildFilter();
}

void PresetBrowserPanel::clearTagFilters()
{
    activeTagFilters.clear();
    rebuildFilter();
}

//==============================================================================
void PresetBrowserPanel::showSaveDialog()
{
    if (processorRef.getPresetManager().isCurrentBankReadOnly())
        return;

    auto* dialog = new juce::Component();
    dialog->setSize (300, 200);

    auto* nameEd = new juce::TextEditor();
    nameEd->setTextToShowWhenEmpty ("Preset name...", textMuted);
    nameEd->setFont (juce::Font (13.0f));
    nameEd->setBounds (70, 10, 220, 24);
    dialog->addAndMakeVisible (nameEd);

    auto* nameLb = new juce::Label ("Name", "Name:");
    nameLb->setColour (juce::Label::textColourId, textSecondary);
    nameLb->setBounds (10, 10, 55, 24);
    nameLb->setJustificationType (juce::Justification::centredRight);
    dialog->addAndMakeVisible (nameLb);

    auto* catCb = new juce::ComboBox();
    catCb->addItemList ({"Init", "Bass", "Lead", "Pad", "Drums", "FX"}, 1);
    // Set current category
    juce::String curCat = processorRef.currentPresetCategory;
    int catIdx = juce::StringArray({"Init", "Bass", "Lead", "Pad", "Drums", "FX"}).indexOf(curCat);
    if (catIdx < 0) catIdx = 0;
    catCb->setSelectedItemIndex (catIdx, juce::dontSendNotification);
    catCb->setBounds (70, 40, 220, 24);
    dialog->addAndMakeVisible (catCb);

    auto* catLb = new juce::Label ("Cat", "Category:");
    catLb->setColour (juce::Label::textColourId, textSecondary);
    catLb->setBounds (10, 40, 55, 24);
    catLb->setJustificationType (juce::Justification::centredRight);
    dialog->addAndMakeVisible (catLb);

    auto* tagsEd = new juce::TextEditor();
    tagsEd->setText (processorRef.currentPresetTags);
    tagsEd->setTextToShowWhenEmpty ("acid, warm, dark...", textMuted);
    tagsEd->setFont (juce::Font (13.0f));
    tagsEd->setBounds (70, 70, 220, 24);
    dialog->addAndMakeVisible (tagsEd);

    auto* tagsLb = new juce::Label ("Tags", "Tags:");
    tagsLb->setColour (juce::Label::textColourId, textSecondary);
    tagsLb->setBounds (10, 70, 55, 24);
    tagsLb->setJustificationType (juce::Justification::centredRight);
    dialog->addAndMakeVisible (tagsLb);

    auto* descEd = new juce::TextEditor();
    descEd->setText (processorRef.currentPresetDescription);
    descEd->setTextToShowWhenEmpty ("Description...", textMuted);
    descEd->setFont (juce::Font (13.0f));
    descEd->setBounds (70, 100, 220, 48);
    descEd->setMultiLine (true);
    dialog->addAndMakeVisible (descEd);

    auto* descLb = new juce::Label ("Desc", "Desc:");
    descLb->setColour (juce::Label::textColourId, textSecondary);
    descLb->setBounds (10, 100, 55, 24);
    descLb->setJustificationType (juce::Justification::centredRight);
    dialog->addAndMakeVisible (descLb);

    auto* saveBtn = new juce::TextButton ("Save");
    saveBtn->setBounds (100, 160, 100, 28);
    saveBtn->setColour (juce::TextButton::buttonColourId, accentBlue);
    saveBtn->setColour (juce::TextButton::textColourOffId, textPrimary);
    dialog->addAndMakeVisible (saveBtn);

    saveBtn->onClick = [this, dialog, nameEd, catCb, tagsEd, descEd]() mutable
    {
        auto name = nameEd->getText().trim();
        if (name.isEmpty())
            return;

        // Update processor metadata before saving
        processorRef.currentPresetCategory = catCb->getText();
        processorRef.currentPresetTags = tagsEd->getText();
        processorRef.currentPresetDescription = descEd->getText();

        processorRef.saveNamedPreset (name);
        delete dialog;
        rebuildFilter();
    };

    auto& box = juce::CallOutBox::launchAsynchronously (std::unique_ptr<juce::Component>(dialog),
                                                         getScreenBounds(), nullptr);
    (void)box;
}

//==============================================================================
void PresetBrowserPanel::showRightClickMenu(int row)
{
    if (row < 0 || row >= filteredPresetIndices.size())
        return;

    int presetIndex = filteredPresetIndices[row];
    auto& pm = processorRef.getPresetManager();

    juce::PopupMenu menu;
    menu.addItem ("Load", [this, presetIndex]()
    {
        processorRef.cancelPreviewNote();
        processorRef.loadPresetAtIndex (presetIndex);
        refresh();
    });

    menu.addItem ("Delete", [this, presetIndex]()
    {
        processorRef.getPresetManager().deletePreset (presetIndex);
        rebuildFilter();
    });

    menu.addItem ("Export", [this, presetIndex]()
    {
        juce::FileChooser chooser ("Export Preset", juce::File(), "*.mvpreset");
        chooser.launchAsync (juce::FileBrowserComponent::saveMode,
            [this, presetIndex](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (!results.isEmpty())
                    processorRef.getPresetManager().exportPreset (presetIndex, results.getReference (0));
            });
    });

    menu.addSeparator();

    // Favorites submenu
    juce::PopupMenu favMenu;
    int currentColor = pm.getFavoriteColor(presetIndex);

    static const char* colorNames[] = {"Red", "Orange", "Yellow", "Green", "Cyan", "Blue", "Purple", "Pink"};

    for (int i = 0; i < 8; ++i)
    {
        bool isCurrent = (currentColor == i);
        favMenu.addItem (juce::String(colorNames[i]) + (isCurrent ? " *" : ""),
            [this, presetIndex, i]()
        {
            processorRef.getPresetManager().setFavorite(presetIndex, i);
            rebuildFilter();
        });
    }

    if (currentColor >= 0)
    {
        favMenu.addSeparator();
        favMenu.addItem ("Clear Favorite", [this, presetIndex]()
        {
            processorRef.getPresetManager().setFavorite(presetIndex, -1);
            rebuildFilter();
        });
    }

    menu.addSubMenu ("Favorite", favMenu);

    menu.showMenuAsync (juce::PopupMenu::Options());
}