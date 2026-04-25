#pragma once
#include "SamplerEngine.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <memory>

class SamplerPanel : public juce::Component,
                     public juce::FileDragAndDropTarget
{
public:
    explicit SamplerPanel (SamplerEngine& engine);
    ~SamplerPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // FileDragAndDropTarget
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;

    // Accessible to ZoneListModel (inner struct)
    std::vector<std::shared_ptr<SamplerZone>> ownedZones;
    int selectedZoneIndex = -1;
    void updateControlsForSelectedZone();

private:
    struct ZoneListModel : public juce::ListBoxModel
    {
        SamplerPanel& panel;
        explicit ZoneListModel (SamplerPanel& p) : panel (p) {}
        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    };

    void loadFile (const juce::File& file);
    void updateZoneList();

    SamplerEngine& samplerEngine;
    juce::AudioFormatManager formatManager;

    // ZoneListModel must be declared before zoneList so zoneList is destroyed first
    ZoneListModel zoneListModel;
    juce::ListBox zoneList;

    juce::TextButton clearButton { "Clear All" };

    juce::Label loopModeLabel  { {}, "Loop:" };
    juce::ComboBox loopModeCombo;

    juce::Label rootNoteLabel  { {}, "Root:" };
    juce::ComboBox rootNoteCombo;

    juce::Label loopStartLabel { {}, "Start:" };
    juce::Slider loopStartSlider;

    juce::Label loopEndLabel   { {}, "End:" };
    juce::Slider loopEndSlider;

    juce::Label xfadeLabel     { {}, "Xfade:" };
    juce::Slider xfadeSlider;

    bool isDragOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerPanel)
};
