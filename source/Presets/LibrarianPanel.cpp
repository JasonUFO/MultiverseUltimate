#include "LibrarianPanel.h"
#include "../PluginProcessor.h"
#include "../MultiverseFlatTheme.h"

namespace
{
    const auto& bgBase      = MultiverseFlatTheme::bgBase;
    const auto& bgRaised    = MultiverseFlatTheme::bgRaised;
    const auto& bgDeep      = MultiverseFlatTheme::bgDeep;
    const auto& accentCyan  = MultiverseFlatTheme::accentCyan;
    const auto& accentPink  = MultiverseFlatTheme::accentPink;
    const auto& accentAmber = MultiverseFlatTheme::accentAmber;
    const auto& textPrimary = MultiverseFlatTheme::textPrimary;
    const auto& textSecondary=MultiverseFlatTheme::textSecondary;
    const auto& textMuted   = MultiverseFlatTheme::textMuted;
    const auto& borderLight = MultiverseFlatTheme::borderLight;

    static const juce::StringArray factoryCategories = { "Init", "Bass", "Lead", "Pad", "Drums", "FX", "Keys", "Arp" };
}

const juce::Colour LibrarianPanel::favColors[8] = {
    juce::Colour (0xFFFF3B30),
    juce::Colour (0xFFFF9500),
    juce::Colour (0xFFFFCC00),
    juce::Colour (0xFF34C759),
    juce::Colour (0xFF00C7BE),
    juce::Colour (0xFF007AFF),
    juce::Colour (0xFFAF52DE),
    juce::Colour (0xFFFF2D55),
};

//==============================================================================
// LibrarianPresetList
//==============================================================================

LibrarianPresetList::LibrarianPresetList(LibrarianPanel& o)
    : owner(o)
{
    setMouseClickGrabsKeyboardFocus(true);
}

void LibrarianPresetList::rebuildSections()
{
    sections.clearQuick();
    selectedPresetIndex = owner.getSelectedPresetIndex();

    auto& pm = owner.processorRef.getPresetManager();
    int total = pm.getPresetCount();
    auto allNames = pm.getPresetNames();

    // Collect active filters from owner
    juce::String activeCat = owner.activeCategory;
    bool favOnly = owner.favFilterActive;
    const auto& tagFilters = owner.activeTagFilters;

    juce::String searchText = owner.searchEditor.getText().toLowerCase();
    juce::StringArray searchTags;
    juce::String textSearch = searchText;
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

    // Character filters
    juce::StringArray includeChars, excludeChars;
    for (auto* cp : owner.characterPills)
    {
        if (cp->state == LibrarianPanel::CharFilter::Include)
            includeChars.add(cp->character);
        else if (cp->state == LibrarianPanel::CharFilter::Exclude)
            excludeChars.add(cp->character);
    }

    // Helper: check if a preset index passes all filters
    auto passesFilter = [&](int i) -> bool
    {
        const auto& meta = pm.getPresetMetadata(i);
        juce::String name = (i < allNames.size()) ? allNames[i] : juce::String{};

        bool matchCategory = (activeCat == "All") || (meta.category == activeCat);
        bool matchFavorite = !favOnly || pm.isFavorite(i);
        bool matchSearch = textSearch.isEmpty() ||
            name.toLowerCase().contains(textSearch) ||
            meta.author.toLowerCase().contains(textSearch) ||
            meta.description.toLowerCase().contains(textSearch);

        bool matchTags = true;
        auto allActiveTags = tagFilters;
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

        bool matchCharacters = true;
        for (const auto& ch : includeChars)
        {
            if (!meta.characters.contains(ch))
            { matchCharacters = false; break; }
        }
        if (matchCharacters)
        {
            for (const auto& ch : excludeChars)
            {
                if (meta.characters.contains(ch))
                { matchCharacters = false; break; }
            }
        }

        return matchCategory && matchFavorite && matchSearch && matchTags && matchCharacters;
    };

    // Build Factory section with sub-sections per category
    {
        Section sec;
        sec.title = "FACTORY";
        sec.isExpanded = true;
        sec.isBookmark = false;

        for (auto& cat : factoryCategories)
        {
            Section::SubSection sub;
            sub.title = cat;
            sub.isExpanded = (activeCat == "All" || activeCat == cat);

            for (int i = 0; i < total; ++i)
            {
                const auto& meta = pm.getPresetMetadata(i);
                if (meta.category == cat && passesFilter(i))
                    sub.presetIndices.add(i);
            }

            if (sub.presetIndices.size() > 0)
                sec.subSections.add(sub);
        }

        sections.add(sec);
    }

    // Build User Presets section
    {
        Section sec;
        sec.title = "USER PRESETS";
        sec.isExpanded = true;
        sec.isBookmark = false;
        Section::SubSection sub;
        sub.title = "User";
        sub.isExpanded = true;

        for (int i = 0; i < total; ++i)
        {
            const auto& meta = pm.getPresetMetadata(i);
            if (!factoryCategories.contains(meta.category) && passesFilter(i))
                sub.presetIndices.add(i);
        }

        sec.subSections.add(sub);
        sections.add(sec);
    }

    // Build Bookmarks section
    {
        Section sec;
        sec.title = "BOOKMARKS";
        sec.isExpanded = false;
        sec.isBookmark = true;

        auto& folders = pm.getBookmarkFolders();
        for (size_t f = 0; f < folders.size(); ++f)
        {
            Section::SubSection bmSub;
            bmSub.title = folders[f].name;
            bmSub.isExpanded = false;
            bmSub.bookmarkFolderIndex = static_cast<int>(f);

            for (auto& relPath : folders[f].presetPaths)
            {
                for (int i = 0; i < total; ++i)
                {
                    if (pm.getRelativePresetPath(i) == relPath && passesFilter(i))
                    {
                        bmSub.presetIndices.add(i);
                        break;
                    }
                }
            }

            sec.subSections.add(bmSub);
        }

        if (folders.size() > 0)
            sections.add(sec);
    }

    // Calculate total height and resize
    int totalH = 0;
    for (auto& sec : sections)
    {
        totalH += sectionHeaderH;
        if (sec.isExpanded)
        {
            for (auto& sub : sec.subSections)
            {
                totalH += subSectionHeaderH;
                if (sub.isExpanded)
                    totalH += sub.presetIndices.size() * presetRowH;
            }
        }
    }

    setSize(getWidth(), jmax(totalH, getParentHeight()));
    repaint();
}

