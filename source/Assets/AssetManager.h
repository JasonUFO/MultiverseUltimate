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
    juce::Image getKnobLarge()        const { return tintedImages.count("knob_large_tinted") ? tintedImages.at("knob_large_tinted") : images.at("knob_large"); }
    juce::Image getKnobSmall()        const { return tintedImages.count("knob_small_tinted") ? tintedImages.at("knob_small_tinted") : images.at("knob_small"); }

    // ── Specialty ────────────────────────────────────────────────
    juce::Image getEQGraph()          const { return tintedImages.count("eq_graph_tinted") ? tintedImages.at("eq_graph_tinted") : images.at("eq_graph"); }
    juce::Image getEQGraphUntinted()  const { return images.at("eq_graph"); }
    juce::Image getLpHp()             const { return tintedImages.count("lphp_tinted") ? tintedImages.at("lphp_tinted") : images.at("lphp"); }
    juce::Image getLpHpUntinted()     const { return images.at("lphp"); }
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

    // Creates an overlay-blended copy of source (lerp with alpha)
    // resultPixel = lerp(srcPixel, tintColor, blendAlpha)
    // Preserves brightness while shifting hue — use for skin-aware image tinting
    static juce::Image tintImageOverlay (const juce::Image& source, juce::Colour tintColor, float blendAlpha);

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