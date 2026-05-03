# Figma Build Guide — Neumorphic Cyberpunk UI

**No screenshots needed.** Build from scratch using these specs.

---

## Build Order

1. **Design System Page** — Colors, typography, effects
2. **Components Page** — Neumorphic card, NeuKnob, buttons, tabs
3. **Panel Mockups** — 10 panels, one by one

---

## 1. Design System Page

### Frame: `Design System` (1920×1080)

### Color Styles (create all as Figma color styles):

| Name | Hex | Notes |
|------|-----|-------|
| `bgVoid` | `#0A0A12` | Plugin window fill |
| `bgBase` | `#0D0D1A` | Panel background |
| `bgRaised` | `#1A1A2E` | Cards, knobs (neumorphic surface) |
| `bgDeep` | `#12121F` | Inset areas |
| `shadowDark` | `#050510` | Neumorphic dark shadow |
| `shadowLight` | `#25253D` | Neumorphic light highlight |
| `neonCyan` | `#00F0FF` | Primary accent |
| `neonPink` | `#FF2A6D` | Secondary accent |
| `neonPurple` | `#B026FF` | Tertiary accent |
| `neonGreen` | `#05FFA1` | Success states |
| `textPrimary` | `#E8E8FF` | Main text |
| `textSecondary` | `#8888AA` | Labels |
| `textMuted` | `#44445A` | Disabled text |

### Text Styles (create all as Figma text styles):

| Name | Font | Size | Weight | Color |
|------|------|------|--------|-------|
| `Section Title` | Inter/System | 10px | Bold | `textSecondary` |
| `Knob Label` | Inter/System | 9px | Regular | `textSecondary` |
| `Value Pill` | Inter/System | 9px | Regular | `textPrimary` |
| `Button Text` | Inter/System | 11px | Regular | `textSecondary` |
| `Tab Text` | Inter/System | 11px | Regular | `textSecondary` |
| `Macro Name` | Inter/System | 12px | Bold | `textPrimary` |

### Effects (save as effect presets):

**Neumorphic Shadow — Dark:**
- Type: Drop Shadow
- X: +3, Y: +3
- Blur: 6
- Color: `shadowDark`
- Opacity: 65%

**Neumorphic Shadow — Light:**
- Type: Drop Shadow
- X: -3, Y: -3
- Blur: 6
- Color: `shadowLight`
- Opacity: 45%

**Neon Glow — Cyan:**
- Type: Drop Shadow
- X: 0, Y: 0
- Blur: 5px
- Color: `neonCyan`
- Opacity: 25%

**Neon Glow — Pink:**
- Type: Drop Shadow
- X: 0, Y: 0
- Blur: 5px
- Color: `neonPink`
- Opacity: 25%

**LED Glow:**
- Type: Drop Shadow
- X: 0, Y: 0
- Blur: 10px
- Color: `neonCyan`
- Opacity: 35%

---

## 2. Components Page

### Frame: `Components` (1920×1200)

### 2.1 Neumorphic Card

**Frame:** 200×150px, corner radius 8px
- Fill: `bgRaised`
- Effects:
  - Neumorphic Shadow — Dark
  - Neumorphic Shadow — Light
- Optional border: 1px solid `neonCyan` at 20% opacity

**Section Title** (inside card, top-left):
- Text: "SECTION" (10px bold, `textSecondary`)
- Position: X=8, Y=8

### 2.2 NeuKnob (Key Component)

**Main disc:** 60-80px circle
- Fill: Radial gradient
  - Center: `bgRaised` lighter by 8%
  - Edge: `bgRaised` darker by 12%
- Effects:
  - Neumorphic Shadow — Dark (X=+4, Y=+4, Blur=8, 70%)
  - Neumorphic Shadow — Light (X=-4, Y=-4, Blur=8, 55%)

**Arc groove (270° sweep, 70% radius):**
- Stroke: `bgDeep`, 3px
- Behind it: `shadowDark`, 5px (depth)

**Fill arc (value indicator):**
- Stroke: `neonCyan`, 3px
- Effect: Neon Glow — Cyan

**Tip dot (end of arc):**
- Circle: 6px
- Fill: `neonCyan` 70%
- Effect: LED Glow

**Center cap:** 40% of disc radius
- Fill: Radial gradient (`bgRaised` +18% → `bgRaised`)
- LED dot: 5px circle, `bgDeep` (off) / `neonCyan` (on)

**Value pill (hover state):**
- Rectangle: auto-width × 15px, corner radius 4px
- Fill: `bgDeep`
- Border: 1px `neonCyan` 50%
- Text: 9px, `textPrimary`, centered

### 2.3 Linear Slider (Horizontal)

**Track:** 100×5px
- Fill: Inset neumorphic (`shadowDark` + `shadowLight`)
- Fill (active): `neonCyan` gradient left→right

**Thumb:** 11×20px
- Fill: `bgRaised` gradient
- Effects: Neumorphic shadows (same as card)
- Border: 1px `shadowLight` 40%