void LibrarianPresetList::paint(juce::Graphics& g)
{
    g.fillAll(bgBase);

    int y = 0;
    int w = getWidth();

    for (int s = 0; s < sections.size(); ++s)
    {
        auto& sec = sections.getReference(s);

        // Section header
        drawSectionHeader(g, juce::Rectangle<int>(0, y, w, sectionHeaderH), sec.title, sec.isExpanded);
        y += sectionHeaderH;

        if (sec.isExpanded)
        {
            for (int sub = 0; sub < sec.subSections.size(); ++sub)
            {
                auto& subSec = sec.subSections.getReference(sub);

                drawSubSectionHeader(g, juce::Rectangle<int>(indent, y, w - indent, subSectionHeaderH), subSec.title, subSec.isExpanded);
                y += subSectionHeaderH;

                if (subSec.isExpanded)
                {
                    for (int p = 0; p < subSec.presetIndices.size(); ++p)
                    {
                        int presetIdx = subSec.presetIndices[p];
                        bool sel = (presetIdx == selectedPresetIndex);
                        bool hov = (presetIdx == hoverPresetIndex);
                        drawPresetRow(g, juce::Rectangle<int>(0, y, w, presetRowH), presetIdx, sel, hov);
                        y += presetRowH;
                    }
                }
            }
        }
    }
}

void LibrarianPresetList::mouseDown(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();
    auto zone = hitTest(pos);

    if (zone.type == HitZone::kSectionHeader)
    {
        if (e.mods.isRightButtonDown() && zone.sectionIdx >= 0)
        {
            auto& sec = sections.getReference(zone.sectionIdx);
            if (sec.isBookmark)
            {
                // Right-click on Bookmarks section header → new folder
                owner.promptNewBookmarkFolder();
                return;
            }
        }
        sections.getReference(zone.sectionIdx).isExpanded =
            !sections.getReference(zone.sectionIdx).isExpanded;
        rebuildSections();
    }
    else if (zone.type == HitZone::kSubSectionHeader)
    {
        if (e.mods.isRightButtonDown() && zone.isInBookmarkSection && zone.bookmarkFolderIdx >= 0)
        {
            // Right-click on bookmark subsection header → folder management
            owner.showBookmarkFolderMenu(zone.bookmarkFolderIdx, e.getScreenPosition());
            return;
        }
        auto& sub = sections.getReference(zone.sectionIdx).subSections.getReference(zone.subIdx);
        sub.isExpanded = !sub.isExpanded;
        rebuildSections();
    }
    else if (zone.type == HitZone::kPresetRow)
    {
        selectedPresetIndex = zone.presetIndex;
        owner.updateMetadataStrip();

        if (e.mods.isRightButtonDown())
        {
            if (zone.isInBookmarkSection)
                owner.showPresetContextMenu(zone.presetIndex, e.getScreenPosition(), zone.bookmarkFolderIdx);
            else
                owner.showPresetContextMenu(zone.presetIndex, e.getScreenPosition(), -1);
        }
        else
            repaint();
    }
}

void LibrarianPresetList::mouseDoubleClick(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();
    auto zone = hitTest(pos);

    if (zone.type == HitZone::kPresetRow)
    {
        selectedPresetIndex = zone.presetIndex;
        owner.loadPresetByIndex(zone.presetIndex);
    }
}

void LibrarianPresetList::mouseMove(const juce::MouseEvent& e)
{
    auto pos = e.getPosition();
    auto zone = hitTest(pos);

    int newHover = (zone.type == HitZone::kPresetRow) ? zone.presetIndex : -1;
    if (newHover != hoverPresetIndex)
    {
        hoverPresetIndex = newHover;
        repaint();
    }
}

