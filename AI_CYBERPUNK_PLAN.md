# Multiverse Ultimate — Neumorphic Cyberpunk UI Plan

**Status:** Figma SVGs created, ready for sign-off  
**Scope:** Full UI replacement (CyberpunkTheme → CyberpunkTheme)  
**Approach:** Figma-first — SVGs in `Figmacomponents/` ready for review

---

## 1. Color Palette

| Purpose | Color Name | Hex | Notes |
|---|---|---|---|
| **Background (deepest)** | `bgVoid` | `#0A0A12` | Plugin window fill |
| **Background (panel)** | `bgBase` | `#0D0D1A` | Panel/component background |
| **Neumorphic base** | `bgRaised` | `#1A1A2E` | Cards, knobs (neumorphic surface) |
| **Neumorphic deep** | `bgDeep` | `#12121F` | Inset areas, text boxes |
| **Shadow dark** | `shadowDark` | `#050510` | Neumorphic dark shadow |
| **Shadow light** | `shadowLight` | `#25253D` | Neumorphic light highlight |
| **NEON CYAN** | `neonCyan` | `#00F0FF` | Primary accent, arcs, glow |
| **NEON PINK** | `neonPink` | `#FF2A6D` | Secondary accent, macro |
| **NEON PURPLE** | `neonPurple` | `#B026FF` | Tertiary accent, FM, special |
| **NEON GREEN** | `neonGreen` | `#05FFA1` | Success, ON states |
| **Text primary** | `textPrimary` | `#E8E8FF` | Main text (slight blue tint) |
| **Text secondary** | `textSecondary` | `#8888AA` | Labels, muted text |
| **Text muted** | `textMuted` | `#44445A` | Disabled text |

**Glow colors** (same as accents but with alpha):
- `neonCyan` with `0.3f` alpha for glow effects
- `neonPink` with `0.3f` alpha for macro-assigned glow

---

## 2. Typography

| Usage | Size | Weight | Color |
|---|---|---|---|
| **Section titles** | 10px | Bold | `textSecondary` |
| **Knob labels** | 9px | Plain | `textSecondary` |
| **Value pill text** | 9px | Plain | `textPrimary` |
| **Button text** | 11px | Plain | `textSecondary` (active: `neonCyan`) |
| **Tab text** | 11px | Plain | `textSecondary` (active: `neonCyan`) |
| **Macro names** | 12px | Bold | `textPrimary` |

*Font: Use system default (handled via `juce::FontOptions` in C++)*

---

## 3. Core Components (Figma Specs)

### 3.1 Neumorphic Card (Section Background)
- **Size**: Variable (per panel section)
- **Corner radius**: 8px
- **Fill**: `bgRaised` (`#1A1A2E`)
- **Shadows** (Figma drop shadow ×2):
  - Dark: X=+3, Y=+3, Blur=6, Color=`shadowDark` 65% opacity
  - Light: X=-3, Y=-3, Blur=6, Color=`shadowLight` 45% opacity
- **Border** (optional glow): 1px solid `neonCyan` at 20% opacity
- **Section title**: 10px bold, `textSecondary`, positioned at X=8, Y=5 from top-left

### 3.2 NeuKnob (Rotary Slider) — KEY COMPONENT
Based on `NeuKnob` + `CyberpunkTheme::drawRotarySlider`:

**Outer disc:**
- **Size**: 60-80px diameter (per panel)
- **Fill**: Radial gradient from `bgRaised` brighter(8%) at top-left → `bgRaised` darker(12%) at bottom-right
- **Neumorphic shadows** (same as card but elliptical):
  - Dark: X=+4, Y=+4, Blur=8, `shadowDark` 70%
  - Light: X=-4, Y=-4, Blur=8, `shadowLight` 55%

**Arc groove (270° sweep):**
- **Radius**: 70% of disc radius
- **Track width**: 3px
- **Groove outer**: `shadowDark` 80%, width 5px (depth)
- **Groove fill**: `bgDeep`, width 3px