### 2.4 Toggle Button (LED Pill)

**Frame:** auto-width × 20px, corner radius 10px
- OFF: Fill `bgRaised` + neumorphic shadows
- ON: Fill `bgDeep` + `neonCyan` 18% tint overlay
- LED dot: 7px circle (`bgDeep` off / `neonCyan` on + glow)
- Text: 11px, left-aligned after dot (`textSecondary` / `textPrimary`)

### 2.5 Text Button

**Frame:** auto-width × 28px, corner radius 6px
- Fill: `bgRaised`
- Effects: Neumorphic shadows (X=±3, Y=±3, Blur=6)
- Hover: Fill `bgRaised` lighter 5%
- Pressed: Fill `bgDeep` + `neonCyan` 15% tint
- Text: 11px centered, `textSecondary` (active: `neonCyan`)

### 2.6 ComboBox (Dropdown)

**Frame:** 120×28px, corner radius 6px
- Fill: `bgRaised`
- Effects: Neumorphic shadows
- Border: 1px `shadowLight` 35% (hover: `neonCyan` 50%)
- Arrow: Downward triangle, `textSecondary`

**Dropdown menu:**
- Fill: `bgRaised`
- Border: 1px `shadowLight`
- Highlighted row: `neonCyan` 18%

### 2.7 Tab Bar

**Background:** 1200×32px
- Fill: `bgVoid` or `bgBase` darker 8%
- Bottom edge: 1px `shadowLight` 40%

**Tab button:** auto-width × 32px
- Normal: transparent, text `textSecondary`
- Hover: `shadowLight` 20% fill
- Active: `neonCyan` 13% fill + 2px bottom border `neonCyan`
- Text (active): `neonCyan`

---

## 3. Panel Mockups

### Main Window: 1200×800px
- Background: `bgVoid`
- Tab bar: 1200×32px at top
- Content area: 1200×768px, fill `bgBase`

### Panel Layout Pattern
Each panel uses collapsible neumorphic cards:
- Padding: 16px
- Gap between cards: 10px
- Cards: Use "Neumorphic Card" component

### Panel Frame Sizes (per `AI_CYBERPUNK_PLAN.md` §4):

| Panel | Cards | Layout Notes |
|-------|-------|--------------|
| **Synth** | 5 | OSC (3 strips), UNISON, FILTER, ENV, FM OPS (conditional) |
| **Drums** | 4 | Transport, Swing/Quant/Chain, Pattern slots, Step grid + 8 tracks |
| **Modulation** | Dynamic | Header + column headers + dynamic rows |
| **Sampler** | 3 | Drop zone, Zone list, Controls |
| **Sequencer** | 4 | Transport, Pattern slots, Step grid, Export |
| **Pro Seq** | 4 | Transport, Lane/mode, Step grid (4 lanes), Step editor |
| **Arp** | 3 | Controls, Step grid (2×16), Step editor |
| **Effects** | 6+1 | Chain strip (top) + Chorus, Distortion, EQ, Compressor, Delay, Reverb |
| **Granular** | 3 | Source (load), Grain controls, Voice Envelope |
| **Macros** | 1 | 2×2 grid of 4 macro knobs |

---

## 4. Figma Component Naming Convention

Use this naming for auto-layout components:

```
Neumorphic Card
├── Section Title
└── Content (auto-layout vertical)

NeuKnob
├── Disc (ellipse)
├── Arc Groove (arc/path)
├── Fill Arc (arc/path)
├── Tip Dot (ellipse)
├── Center Cap (ellipse)
│   └── LED Dot (ellipse)
└── Value Pill (instance swap)

Toggle Pill
├── Background (rectangle)
├── LED Dot (ellipse)
└── Label (text)
```

---

## 5. Build Checklist

- [ ] Create all color styles (12 colors)
- [ ] Create all text styles (6 styles)
- [ ] Save effect presets (5 effects)
- [ ] Build Neumorphic Card component
- [ ] Build NeuKnob component (most complex)
- [ ] Build Linear Slider component
- [ ] Build Toggle Pill component
- [ ] Build Text Button component
- [ ] Build ComboBox component
- [ ] Build Tab Bar component
- [ ] Mockup SynthPanel (proof-of-concept)
- [ ] **GET SIGN-OFF** on SynthPanel
- [ ] Mockup remaining 9 panels
- [ ] **GET FINAL SIGN-OFF**

---

## 6. Notes for Figma

- All knobs: Use 270° arc (not 360°)
- Neon glow: Multiple shadow layers for intensity
- Cards: Apply both shadows simultaneously (dark + light)
- Use Figma auto-layout for all panels (easier iteration)
- Create component variants for ON/OFF states
- Use Figma's "Instance" feature to reuse components across panels

---

**When Figma designs are approved:** Implement C++ in `CyberpunkTheme.h/.cpp` to match these specs exactly.
