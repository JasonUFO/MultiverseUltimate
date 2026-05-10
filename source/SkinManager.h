#pragma once
#include <JuceHeader.h>

// ─── Skin definition ────────────────────────────────────────────────────────
// A Skin holds every colour the UI needs. 20 predefined skins are provided;
// the user picks one from the Global panel dropdown at runtime.

struct Skin
{
    juce::String name;          // Display name for the selector

    // Backgrounds
    juce::Colour bgVoid;       // Deepest background (window chrome)
    juce::Colour bgBase;       // Main panel background
    juce::Colour bgRaised;    // Cards, raised surfaces
    juce::Colour bgDeep;      // Tracks, deep insets
    juce::Colour bgHover;     // Hover states

    // Accents
    juce::Colour accent1;     // Primary accent (replaces accentCyan)
    juce::Colour accent2;     // Secondary accent (replaces accentPink)
    juce::Colour accent3;     // Tertiary accent (replaces accentPurple)
    juce::Colour accent4;     // Fourth accent (replaces accentGreen)
    juce::Colour accent5;     // Fifth accent (replaces accentAmber)

    // Borders & text
    juce::Colour borderLight;
    juce::Colour borderActive;
    juce::Colour textPrimary;
    juce::Colour textSecondary;
    juce::Colour textMuted;
    juce::Colour textLabel;

    // Extra colours for richer UI
    juce::Colour glowStrong;  // Glow on active elements
    juce::Colour glowSoft;    // Soft glow for hover / backgrounds
    juce::Colour keyboardWhite;
    juce::Colour keyboardBlack;
    juce::Colour keyboardHighlight;
    juce::Colour wheelTrack;
    juce::Colour wheelFill;

    // Tab colours
    juce::Colour tabPrimaryBg;
    juce::Colour tabSecondaryBg;
    juce::Colour tabActiveBg;
    juce::Colour tabActiveGlow;

    // Keyboard / wheel sizes (not colours, but part of the skin)
    float keyboardHeight;      // bottom strip height for keyboard
    float wheelWidth;          // pitch/mod wheel width

    // 3D depth properties (computed from base colours if not explicitly set)
    juce::Colour shadowDark;      // Dark shadow for depth (bottom-right bevel)
    juce::Colour shadowLight;     // Light highlight for bevel (top-left bevel)
    juce::Colour highlightSpec;   // Specular highlight colour
    juce::Colour panelGradient1;  // Panel gradient start (top)
    juce::Colour panelGradient2;  // Panel gradient end (bottom)
    juce::Colour insetBg;         // Inset/recessed background
    float bevelStrength;           // 0.0-1.0, how pronounced 3D bevels are
    float glowIntensity;           // 0.0-2.0, glow brightness multiplier

    // Convenience aliases (back-compat)
    juce::Colour accentCyan;   // alias accent1
    juce::Colour accentPink;   // alias accent2
    juce::Colour accentPurple; // alias accent3
    juce::Colour accentGreen;  // alias accent4
    juce::Colour accentAmber;  // alias accent5
};

// ─── SkinManager ────────────────────────────────────────────────────────────
// Singleton-style manager. All UI code calls SkinManager::current() to get
// colours.  When the skin changes, SkinManager triggers a global repaint.

class SkinManager
{
public:
    // Returns the singleton instance
    static SkinManager& instance();

    // Current skin access
    const Skin& current() const { return skins[currentIndex_]; }
    int getSkinIndex() const { return currentIndex_; }

    // Switch skin by index; triggers repaint on all top-level components
    void setSkin(int index);

    // Switch by name; returns true if found
    bool setSkinByName(const juce::String& name);

    // Number of available skins
    int numSkins() const { return static_cast<int>(skins.size()); }

    // Get skin name by index
    juce::String skinName(int index) const;

    // Register a component to repaint when skin changes
    void addListener(juce::Component* c);
    void removeListener(juce::Component* c);

    // Register a callback to fire when skin changes (for syncing UI, updating colours, etc.)
    void addSkinChangeCallback(std::function<void()> callback);
    void removeSkinChangeCallback(std::function<void()>* callback);

    // Persist / restore the skin choice
    void saveToState(juce::XmlElement& xml) const;
    void loadFromState(const juce::XmlElement& xml);

private:
    SkinManager();
    ~SkinManager() = default;

    std::vector<Skin> skins;
    int currentIndex_ = 0;
    std::vector<juce::Component*> listeners;
    std::vector<std::pair<std::function<void()>* , std::function<void()>>> skinChangeCallbacks;

    // Build all 20 skin presets
    void buildPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SkinManager)
};