LibrarianPresetList::HitZone LibrarianPresetList::hitTest(const juce::Point<int>& pos) const
{
    int y = 0;
    for (int s = 0; s < sections.size(); ++s)
    {
        auto& sec = sections.getReference(s);
        if (pos.y >= y && pos.y < y + sectionHeaderH)
            return { HitZone::kSectionHeader, s, -1, -1, sec.isBookmark, sec.bookmarkFolderIndex };
        y += sectionHeaderH;

        if (sec.isExpanded)
        {
            for (int sub = 0; sub < sec.subSections.size(); ++sub)
            {
                auto& subSec = sec.subSections.getReference(sub);
                if (pos.y >= y && pos.y < y + subSectionHeaderH)
                {
                    return { HitZone::kSubSectionHeader, s, sub, -1, sec.isBookmark, subSec.bookmarkFolderIndex };
                }
                y += subSectionHeaderH;

                if (subSec.isExpanded)
                {
                    for (int p = 0; p < subSec.presetIndices.size(); ++p)
                    {
                        if (pos.y >= y && pos.y < y + presetRowH)
                            return { HitZone::kPresetRow, s, sub, subSec.presetIndices[p], sec.isBookmark, subSec.bookmarkFolderIndex };
                        y += presetRowH;
                    }
                }
            }
        }
    }

    return { HitZone::kNothing, -1, -1, -1, false, -1 };
}

void LibrarianPresetList::drawSectionHeader(juce::Graphics& g,
    const juce::Rectangle<int>& bounds, const juce::String& title, bool expanded)
{
    g.setColour(bgDeep);
    g.fillRect(bounds);

    g.setColour(textSecondary);
    g.setFont(MultiverseFlatTheme::headerFont());
    g.drawText(title, bounds.getX() + 6, bounds.getY(), bounds.getWidth() - 20, bounds.getHeight(),
        juce::Justification::centredLeft);

    // Disclosure triangle
    float triX = bounds.getRight() - 16.0f;
    float triY = bounds.getCentreY();
    g.setColour(accentCyan);
    juce::Path tri;
    if (expanded)
    {
        tri.addTriangle(triX - 4, triY - 3, triX + 4, triY - 3, triX, triY + 3);
    }
    else
    {
        tri.addTriangle(triX - 3, triY - 4, triX - 3, triY + 4, triX + 3, triY);
    }
    g.fillPath(tri);
}

void LibrarianPresetList::drawSubSectionHeader(juce::Graphics& g,
    const juce::Rectangle<int>& bounds, const juce::String& title, bool expanded)
{
    g.setColour(bgBase);
    g.fillRect(bounds);

    g.setColour(textMuted);
    g.setFont(MultiverseFlatTheme::labelFont());
    g.drawText(title, bounds.getX() + 6, bounds.getY(), bounds.getWidth() - 14, bounds.getHeight(),
        juce::Justification::centredLeft);

    // Small disclosure triangle
    float triX = bounds.getRight() - 12.0f;
    float triY = bounds.getCentreY();
    g.setColour(textMuted);
    juce::Path tri;
    if (expanded)
    {
        tri.addTriangle(triX - 3, triY - 2, triX + 3, triY - 2, triX, triY + 2);
    }
    else
    {
        tri.addTriangle(triX - 2, triY - 3, triX - 2, triY + 3, triX + 2, triY);
    }
    g.fillPath(tri);
}

void LibrarianPresetList::drawPresetRow(juce::Graphics& g,
    const juce::Rectangle<int>& bounds, int presetIndex, bool selected, bool hovered)
{
    auto& pm = owner.processorRef.getPresetManager();
    auto allNames = pm.getPresetNames();

    auto rowBounds = bounds.toFloat().reduced(4.0f, 1.0f);
    float corner = 5.0f;

    if (selected)
    {
        g.setColour(accentCyan.withAlpha(0.25f));
        g.fillRoundedRectangle(rowBounds, corner);
        g.setColour(accentCyan.withAlpha(0.6f));
        g.drawRoundedRectangle(rowBounds.reduced(0.5f), corner, 1.5f);
    }
    else if (hovered)
    {
        g.setColour(bgRaised);
        g.fillRoundedRectangle(rowBounds, corner);
    }

    // Favorite dot
    int favCol = pm.getFavoriteColor(presetIndex);
    if (favCol >= 0)
    {
        g.setColour(LibrarianPanel::favColors[favCol]);
        g.fillEllipse(bounds.getX() + indent + 4.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);
    }
    else
    {
        g.setColour(textMuted);
        g.drawEllipse(bounds.getX() + indent + 4.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f, 0.8f);
    }

    // Preset name
    juce::String name = (presetIndex >= 0 && presetIndex < allNames.size()) ? allNames[presetIndex] : juce::String{};
    g.setColour(selected ? textPrimary : textSecondary);
    g.setFont(MultiverseFlatTheme::titleFont());
    g.drawText(name, bounds.getX() + indent + 16, bounds.getY(), bounds.getWidth() - indent - 70, bounds.getHeight(),
        juce::Justification::centredLeft);

    // Category (right-aligned)
    const auto& meta = pm.getPresetMetadata(presetIndex);
    g.setColour(textMuted);
    g.setFont(MultiverseFlatTheme::labelFont());
    g.drawText(meta.category, bounds.getWidth() - 56, bounds.getY(), 46, bounds.getHeight(),
        juce::Justification::centredRight);
}

