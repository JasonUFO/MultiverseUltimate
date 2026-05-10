#pragma once

#include <JuceHeader.h>

class AssetManager
{
public:
    static AssetManager& instance();

    // Call once at plugin startup
    void initialise();

    // Call on skin change to regenerate tinted variants
    void onSkinChanged();

    // ── Knob images ──────────────────────────────────────────────
    juce::Image getKnobLarge()        const { return images.at("knob_large"); }
    juce::Image getKnobSmall()        const { return images.at("knob_small"); }

    // ── Button images ────────────────────────────────────────────
    juce::Image getButtonBigOn()      const { return images.at("button_big_on"); }
    juce::Image getButtonBigOff()     const { return images.at("button_big_off"); }
    juce::Image getButtonNormal()     const { return images.at("button_normal"); }
    juce::Image getButtonHover()      const { return images.at("button_hover"); }
    juce::Image getButtonPressed()    const { return images.at("button_pressed"); }
    juce::Image getButtonDisabled()   const { return images.at("button_disabled"); }

    // ── Toggle / radio ───────────────────────────────────────────
    juce::Image getRadioButtonOff()   const { return images.at("radio_off"); }
    juce::Image getRadioButtonOn()    const { return images.at("radio_on"); }

    // ── Dropdown ─────────────────────────────────────────────────
    juce::Image getDropdown()         const { return images.at("dropdown"); }
    juce::Image getDropdownArrow()    const { return images.at("dropdown_arrow"); }

    // ── Header ───────────────────────────────────────────────────
    juce::Image getHeaderBig()        const { return images.at("header_big"); }
    juce::Image getHeaderSmall()      const { return images.at("header_small"); }

    // ── Panel backgrounds ────────────────────────────────────────
    juce::Image getContent()          const { return images.at("content"); }
    juce::Image getBox()              const { return images.at("box"); }
    juce::Image getBoxActive()         const { return tintedImages.count("box_active") ? tintedImages.at("box_active") : images.at("box"); }
    juce::Image getFrame34()          const { return images.at("frame_34"); }
    juce::Image getFrame38()          const { return images.at("frame_38"); }
    juce::Image getFrame482()         const { return images.at("frame_482"); }
    juce::Image getFrame488()         const { return images.at("frame_488"); }

    // ── Dividers & tabs ─────────────────────────────────────────
    juce::Image getFrame1()           const { return images.at("frame_1"); }
    juce::Image getFrame11()          const { return images.at("frame_11"); }
    juce::Image getFrame37()          const { return images.at("frame_37"); }

    // ── Specialty ────────────────────────────────────────────────
    juce::Image getEQGraph()          const { return images.at("eq_graph"); }
    juce::Image getLpHp()             const { return images.at("lphp"); }
    juce::Image getWheelSprite()      const { return images.at("wheel_sprite"); }

    // ── Wheel sprite dimensions ──────────────────────────────────
    static constexpr int WHEEL_FRAME_W = 167;
    static constexpr int WHEEL_FRAME_H = 167;
    static constexpr int WHEEL_NUM_FRAMES = 336;

    // ── Knob visual center offset (px below geometric center) ─────
    static constexpr float KNOB_LARGE_CENTER_Y_OFFSET = 24.0f;  // 484/2 = 242, visual center ~218
    static constexpr float KNOB_SMALL_CENTER_Y_OFFSET = 12.0f;

    // ── Tinting ──────────────────────────────────────────────────
    // Creates a colour-multiplied copy of source
    static juce::Image tintImage (const juce::Image& source, juce::Colour tint);

    // ── Drawing helpers ──────────────────────────────────────────
    // Draw image scaled to fit dest bounds, maintaining aspect ratio
    static void drawImageFitted (juce::Graphics& g, const juce::Image& source,
                                 juce::Rectangle<float> dest,
                                 juce::RectanglePlacement placement = juce::RectanglePlacement::centred);

    // 9-slice: draw image stretched with borders preserved
    static void drawImage9Slice (juce::Graphics& g, const juce::Image& source,
                                  juce::Rectangle<float> dest,
                                  int leftBorder, int rightBorder,
                                  int topBorder, int bottomBorder);

    // Draw a wheel sprite frame
    void drawWheelFrame (juce::Graphics& g, juce::Rectangle<float> dest,
                         float value, bool isPitchWheel) const;

private:
    AssetManager() = default;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AssetManager)

    std::unordered_map<std::string, juce::Image> images;
    std::unordered_map<std::string, juce::Image> tintedImages;

    void loadAllImages();
    void generateTintedVariants();
};