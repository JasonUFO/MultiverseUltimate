#include "SkinManager.h"
#include "Assets/AssetManager.h"

// Helper: make a Colour from hex
static juce::Colour C(uint32_t argb) { return juce::Colour(argb); }

//==============================================================================
SkinManager& SkinManager::instance()
{
    static SkinManager mgr;
    return mgr;
}

//==============================================================================
// Skin presets — 20 sci-themed skins
//
// Naming convention: sci-fi / space / cyber / tech themes

static Skin makeSkin(
    const char* name,
    uint32_t bgVoid, uint32_t bgBase, uint32_t bgRaised, uint32_t bgDeep, uint32_t bgHover,
    uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5,
    uint32_t borderLight_, uint32_t borderActive_,
    uint32_t textPri, uint32_t textSec, uint32_t textMut, uint32_t textLbl,
    uint32_t glowS, uint32_t glowF,
    uint32_t kbW, uint32_t kbB, uint32_t kbH,
    uint32_t whTr, uint32_t whFl,
    uint32_t tabPri, uint32_t tabSec, uint32_t tabAct, uint32_t tabGl,
    float kbHeight = 110.f, float whWidth = 36.f)
{
    Skin s;
    s.name = name;
    s.bgVoid = C(bgVoid); s.bgBase = C(bgBase); s.bgRaised = C(bgRaised);
    s.bgDeep = C(bgDeep); s.bgHover = C(bgHover);
    s.accent1 = C(a1); s.accent2 = C(a2); s.accent3 = C(a3);
    s.accent4 = C(a4); s.accent5 = C(a5);
    s.accentCyan = C(a1); s.accentPink = C(a2); s.accentPurple = C(a3);
    s.accentGreen = C(a4); s.accentAmber = C(a5);
    s.borderLight = C(borderLight_); s.borderActive = C(borderActive_);
    s.textPrimary = C(textPri); s.textSecondary = C(textSec);
    s.textMuted = C(textMut); s.textLabel = C(textLbl);
    s.glowStrong = C(glowS); s.glowSoft = C(glowF);
    s.keyboardWhite = C(kbW); s.keyboardBlack = C(kbB); s.keyboardHighlight = C(kbH);
    s.wheelTrack = C(whTr); s.wheelFill = C(whFl);
    s.tabPrimaryBg = C(tabPri); s.tabSecondaryBg = C(tabSec);
    s.tabActiveBg = C(tabAct); s.tabActiveGlow = C(tabGl);
    s.keyboardHeight = kbHeight; s.wheelWidth = whWidth;
    return s;
}