void LibrarianPresetList::selectPreset(int presetIndex)
{
    selectedPresetIndex = presetIndex;
    repaint();
}

//==============================================================================
// LibrarianPanel
//==============================================================================

LibrarianPanel::LibrarianPanel(PluginProcessor& p)
    : processorRef(p), presetListContent(*this)
{
    // History nav buttons
    for (auto* btn : { &historyBack, &historyForward })
    {
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn->setColour(juce::TextButton::textColourOffId, textSecondary);
        btn->addListener(this);
        addAndMakeVisible(btn);
    }

    // Search editor
    searchEditor.setTextToShowWhenEmpty("Search... #tag", textMuted);
    searchEditor.setFont(MultiverseFlatTheme::titleFont());
    searchEditor.addListener(this);
    addAndMakeVisible(searchEditor);

    // Category buttons
    categoryButtons = { &catAll, &catBass, &catLead, &catPad, &catDrums, &catFX, &catKeys, &catArp };
    for (auto* btn : categoryButtons)
    {
        btn->setClickingTogglesState(true);
        btn->setRadioGroupId(101);
        btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        btn->setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
        btn->setColour(juce::TextButton::textColourOffId, textSecondary);
        btn->setColour(juce::TextButton::textColourOnId, textPrimary);
        btn->addListener(this);
        addAndMakeVisible(btn);
    }
    catAll.setToggleState(true, juce::dontSendNotification);

    // Character pills
    charDark.character   = "dark";
    charBright.character = "bright";
    charActive.character = "active";
    charSpiky.character  = "spiky";
    charWide.character   = "wide";
    charDirty.character  = "dirty";

    characterPills = { &charDark, &charBright, &charActive, &charSpiky, &charWide, &charDirty };

    for (auto* cp : characterPills)
    {
        cp->button.setClickingTogglesState(false);
        cp->button.setColour(juce::TextButton::buttonColourId, bgDeep);
        cp->button.setColour(juce::TextButton::textColourOffId, textMuted);
        cp->button.addListener(this);
        addAndMakeVisible(cp->button);
    }
    charDark.button.setButtonText("Dark");
    charBright.button.setButtonText("Bright");
    charActive.button.setButtonText("Active");
    charSpiky.button.setButtonText("Spiky");
    charWide.button.setButtonText("Wide");
    charDirty.button.setButtonText("Dirty");

    // Favorite filter button
    favFilterButton.setClickingTogglesState(false);
    favFilterButton.setColour(juce::TextButton::buttonColourId, bgDeep);
    favFilterButton.setColour(juce::TextButton::textColourOffId, textMuted);
    favFilterButton.addListener(this);
    addAndMakeVisible(favFilterButton);

    // Auto-preview toggle
    autoPreviewButton.setToggleState(true, juce::dontSendNotification);
    autoPreviewButton.setColour(juce::ToggleButton::textColourId, textSecondary);
    autoPreviewButton.setColour(juce::ToggleButton::tickColourId, accentCyan);
    autoPreviewButton.addListener(this);
    addAndMakeVisible(autoPreviewButton);

    // Preset list viewport
    presetListViewport.setViewedComponent(&presetListContent, false);
    presetListViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(presetListViewport);

    // Tag filter area
    addAndMakeVisible(tagFilterArea);

    // Metadata detail strip
    authorLabel.setFont(MultiverseFlatTheme::labelFont());
    authorLabel.setColour(juce::Label::textColourId, textSecondary);
    authorLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(authorLabel);

    descriptionLabel.setFont(MultiverseFlatTheme::labelFont());
    descriptionLabel.setColour(juce::Label::textColourId, textSecondary);
    descriptionLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(descriptionLabel);

    tagsLabel.setFont(MultiverseFlatTheme::labelFont());
    tagsLabel.setColour(juce::Label::textColourId, accentCyan);
    tagsLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(tagsLabel);

    charactersLabel.setFont(MultiverseFlatTheme::labelFont());
    charactersLabel.setColour(juce::Label::textColourId, accentAmber);
    charactersLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(charactersLabel);

    rebuildFilter();
    startTimer(50);
}

//==============================================================================
void LibrarianPanel::paint(juce::Graphics& g)
{
    g.fillAll(bgBase);

    // Tag filter area background
    if (activeTagFilters.size() > 0)
    {
        auto ta = tagFilterArea.getBounds().toFloat().reduced(2, 2);
        MultiverseFlatTheme::drawCard(g, ta, 6.0f);
        g.setColour(bgDeep);
        g.fillRoundedRectangle(ta, 6.0f);
    }

    // Metadata strip background
    auto metaArea = getLocalBounds().removeFromBottom(48).toFloat().reduced(6, 4);
    MultiverseFlatTheme::drawCard(g, metaArea, 8.0f);
    g.setColour(bgRaised);
    g.fillRoundedRectangle(metaArea, 8.0f);
}