**Fill arc (neon glow):**
- **Color**: `neonCyan` (`#00F0FF`) — or `neonPink` when macro-assigned
- **Width**: 3px
- **Glow effect** (Figma): Outer glow, `neonCyan` 25% opacity, spread 5px

**Tip dot (end of fill arc):**
- **Size**: 6px circle
- **Fill**: `neonCyan` 70% opacity
- **Glow**: Outer glow 10px, `neonCyan` 35%

**Center cap:**
- **Size**: 40% of disc radius
- **Fill**: Radial gradient, `bgRaised` brighter(18%) → `bgRaised`
- **LED dot**: 5px circle at center
  - OFF: `bgDeep` (`#12121F`)
  - ON (value > 0): `neonCyan` with glow

**Value pill (appears on hover/drag):**
- **Position**: Centered above knob, Y=2px
- **Size**: Auto-width (min 40px) × 15px
- **Fill**: `bgDeep`
- **Border**: 1px `neonCyan` 50% opacity
- **Text**: 9px, `textPrimary`, centered

### 3.3 Linear Slider (Horizontal/Vertical)
- **Track height**: 5px (horizontal) / width 5px (vertical)
- **Track fill**: Inset neumorphic (dark shadow + light rim)
- **Fill**: `neonCyan` gradient (horizontal: left→right, vertical: bottom→top)
- **Thumb**: 11×20px (H) or 20×11px (V)
  - Neumorphic shadows (same as card)
  - Fill: `bgRaised` gradient
  - Border: 1px `shadowLight` 40%

### 3.4 Toggle Button (LED Pill)
- **Shape**: Rounded rectangle, height=20px, corner radius=10px
- **OFF**: Fill=`bgRaised`, neumorphic shadows
- **ON**: Fill=`bgDeep`, tint overlay `neonCyan` 18% opacity
- **LED dot**: 7px circle
  - OFF: `bgDeep` (dark)
  - ON: `neonCyan` with glow (outer glow 5px, 35%)
- **Text**: 11px, left-aligned after LED dot, `textSecondary` (ON: `textPrimary`)

### 3.5 Text Button
- **Shape**: Rounded rectangle, corner radius=6px
- **Normal**: Fill=`bgRaised`, neumorphic shadows (X=±3, Y=±3, Blur=6)
- **Hover**: Fill=`bgRaised` brighter(5%)
- **Pressed/Active**: Fill=`bgDeep`, tint overlay `neonCyan` 15%
- **Text**: 11px, centered, `textSecondary` (active: `neonCyan`)

### 3.6 ComboBox (Dropdown)
- **Shape**: Rounded rectangle, corner radius=6px
- **Fill**: `bgRaised`, neumorphic shadows
- **Border**: 1px `shadowLight` 35% (on hover: `neonCyan` 50%)
- **Arrow**: Downward triangle, `textSecondary`
- **Dropdown menu**: `bgRaised` fill, `shadowLight` border, highlighted row: `neonCyan` 18%

### 3.7 Tab Bar (Top of Plugin)
- **Background**: `bgVoid` (no fill) or `bgBase` darker(8%)
- **Bottom edge**: 1px `shadowLight` 40%
- **Tab button**:
  - Normal: transparent, text `textSecondary`
  - Hover: `shadowLight` 20% fill
  - Active: `neonCyan` 13% fill + 2px bottom border `neonCyan`
  - Text (active): `neonCyan`

### 3.8 Labels
- **Text color**: `textSecondary` (or `textLabel` for section headers)
- **No background** (transparent)

---

## 4. Panel Layouts (Figma Frame Reference)

### Main Window: 1200×800px
- **Background**: `bgVoid` (`#0A0A12`)
- **Tab bar**: Top 32px, `bgBase`
- **Content area**: Remaining 768px, `bgBase` fill

### Panel Layout Pattern (8 panels + 2 special):
Each panel uses **collapsible neumorphic cards** with 16px padding, 10px gaps.