void SkinManager::buildPresets()
{
    skins = {
        // 0 — Neon Abyss (default, original palette enhanced)
        makeSkin("Neon Abyss",
            0xFF0F1014, 0xFF15171C, 0xFF262930, 0xFF111318, 0xFF323540,
            0xFF00D4FF, 0xFFFF2A6D, 0xFF9B6DFF, 0xFF00FF87, 0xFFFFB800,
            0xFF404558, 0xFF00D4FF,
            0xFFF0F0F8, 0xFF9098A8, 0xFF556070, 0xFF7080A0,
            0xFF00D4FF, 0x4D00D4FF,
            0xFFD8D8E0, 0xFF1A1C22, 0xFF00D4FF,
            0xFF111318, 0xFF00D4FF,
            0xFF0A0B0E, 0xFF111318, 0xFF15171C, 0xFF00D4FF),

        // 1 — Stellar Command
        makeSkin("Stellar Command",
            0xFF0A0E1A, 0xFF101828, 0xFF1E2A42, 0xFF0C1220, 0xFF2A3858,
            0xFF4DA6FF, 0xFFFF6B9D, 0xFF8B5CF6, 0xFF00E5A0, 0xFFFFD93D,
            0xFF3A4A68, 0xFF4DA6FF,
            0xFFE8ECF4, 0xFF8899B4, 0xFF4E6080, 0xFF6E84A8,
            0xFF4DA6FF, 0x4D4DA6FF,
            0xFFC8D0E0, 0xFF18243A, 0xFF4DA6FF,
            0xFF0C1220, 0xFF4DA6FF,
            0xFF080C16, 0xFF0E1626, 0xFF182844, 0xFF4DA6FF),

        // 2 — Quantum Flux
        makeSkin("Quantum Flux",
            0xFF0D0020, 0xFF140030, 0xFF240050, 0xFF0E0028, 0xFF360068,
            0xFFCC00FF, 0xFFFF1493, 0xFF00FFCC, 0xFFFFD700, 0xFF00BFFF,
            0xFF55007F, 0xFFCC00FF,
            0xFFF0E0FF, 0xFFA080CC, 0xFF664488, 0xFF8866AA,
            0xFFCC00FF, 0x4DCC00FF,
            0xFFD0C0E0, 0xFF200040, 0xFFCC00FF,
            0xFF100024, 0xFFCC00FF,
            0xFF080018, 0xFF120028, 0xFF1C0040, 0xFFCC00FF),

        // 3 — Solar Flare
        makeSkin("Solar Flare",
            0xFF1A0A00, 0xFF281400, 0xFF3E2010, 0xFF1E0C00, 0xFF503018,
            0xFFFF6600, 0xFFFFAA00, 0xFFFF3333, 0xFFFFEE00, 0xFFFF4444,
            0xFF5E3820, 0xFFFF6600,
            0xFFFFF0E0, 0xFFCCAA80, 0xFF886644, 0xFFAA8866,
            0xFFFF6600, 0x4DFF6600,
            0xFFEED8C0, 0xFF2A1808, 0xFFFF6600,
            0xFF200E00, 0xFFFF6600,
            0xFF140800, 0xFF1E0E00, 0xFF302010, 0xFFFF6600),

        // 4 — Void Walker
        makeSkin("Void Walker",
            0xFF000000, 0xFF0A0A10, 0xFF161622, 0xFF060610, 0xFF20202E,
            0xFF8B00FF, 0xFFFF00AA, 0xFF00FFFF, 0xFF44FF00, 0xFFFFFF00,
            0xFF303044, 0xFF8B00FF,
            0xFFE0E0F8, 0xFF7878A0, 0xFF3E3E60, 0xFF5A5A80,
            0xFF8B00FF, 0x4D8B00FF,
            0xFFC0C0D8, 0xFF10101A, 0xFF8B00FF,
            0xFF080810, 0xFF8B00FF,
            0xFF000008, 0xFF0A0A12, 0xFF14142A, 0xFF8B00FF),

        // 5 — Hologram
        makeSkin("Hologram",
            0xFF001020, 0xFF001830, 0xFF002848, 0xFF001428, 0xFF003858,
            0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00, 0xFF00FF80, 0xFFFF8040,
            0xFF005070, 0xFF00FFFF,
            0xFFE0FFFF, 0xFF80CCCC, 0xFF408888, 0xFF60AACC,
            0xFF00FFFF, 0x4D00FFFF,
            0xFFCCFFFF, 0xFF002840, 0xFF00FFFF,
            0xFF001828, 0xFF00FFFF,
            0xFF000C18, 0xFF001420, 0xFF002838, 0xFF00FFFF),

        // 6 — Dark Matter
        makeSkin("Dark Matter",
            0xFF08080C, 0xFF101018, 0xFF1C1C28, 0xFF0A0A12, 0xFF282838,
            0xFF00FF88, 0xFFFF3366, 0xFF8844FF, 0xFF00CCFF, 0xFFFFBB33,
            0xFF383848, 0xFF00FF88,
            0xFFE8E8F0, 0xFF888898, 0xFF484858, 0xFF686878,
            0xFF00FF88, 0x4D00FF88,
            0xFFD0D0D8, 0xFF141418, 0xFF00FF88,
            0xFF0C0C14, 0xFF00FF88,
            0xFF06060A, 0xFF0C0C16, 0xFF161624, 0xFF00FF88),

        // 7 — Nebula Core
        makeSkin("Nebula Core",
            0xFF140018, 0xFF200024, 0xFF34003A, 0xFF18001C, 0xFF480050,
            0xFFFF00FF, 0xFF00CCFF, 0xFFFFAA00, 0xFF44FF88, 0xFFFF5555,
            0xFF5A0068, 0xFFFF00FF,
            0xFFF8E0FF, 0xFFAA78CC, 0xFF664488, 0xFF8866AA,
            0xFFFF00FF, 0x4DFF00FF,
            0xFFD8C0E0, 0xFF280030, 0xFFFF00FF,
            0xFF180020, 0xFFFF00FF,
            0xFF100014, 0xFF1C0020, 0xFF300040, 0xFFFF00FF),

        // 8 — Circuit Board
        makeSkin("Circuit Board",
            0xFF081008, 0xFF0E1C0E, 0xFF1C2E1C, 0xFF0A160A, 0xFF283828,
            0xFF00FF44, 0xFFFFCC00, 0xFF00CCCC, 0xFFFF6633, 0xFF44AAFF,
            0xFF384838, 0xFF00FF44,
            0xFFD8FFD8, 0xFF88CC88, 0xFF448844, 0xFF66AA66,
            0xFF00FF44, 0x4D00FF44,
            0xFFC0E0C0, 0xFF142014, 0xFF00FF44,
            0xFF0C180C, 0xFF00FF44,
            0xFF060E06, 0xFF0C180C, 0xFF1A2E1A, 0xFF00FF44),

        // 9 — Plasma Drive
        makeSkin("Plasma Drive",
            0xFF140008, 0xFF220010, 0xFF380020, 0xFF1A000C, 0xFF4A0028,
            0xFFFF0066, 0xFFFF8800, 0xFFCC00FF, 0xFF00FFAA, 0xFFFFFF44,
            0xFF5A2030, 0xFFFF0066,
            0xFFFFE0F0, 0xFFCC88AA, 0xFF744460, 0xFFAA6688,
            0xFFFF0066, 0x4DFF0066,
            0xFFE0C0CC, 0xFF260014, 0xFFFF0066,
            0xFF1E0010, 0xFFFF0066,
            0xFF100008, 0xFF1A000E, 0xFF300024, 0xFFFF0066),

        // 10 — Titan Forge
        makeSkin("Titan Forge",
            0xFF101008, 0xFF1C1C10, 0xFF2C2C1C, 0xFF141410, 0xFF383828,
            0xFFFFCC00, 0xFFFF8800, 0xFFAAAAFF, 0xFF44DDAA, 0xFFFF5544,
            0xFF444434, 0xFFFFCC00,
            0xFFF0F0E0, 0xFFAAAA88, 0xFF666648, 0xFF888868,
            0xFFFFCC00, 0x4DFFCC00,
            0xFFD8D8C0, 0xFF201C10, 0xFFFFCC00,
            0xFF181408, 0xFFFFCC00,
            0xFF0C0C06, 0xFF161610, 0xFF28281C, 0xFFFFCC00),

        // 11 — Ion Storm
        makeSkin("Ion Storm",
            0xFF0A0818, 0xFF121028, 0xFF221C42, 0xFF0E0C20, 0xFF302858,
            0xFF8888FF, 0xFFFF44CC, 0xFF00FFDD, 0xFFFF8844, 0xFF44FF44,
            0xFF443868, 0xFF8888FF,
            0xFFE8E0FF, 0xFF9088BB, 0xFF504888, 0xFF7068A0,
            0xFF8888FF, 0x4D8888FF,
            0xFFC8C0E0, 0xFF1A1840, 0xFF8888FF,
            0xFF101028, 0xFF8888FF,
            0xFF080614, 0xFF0E0C22, 0xFF1C1838, 0xFF8888FF),

        // 12 — Cryo Chamber
        makeSkin("Cryo Chamber",
            0xFF000C18, 0xFF001420, 0xFF002438, 0xFF000A14, 0xFF003050,
            0xFF00CCFF, 0xFF6688FF, 0xFFAADDFF, 0xFF44FFAA, 0xFFEEEEFF,
            0xFF004468, 0xFF00CCFF,
            0xFFE0F8FF, 0xFF80AAC8, 0xFF406888, 0xFF6090B0,
            0xFF00CCFF, 0x4D00CCFF,
            0xFFC0E0F0, 0xFF001830, 0xFF00CCFF,
            0xFF001020, 0xFF00CCFF,
            0xFF000610, 0xFF000E1A, 0xFF001C30, 0xFF00CCFF),

        // 13 — Binary Sunset
        makeSkin("Binary Sunset",
            0xFF180C00, 0xFF281A08, 0xFF3E2810, 0xFF1E1200, 0xFF503418,
            0xFFFF8833, 0xFFFFCC44, 0xFFFF4466, 0xFF66DDAA, 0xFFFFAA00,
            0xFF5E3C20, 0xFFFF8833,
            0xFFFFF0E0, 0xFFCCAA88, 0xFF886648, 0xFFAA8868,
            0xFFFF8833, 0x4DFF8833,
            0xFFE0D0C0, 0xFF2A1808, 0xFFFF8833,
            0xFF200E00, 0xFFFF8833,
            0xFF140800, 0xFF1E1200, 0xFF302018, 0xFFFF8833),

        // 14 — Xenomorph
        makeSkin("Xenomorph",
            0xFF040C08, 0xFF081810, 0xFF142820, 0xFF06100C, 0xFF1C3830,
            0xFF00FFAA, 0xFFFF3355, 0xFF66FFCC, 0xFFCC8800, 0xFF00DDFF,
            0xFF284438, 0xFF00FFAA,
            0xFFC0FFE0, 0xFF68CC88, 0xFF2C7050, 0xFF4C9870,
            0xFF00FFAA, 0x4D00FFAA,
            0xFFB0DCC0, 0xFF0E1C14, 0xFF00FFAA,
            0xFF0A160E, 0xFF00FFAA,
            0xFF040A06, 0xFF081410, 0xFF142420, 0xFF00FFAA),

        // 15 — Supernova Remnant
        makeSkin("Supernova Remnant",
            0xFF100008, 0xFF1C0010, 0xFF300020, 0xFF14000A, 0xFF420030,
            0xFFFF4488, 0xFFFFAA33, 0xFFBB44FF, 0xFF44FFCC, 0xFFFFFF44,
            0xFF502840, 0xFFFF4488,
            0xFFFFE0F0, 0xFFCC88AA, 0xFF704468, 0xFFAA6690,
            0xFFFF4488, 0x4DFF4488,
            0xFFE0C0D0, 0xFF240018, 0xFFFF4488,
            0xFF1C0010, 0xFFFF4488,
            0xFF0C0006, 0xFF16000C, 0xFF280020, 0xFFFF4488),

        // 16 — Warp Gate
        makeSkin("Warp Gate",
            0xFF080418, 0xFF10082A, 0xFF1C1444, 0xFF0C0620, 0xFF282058,
            0xFF8844FF, 0xFFFF00CC, 0xFF00FFEE, 0xFFFFAA00, 0xFF44DDFF,
            0xFF3C2868, 0xFF8844FF,
            0xFFE0D0FF, 0xFF8878AA, 0xFF443888, 0xFF6858A0,
            0xFF8844FF, 0x4D8844FF,
            0xFFC0B0E0, 0xFF180E30, 0xFF8844FF,
            0xFF100828, 0xFF8844FF,
            0xFF060410, 0xFF0C0822, 0xFF181040, 0xFF8844FF),

        // 17 — Fusion Core
        makeSkin("Fusion Core",
            0xFF100800, 0xFF1C1200, 0xFF2E2000, 0xFF140C00, 0xFF403000,
            0xFFFFDD00, 0xFFFF6600, 0xFFFF2200, 0xFF00FF88, 0xFF44AAFF,
            0xFF4E3820, 0xFFFFDD00,
            0xFFFFF8E0, 0xFFCCBB88, 0xFF887748, 0xFFAA9968,
            0xFFFFDD00, 0x4DFFDD00,
            0xFFE0D8B0, 0xFF221400, 0xFFFFDD00,
            0xFF1A0E00, 0xFFFFDD00,
            0xFF0C0600, 0xFF140E00, 0xFF281C00, 0xFFFFDD00),

        // 18 — Deep Signal
        makeSkin("Deep Signal",
            0xFF000814, 0xFF001020, 0xFF001E38, 0xFF000C1A, 0xFF002A4A,
            0xFF0088FF, 0xFF00CCCC, 0xFF4466FF, 0xFF44FFAA, 0xFFFFCC44,
            0xFF003058, 0xFF0088FF,
            0xFFD0E8FF, 0xFF6898CC, 0xFF285888, 0xFF4878AA,
            0xFF0088FF, 0x4D0088FF,
            0xFFB0D0F0, 0xFF001830, 0xFF0088FF,
            0xFF001220, 0xFF0088FF,
            0xFF000610, 0xFF000E1C, 0xFF001838, 0xFF0088FF),

        // 19 — Chronosphere
        makeSkin("Chronosphere",
            0xFF0C0C14, 0xFF14141E, 0xFF22222E, 0xFF10101A, 0xFF2E2E40,
            0xFFCCAAFF, 0xFFFF66CC, 0xFF66FFEE, 0xFFFFBB44, 0xFF66AAFF,
            0xFF3C3C50, 0xFFCCAAFF,
            0xFFF0E8FF, 0xFF9490AA, 0xFF504C68, 0xFF706888,
            0xFFCCAAFF, 0x4DCCAAFF,
            0xFFD0C8E8, 0xFF1A1A26, 0xFFCCAAFF,
            0xFF12121C, 0xFFCCAAFF,
            0xFF08080E, 0xFF0E0E18, 0xFF1E1E2C, 0xFFCCAAFF)
    };
}