//==============================================================================
void LibrarianPanel::resized()
{
    auto area = getLocalBounds().reduced(6, 6);

    // History + Search row (28px)
    auto searchBar = area.removeFromTop(28);
    autoPreviewButton.setBounds(searchBar.removeFromRight(28).reduced(2, 2));
    historyForward.setBounds(searchBar.removeFromRight(20).reduced(0, 2));
    historyBack.setBounds(searchBar.removeFromRight(20).reduced(0, 2));
    searchEditor.setBounds(searchBar.reduced(0, 2));
    area.removeFromTop(6);

    // Category pills row (26px)
    auto catRow = area.removeFromTop(26);
    const int pillW = 36;
    const int pillPad = 2;
    int cx = catRow.getX();
    for (auto* btn : categoryButtons)
    {
        btn->setBounds(cx, catRow.getY() + 2, pillW, catRow.getHeight() - 6);
        cx += pillW + pillPad;
    }
    area.removeFromTop(4);

    // Character pills row (26px) + fav filter
    auto charRow = area.removeFromTop(26);
    const int charPillW = 40;
    cx = charRow.getX();
    for (auto* cp : characterPills)
    {
        cp->button.setBounds(cx, charRow.getY() + 2, charPillW, charRow.getHeight() - 6);
        cx += charPillW + pillPad;
    }
    favFilterButton.setBounds(cx + 4, charRow.getY() + 2, 26, charRow.getHeight() - 6);
    area.removeFromTop(4);

    // Tag filter area (24px, shown when filters active)
    tagFilterArea.setBounds(area.removeFromTop(activeTagFilters.size() > 0 ? 24 : 0));
    if (activeTagFilters.size() > 0)
        area.removeFromTop(4);

    // Metadata detail strip at bottom (48px)
    auto metaArea = area.removeFromBottom(48);
    auto metaRow1 = metaArea.removeFromTop(18);
    charactersLabel.setBounds(metaRow1.removeFromRight(metaRow1.getWidth() / 2).reduced(10, 2));
    authorLabel.setBounds(metaRow1.reduced(10, 2));
    auto metaRow2 = metaArea.removeFromTop(16);
    tagsLabel.setBounds(metaRow2.removeFromRight(metaRow2.getWidth() / 2).reduced(10, 0));
    descriptionLabel.setBounds(metaRow2.reduced(10, 0));

    // Preset list viewport fills remaining space
    presetListViewport.setBounds(area);
    presetListContent.setSize(area.getWidth(), presetListContent.getHeight());
    presetListContent.rebuildSections();
}

//==============================================================================
void LibrarianPanel::rebuildFilter()
{
    presetListContent.rebuildSections();
    updateMetadataStrip();
}

//==============================================================================
void LibrarianPanel::setActiveCategory(juce::TextButton* cat)
{
    for (auto* btn : categoryButtons)
        btn->setToggleState(btn == cat, juce::dontSendNotification);

    if (cat == &catAll)        activeCategory = "All";
    else if (cat == &catBass)   activeCategory = "Bass";
    else if (cat == &catLead)   activeCategory = "Lead";
    else if (cat == &catPad)    activeCategory = "Pad";
    else if (cat == &catDrums)  activeCategory = "Drums";
    else if (cat == &catFX)     activeCategory = "FX";
    else if (cat == &catKeys)   activeCategory = "Keys";
    else if (cat == &catArp)    activeCategory = "Arp";

    rebuildFilter();
}

//==============================================================================
void LibrarianPanel::cycleCharacter(CharPill& pill)
{
    if (pill.state == CharFilter::None)
    {
        pill.state = CharFilter::Include;
        pill.button.setColour(juce::TextButton::buttonColourId, accentCyan.withAlpha(0.2f));
        pill.button.setColour(juce::TextButton::textColourOffId, accentCyan);
        pill.button.setColour(juce::TextButton::textColourOnId, accentCyan);
    }
    else if (pill.state == CharFilter::Include)
    {
        pill.state = CharFilter::Exclude;
        pill.button.setColour(juce::TextButton::buttonColourId, accentPink.withAlpha(0.2f));
        pill.button.setColour(juce::TextButton::textColourOffId, accentPink);
        pill.button.setColour(juce::TextButton::textColourOnId, accentPink);
    }
    else
    {
        pill.state = CharFilter::None;
        pill.button.setColour(juce::TextButton::buttonColourId, bgDeep);
        pill.button.setColour(juce::TextButton::textColourOffId, textMuted);
        pill.button.setColour(juce::TextButton::textColourOnId, textMuted);
    }
    rebuildFilter();
}