| Panel | # Cards | Card Sections |
|---|---|---|
| **Synth** | 5 | OSC (3 strips), UNISON, FILTER, ENV, FM OPS (conditional) |
| **Drums** | 4 | Transport, Swing/Quant/Chain, Pattern slots, Step grid + 8 track rows |
| **Modulation** | Dynamic | Header + column headers + dynamic rows (addable) |
| **Sampler** | 3 | Drop zone, Zone list, Controls |
| **Sequencer** | 4 | Transport, Pattern slots, Step grid, Export |
| **Pro Seq** | 4 | Transport, Lane/mode, Step grid (4 lanes), Step editor |
| **Arp** | 3 | Controls, Step grid (2 rows × 16), Step editor |
| **Effects** | 6 + 1 | Chain strip (top) + Chorus, Distortion, EQ, Compressor, Delay, Reverb |
| **Granular** | 3 | Source (load), Grain controls, Voice Envelope |
| **Macros** | 1 | 2×2 grid of 4 macro knobs |

---

## 5. Neon Glow Effects (Figma)

### 5.1 Knob Arc Glow
- When value > 0: Apply outer glow to fill arc
  - Color: `neonCyan` (or `neonPink` for macro)
  - Opacity: 25%
  - Blur: 5px

### 5.2 LED Dot Glow
- Active LED: Outer glow 10px, `neonCyan` 35%

### 5.3 Active Tab/Button Glow
- Subtle outer glow 3px, `neonCyan` 20%

### 5.4 Card Border Glow (Optional)
- 1px stroke on card border, `neonCyan` 20% opacity
- Can pulse gently (animation — future enhancement)

---

## 6. Cyberpunk-Specific Enhancements

### 6.1 Scanline Overlay (subtle)
- Optional: Add 1px lines every 4px, `bgVoid` 3% opacity
- Gives CRT/cyberpunk feel

### 6.2 Hexagonal Accents
- Small hex shapes in corner of cards (decorative only)
- Color: `neonCyan` 10% opacity

### 6.3 Data Display (SynthDisplay, Oscilloscope)
- Oscilloscope stroke: `neonCyan` with glow 25%
- FFT bars: Gradient `neonPurple` → `neonCyan`
- Grid lines: `textMuted` 8% opacity

---

## 7. Implementation Plan (Next Session)

### 7.1 New Theme Class
Create `CyberpunkTheme : public juce::LookAndFeel_V4`
- Replace all `CyberpunkTheme` references with `CyberpunkTheme`
- Update color constants to new palette
- Keep `drawNeumorphicRect()` / `drawNeumorphicEllipse()` helpers (update colors)

### 7.2 Files to Modify
1. **New file**: `Source/CyberpunkTheme.h` + `.cpp` (based on CyberpunkTheme)
2. **Modify**: `PluginEditor.cpp` — `setLookAndFeel(&cyberpunkTheme)`
3. **Modify**: `NeuKnob.cpp` — use `neonCyan` / `neonPink` for arcs
4. **Modify**: All 10 panel `paint()` methods — use new color constants
5. **Modify**: All panel headers — update section card colors

### 7.3 Key Changes
- Macro-assigned: Use `neonPink` instead of `accentAmber`
- Knob arcs: `neonCyan` primary, `neonPink` for macro
- Background: `bgVoid` (`#0A0A12`) for main window
- Cards: `bgRaised` (`#1A1A2E`) with neon glow borders

### 7.4 Component Count to Update
- `NeuKnob`: ~63+ instances across all panels
- `CyberpunkTheme::drawNeumorphicRect` calls: ~20+ (all panel paint() methods)
- `drawRotarySlider`: 1 (in theme, uses new colors)
- `drawLinearSlider`: 1 (in theme, uses new colors)
- All label/text colors throughout

---

## 8. Session Checklist (For Next Time)

- [ ] Create `CyberpunkTheme.h` + `.cpp`
- [ ] Update `PluginEditor` to use new theme
- [ ] Update `NeuKnob` arc colors
- [ ] Update all 10 panel `paint()` methods
- [ ] Test build (VST3 + AU)
- [ ] Verify all knobs/sliders render correctly
- [ ] Push to GitHub with tag `cyberpunk-theme-complete`

---

**Design by:** opencode/big-pickle  
**Date:** 2026-05-03  
**Status:** Ready for implementation