SkinManager::SkinManager()
{
    buildPresets();
}

juce::String SkinManager::skinName(int index) const
{
    if (index >= 0 && index < static_cast<int>(skins.size()))
        return skins[index].name;
    return "Unknown";
}

void SkinManager::setSkin(int index)
{
    if (index < 0 || index >= static_cast<int>(skins.size()))
        return;
    currentIndex_ = index;
    AssetManager::instance().onSkinChanged();
    for (auto* c : listeners)
        if (c != nullptr)
            c->repaint();
}

bool SkinManager::setSkinByName(const juce::String& name)
{
    for (int i = 0; i < static_cast<int>(skins.size()); ++i)
    {
        if (skins[i].name == name)
        {
            setSkin(i);
            return true;
        }
    }
    return false;
}

void SkinManager::addListener(juce::Component* c)
{
    if (c != nullptr)
        listeners.push_back(c);
}

void SkinManager::removeListener(juce::Component* c)
{
    listeners.erase(std::remove(listeners.begin(), listeners.end(), c), listeners.end());
}

void SkinManager::saveToState(juce::XmlElement& xml) const
{
    xml.setAttribute("skinIndex", currentIndex_);
    xml.setAttribute("skinName", skins[currentIndex_].name);
}

void SkinManager::loadFromState(const juce::XmlElement& xml)
{
    int idx = xml.getIntAttribute("skinIndex", 0);
    juce::String name = xml.getStringAttribute("skinName", "");
    // Prefer name match for robustness
    if (name.isNotEmpty())
    {
        for (int i = 0; i < static_cast<int>(skins.size()); ++i)
        {
            if (skins[i].name == name)
            {
                setSkin(i);
                return;
            }
        }
    }
    setSkin(idx);
}