//==============================================================================
void LibrarianPanel::buttonClicked(juce::Button* b)
{
    if (b == &historyBack)
    {
        auto& pm = processorRef.getPresetManager();
        if (pm.canGoBack())
        {
            int idx = pm.goBack();
            processorRef.cancelPreviewNote();
            processorRef.loadPresetAtIndex(idx);
            refresh();
        }
    }
    else if (b == &historyForward)
    {
        auto& pm = processorRef.getPresetManager();
        if (pm.canGoForward())
        {
            int idx = pm.goForward();
            processorRef.cancelPreviewNote();
            processorRef.loadPresetAtIndex(idx);
            refresh();
        }
    }
    else if (b == &autoPreviewButton)
    {
        autoPreviewEnabled = autoPreviewButton.getToggleState();
        if (!autoPreviewEnabled)
            processorRef.cancelPreviewNote();
    }
    else if (b == &favFilterButton)
    {
        favFilterActive = !favFilterActive;
        favFilterButton.setColour(juce::TextButton::buttonColourId,
            favFilterActive ? accentCyan.withAlpha(0.2f) : bgDeep);
        favFilterButton.setColour(juce::TextButton::textColourOffId,
            favFilterActive ? accentCyan : textMuted);
        rebuildFilter();
    }
    else
    {
        // Check character pills
        for (auto* cp : characterPills)
        {
            if (b == &cp->button)
            {
                cycleCharacter(*cp);
                return;
            }
        }

        // Check category buttons
        for (auto* btn : categoryButtons)
        {
            if (b == btn)
            {
                setActiveCategory(btn);
                return;
            }
        }
    }
}

//==============================================================================
void LibrarianPanel::textEditorTextChanged(juce::TextEditor&)
{
    rebuildFilter();
}

void LibrarianPanel::textEditorReturnKeyPressed(juce::TextEditor&)
{
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
        juce::StringArray remaining;
        for (auto& w : words)
            if (!w.startsWith("#"))
                remaining.add(w);
        searchEditor.setText(remaining.joinIntoString(" "), juce::dontSendNotification);
        rebuildFilter();
    }
}

void LibrarianPanel::textEditorEscapeKeyPressed(juce::TextEditor& ed)
{
    ed.clear();
    rebuildFilter();
}

void LibrarianPanel::textEditorFocusLost(juce::TextEditor&) {}

//==============================================================================
void LibrarianPanel::timerCallback()
{
    if (!autoPreviewEnabled)
        return;

    int idx = presetListContent.getSelectedPresetIndex();
    if (idx != lastPreviewedIndex && idx >= 0)
    {
        lastPreviewedIndex = idx;
        processorRef.triggerPreviewNote();
    }
}

//==============================================================================
void LibrarianPanel::loadPresetByIndex(int presetIndex)
{
    processorRef.cancelPreviewNote();
    processorRef.loadPresetAtIndex(presetIndex);
    refresh();
}

void LibrarianPanel::saveCurrentPreset()
{
    showSaveDialog();
}

void LibrarianPanel::refresh()
{
    rebuildFilter();
}

void LibrarianPanel::focusSearchEditor()
{
    searchEditor.grabKeyboardFocus();
    searchEditor.selectAll();
}

int LibrarianPanel::getSelectedPresetIndex() const
{
    return presetListContent.getSelectedPresetIndex();
}

//==============================================================================
void LibrarianPanel::importPreset()
{
    juce::FileChooser chooser("Import Preset", juce::File(), "*.mvpreset");
    chooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();
            if (!results.isEmpty())
            {
                processorRef.getPresetManager().importPreset(results.getReference(0));
                rebuildFilter();
            }
        });
}

void LibrarianPanel::exportPreset()
{
    int idx = presetListContent.getSelectedPresetIndex();
    if (idx < 0) return;

    juce::FileChooser chooser("Export Preset", juce::File(), "*.mvpreset");
    chooser.launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, idx](const juce::FileChooser& fc)
        {
            auto results = fc.getResults();
            if (!results.isEmpty())
                processorRef.getPresetManager().exportPreset(idx, results.getReference(0));
        });
}

//==============================================================================
void LibrarianPanel::updateMetadataStrip()
{
    int idx = presetListContent.getSelectedPresetIndex();
    if (idx >= 0)
    {
        const auto& meta = processorRef.getPresetManager().getPresetMetadata(idx);
        authorLabel.setText("Author: " + meta.author, juce::dontSendNotification);
        descriptionLabel.setText(meta.description.isNotEmpty() ? meta.description : "No description", juce::dontSendNotification);
        tagsLabel.setText(meta.tags.size() > 0 ? ("#" + meta.tags.joinIntoString(" #")) : "", juce::dontSendNotification);
        charactersLabel.setText(meta.characters.size() > 0 ? meta.characters.joinIntoString(", ") : "", juce::dontSendNotification);
    }
    else
    {
        authorLabel.setText("", juce::dontSendNotification);
        descriptionLabel.setText("", juce::dontSendNotification);
        tagsLabel.setText("", juce::dontSendNotification);
        charactersLabel.setText("", juce::dontSendNotification);
    }
}

//==============================================================================
void LibrarianPanel::addTagFilter(const juce::String& tag)
{
    if (!activeTagFilters.contains(tag))
    {
        activeTagFilters.add(tag);
        rebuildFilter();
    }
}

void LibrarianPanel::removeTagFilter(const juce::String& tag)
{
    activeTagFilters.removeString(tag);
    rebuildFilter();
}

void LibrarianPanel::clearTagFilters()
{
    activeTagFilters.clear();
    rebuildFilter();
}

//==============================================================================
void LibrarianPanel::showSaveDialog()
{
    if (processorRef.getPresetManager().isCurrentBankReadOnly())
        return;

    auto* dialog = new juce::Component();
    dialog->setSize(300, 220);

    auto* nameEd = new juce::TextEditor();
    nameEd->setTextToShowWhenEmpty("Preset name...", textMuted);
    nameEd->setFont(MultiverseFlatTheme::titleFont());
    nameEd->setBounds(70, 10, 220, 24);
    dialog->addAndMakeVisible(nameEd);

    auto* nameLb = new juce::Label("Name", "Name:");
    nameLb->setColour(juce::Label::textColourId, textSecondary);
    nameLb->setBounds(10, 10, 55, 24);
    nameLb->setJustificationType(juce::Justification::centredRight);
    dialog->addAndMakeVisible(nameLb);

    auto* catCb = new juce::ComboBox();
    catCb->addItemList({"Init", "Bass", "Lead", "Pad", "Drums", "FX", "Keys", "Arp"}, 1);
    juce::String curCat = processorRef.currentPresetCategory;
    int catIdx = juce::StringArray({"Init", "Bass", "Lead", "Pad", "Drums", "FX", "Keys", "Arp"}).indexOf(curCat);
    if (catIdx < 0) catIdx = 0;
    catCb->setSelectedItemIndex(catIdx, juce::dontSendNotification);
    catCb->setBounds(70, 40, 220, 24);
    dialog->addAndMakeVisible(catCb);

    auto* catLb = new juce::Label("Cat", "Category:");
    catLb->setColour(juce::Label::textColourId, textSecondary);
    catLb->setBounds(10, 40, 55, 24);
    catLb->setJustificationType(juce::Justification::centredRight);
    dialog->addAndMakeVisible(catLb);

    auto* tagsEd = new juce::TextEditor();
    tagsEd->setText(processorRef.currentPresetTags);
    tagsEd->setTextToShowWhenEmpty("acid, warm, dark...", textMuted);
    tagsEd->setFont(MultiverseFlatTheme::titleFont());
    tagsEd->setBounds(70, 70, 220, 24);
    dialog->addAndMakeVisible(tagsEd);

    auto* tagsLb = new juce::Label("Tags", "Tags:");
    tagsLb->setColour(juce::Label::textColourId, textSecondary);
    tagsLb->setBounds(10, 70, 55, 24);
    tagsLb->setJustificationType(juce::Justification::centredRight);
    dialog->addAndMakeVisible(tagsLb);

    auto* charEd = new juce::TextEditor();
    charEd->setText(processorRef.currentPresetCharacters);
    charEd->setTextToShowWhenEmpty("dark, bright, active...", textMuted);
    charEd->setFont(MultiverseFlatTheme::titleFont());
    charEd->setBounds(70, 100, 220, 24);
    dialog->addAndMakeVisible(charEd);

    auto* charLb = new juce::Label("Char", "Chars:");
    charLb->setColour(juce::Label::textColourId, textSecondary);
    charLb->setBounds(10, 100, 55, 24);
    charLb->setJustificationType(juce::Justification::centredRight);
    dialog->addAndMakeVisible(charLb);

    auto* descEd = new juce::TextEditor();
    descEd->setText(processorRef.currentPresetDescription);
    descEd->setTextToShowWhenEmpty("Description...", textMuted);
    descEd->setFont(MultiverseFlatTheme::titleFont());
    descEd->setBounds(70, 130, 220, 48);
    descEd->setMultiLine(true);
    dialog->addAndMakeVisible(descEd);

    auto* descLb = new juce::Label("Desc", "Desc:");
    descLb->setColour(juce::Label::textColourId, textSecondary);
    descLb->setBounds(10, 130, 55, 24);
    descLb->setJustificationType(juce::Justification::centredRight);
    dialog->addAndMakeVisible(descLb);

    auto* saveBtn = new juce::TextButton("Save");
    saveBtn->setBounds(100, 188, 100, 28);
    saveBtn->setColour(juce::TextButton::buttonColourId, accentCyan);
    saveBtn->setColour(juce::TextButton::textColourOffId, textPrimary);
    dialog->addAndMakeVisible(saveBtn);

    saveBtn->onClick = [this, dialog, nameEd, catCb, tagsEd, charEd, descEd]() mutable
    {
        auto name = nameEd->getText().trim();
        if (name.isEmpty())
            return;

        processorRef.currentPresetCategory = catCb->getText();
        processorRef.currentPresetTags = tagsEd->getText();
        processorRef.currentPresetCharacters = charEd->getText();
        processorRef.currentPresetDescription = descEd->getText();

        processorRef.saveNamedPreset(name);
        delete dialog;
        rebuildFilter();
    };

    auto& box = juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(dialog),
        getScreenBounds(), nullptr);
    (void)box;
}

//==============================================================================
void LibrarianPanel::showPresetContextMenu(int presetIndex, const juce::Point<int>& screenPos, int bookmarkFolderIdx)
{
    auto& pm = processorRef.getPresetManager();

    juce::PopupMenu menu;
    menu.addItem("Load", [this, presetIndex]()
    {
        processorRef.cancelPreviewNote();
        processorRef.loadPresetAtIndex(presetIndex);
        refresh();
    });

    menu.addItem("Save", [this]() { saveCurrentPreset(); });

    menu.addItem("Delete", [this, presetIndex]()
    {
        processorRef.getPresetManager().deletePreset(presetIndex);
        rebuildFilter();
    });

    menu.addItem("Import", [this]() { importPreset(); });

    menu.addItem("Export", [this, presetIndex]()
    {
        juce::FileChooser chooser("Export Preset", juce::File(), "*.mvpreset");
        chooser.launchAsync(juce::FileBrowserComponent::saveMode,
            [this, presetIndex](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (!results.isEmpty())
                    processorRef.getPresetManager().exportPreset(presetIndex, results.getReference(0));
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
        favMenu.addItem(juce::String(colorNames[i]) + (isCurrent ? " *" : ""),
            [this, presetIndex, i]()
        {
            processorRef.getPresetManager().setFavorite(presetIndex, i);
            rebuildFilter();
        });
    }

    if (currentColor >= 0)
    {
        favMenu.addSeparator();
        favMenu.addItem("Clear Favorite", [this, presetIndex]()
        {
            processorRef.getPresetManager().setFavorite(presetIndex, -1);
            rebuildFilter();
        });
    }

    menu.addSubMenu("Favorite", favMenu);

    // Add to Bookmark submenu
    auto& folders = pm.getBookmarkFolders();
    if (folders.size() > 0)
    {
        juce::PopupMenu bmMenu;
        auto relPath = pm.getRelativePresetPath(presetIndex);
        for (size_t f = 0; f < folders.size(); ++f)
        {
            bool alreadyIn = folders[f].presetPaths.contains(relPath);
            bmMenu.addItem(folders[f].name + (alreadyIn ? " *" : ""),
                [this, f, relPath]()
            {
                processorRef.getPresetManager().addPresetToBookmark(static_cast<int>(f), relPath);
                rebuildFilter();
            });
        }
        menu.addSubMenu("Add to Bookmark", bmMenu);
    }

    // Remove from Bookmark folder (if inside a bookmark)
    if (bookmarkFolderIdx >= 0 && bookmarkFolderIdx < static_cast<int>(folders.size()))
    {
        auto relPath = pm.getRelativePresetPath(presetIndex);
        if (folders[bookmarkFolderIdx].presetPaths.contains(relPath))
        {
            menu.addItem("Remove from '" + folders[bookmarkFolderIdx].name + "'", [this, bookmarkFolderIdx, relPath]()
            {
                processorRef.getPresetManager().removePresetFromBookmark(bookmarkFolderIdx, relPath);
                rebuildFilter();
            });
        }
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(
        juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1)));
}

void LibrarianPanel::showBookmarkFolderMenu(int folderIndex, const juce::Point<int>& screenPos)
{
    juce::PopupMenu menu;

    menu.addItem("Rename Folder", [this, folderIndex]()
    {
        promptNewBookmarkFolder();
    });

    menu.addItem("Delete Folder", [this, folderIndex]()
    {
        processorRef.getPresetManager().deleteBookmarkFolder(folderIndex);
        rebuildFilter();
    });

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(
        juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1)));
}

void LibrarianPanel::promptNewBookmarkFolder()
{
    auto* dialog = new juce::Component();
    dialog->setSize(250, 80);

    auto* nameEd = new juce::TextEditor();
    nameEd->setTextToShowWhenEmpty("Folder name...", textMuted);
    nameEd->setFont(MultiverseFlatTheme::titleFont());
    nameEd->setBounds(10, 10, 230, 24);
    dialog->addAndMakeVisible(nameEd);

    auto* createBtn = new juce::TextButton("Create");
    createBtn->setBounds(75, 44, 100, 28);
    createBtn->setColour(juce::TextButton::buttonColourId, accentCyan);
    createBtn->setColour(juce::TextButton::textColourOffId, textPrimary);
    dialog->addAndMakeVisible(createBtn);

    createBtn->onClick = [this, dialog, nameEd]() mutable
    {
        auto name = nameEd->getText().trim();
        if (name.isNotEmpty())
        {
            processorRef.getPresetManager().createBookmarkFolder(name);
            rebuildFilter();
        }
        delete dialog;
    };

    auto& box = juce::CallOutBox::launchAsynchronously(std::unique_ptr<juce::Component>(dialog),
        getScreenBounds(), nullptr);
    (void)box;
}