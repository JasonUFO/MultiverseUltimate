# Multiverse Ultimate — Figma Design Brief
# Complete UI Replacement Spec for Cyberpunk/Neon Theme

**Date:** 2026-05-04  
**Purpose:** Complete UI redesign brief for Figma AI / design team  
**Goal:** Match/surpass Serum 2, Nexus 5, Avenger 2 visual quality

---

## 1. Design System

### 1.1 Color Palette (Exact Hex Values)
```
bgVoid:      #0A0A12  (deepest background - plugin window fill)
bgBase:      #0D0D1A  (panel/component background)
bgRaised:    #1A1A2E  (cards, knobs - neumorphic surface)
bgDeep:      #12121F  (inset areas, text boxes)
shadowDark:  #050510  (neumorphic dark shadow)
shadowLight: #25253D (neumorphic light highlight)

neonCyan:    #00F0FF  (primary accent, arcs, glow, active tabs)
neonPink:    #FF2A6D  (secondary accent, macro-assigned)
neonPurple:  #B026FF  (tertiary accent, FM sections)
neonGreen:   #05FFA1  (success states, ON states)

textPrimary:   #E8E8FF (main text - slight blue tint)
textSecondary: #8888AA (labels, muted text)
textMuted:     #44445A (disabled text)
```

### 1.2 Typography
```
Section titles:  10px, Bold, textSecondary
Knob labels:     9px, Plain, textSecondary
Value pill text:  9px, Plain, textPrimary
Button text:     11px, Plain, textSecondary (active: neonCyan)
Tab text:        11px, Plain, textSecondary (active: neonCyan)
Macro names:     12px, Bold, textPrimary
```

### 1.3 Core Component Styles

#### Neumorphic Card (Section Background)
- **Corner radius:** 8px
- **Fill:** bgRaised (#1A1A2E)
- **Shadows:**
  - Dark: X=+3, Y=+3, Blur=6, shadowDark 65% opacity
  - Light: X=-3, Y=-3, Blur=6, shadowLight 45% opacity
- **Optional glow border:** 1px solid neonCyan at 20% opacity
- **Section title:** 10px bold, textSecondary, X=8, Y=5 from top-left

#### NeuKnob (Rotary Slider) — PRIMARY COMPONENT
- **Size:** 60-80px diameter (per panel context)
- **Outer disc fill:** Radial gradient from bgRaised+8% at top-left → bgRaised-12% at bottom-right
- **Arc groove (270° sweep):**
  - Radius: 70% of disc radius
  - Track width: 3px
  - Groove outer: shadowDark 80%, width 5px
  - Groove fill: bgDeep, width 3px
- **Fill arc (neon glow):**
  - Color: neonCyan (or neonPink when macro-assigned)
  - Width: 3px
  - Glow: outer glow, neonCyan 25% opacity, spread 5px
- **Tip dot:** 6px circle, neonCyan 70% opacity + glow
- **Center cap:** 40% of disc radius, radial gradient bgRaised+18% → bgRaised
- **LED dot:** 5px at center — OFF: bgDeep, ON: neonCyan with glow
- **Value pill (hover/drag):** Centered above knob, 40px×15px min, bgDeep fill, 1px neonCyan 50% border, 9px textPrimary text

#### Linear Slider (Horizontal/Vertical)
- **Track:** 5px height (H) or width (V)
- **Track fill:** Inset neumorphic (dark shadow + light rim), bgDeep fill
- **Fill:** neonCyan gradient (left→right or bottom→top)
- **Thumb:** 11×20px (H) or 20×11px (V), neumorphic shadows, bgRaised gradient fill

#### Toggle Button (LED Pill)
- **Shape:** Rounded rect, height=20px, corner radius=10px
- **OFF:** bgRaised fill, neumorphic shadows
- **ON:** bgDeep fill, neonCyan 18% tint overlay
- **LED dot:** 7px circle — OFF: bgDeep, ON: neonCyan with 5px glow
- **Text:** 11px, left-aligned after LED, textSecondary (ON: textPrimary)

#### ComboBox (Dropdown)
- **Shape:** Rounded rect, corner radius=6px
- **Fill:** bgRaised, neumorphic shadows
- **Border:** 1px shadowLight 35% (hover: neonCyan 50%)
- **Arrow:** Downward triangle, textSecondary
- **Dropdown menu:** bgRaised fill, shadowLight border, highlighted row: neonCyan 18%

#### Text Button
- **Shape:** Rounded rect, corner radius=6px
- **Normal:** bgRaised, neumorphic shadows (X=±3, Y=±3, Blur=6)
- **Hover:** bgRaised+5%
- **Active:** bgDeep, neonCyan 15% tint
- **Text:** 11px, centered, textSecondary (active: neonCyan)

---

## 2. Window Layout (1200×800px)

```
┌─────────────────────────────────────────────────────────────┐
│  Header [32px]                                            │
│  [MIDI Learn] [Presets] [?]                        [DAW]  │
├─────────────────────────────────────────────────────────────┤
│  Preset Browser [220px] — collapsible, only when active    │
│  Search bar + Category pills + Preset list                 │
├─────────────────────────────────────────────────────────────┤
│  Tab Bar [32px]                                           │
│  [Synth] [Drums] [Modulation] [Sampler] [Sequencer]       │
│  [Arp] [Effects] [Macros] [Granular] [Layers]            │
├─────────────────────────────────────────────────────────────┤
│                                                           │
│  TAB CONTENT AREA [~736px]                               │
│  (varies per tab — see section specs below)                 │
│                                                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Panel Specifications

### 3.1 SYNTH Panel (Primary Panel)

**Layout:** Vertical stack of section cards (16px padding, 10px gaps)

#### Header Strip [46px]
```
┌─────────────────────────────────────────────────────────────┐
│ MODE [Classic▼]  [Save] [Load]    │ VOICE [Poly▼]        │
│                                 │ PORTA [====|____] [Always] │
│                                 │ MPE [○]                 │
└─────────────────────────────────────────────────────────────┘
```

#### OSC Section [190px] — 3 oscillator strips in 3 columns
**Each OSC strip (1-3):**
```
┌─────────────────┐
│ OSC 1     [Classic▼]                      │
│ ┌─────┐   Level: 0.75    Detune: +3st    │
│ │     │   Waveform: [Saw▼]  WavePos: 0.5  │
│ │KNB  │   [LOAD WT] [EDIT WT]             │
│ └─────┘   file: saw.wav                    │
└─────────────────┘
```
- **Attachments:** osc1Type, osc1Level, osc1Detune, osc1Waveform, osc1WavePos (×3)

#### SUB/NOISE Section [126px] — 2 columns
**SUB column:**
- Enable toggle, Waveform [Sine/Square], Level knob

**NOISE column:**
- Enable toggle, Level knob, Color knob (200Hz-20kHz)

#### UNISON Section [120px]
- Voices [1-8▼], Detune knob, Width knob, Spread mode [Stacked/Chord/Random▼]

#### FILTER Section [185px]
- Cutoff knob (20-20kHz, skew 0.3), Resonance knob (0.1-10), Type [LP/HP/BP/Notch▼], OS [Off/2x/4x/Auto▼]

#### ENV Section [130px] — CLASSIC MODE ONLY
- Attack, Decay, Sustain, Release knobs (skew 0.4)

#### FM Section — FM MODE ONLY
```
┌─────────────────────────────────────────────────────────────┐
│ FM ALGORITHM [1▼]                                        │
│ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐    │
│ │  OP 1    │ │  OP 2    │ │  OP 3    │ │  OP 4    │    │
│ │ Ratio: 2x │ │ Ratio: 1x │ │ Ratio: 3x │ │ Ratio: 0.5│    │
│ │ Level: 80%│ │ Level: 60%│ │ Level: 40%│ │ Level: 20%│    │
│ │ Fdbk: 0% │ │ Fdbk: 0% │ │ Fdbk: 0% │ │ Fdbk: 0% │    │
│ │ A  D  S  R│ │ A  D  S  R│ │ A  D  S  R│ │ A  D  S  R│    │
│ └──────────┘ └──────────┘ └──────────┘ └──────────┘    │
└─────────────────────────────────────────────────────────────┘
```

#### SynthDisplay [90px] — at bottom of Synth panel
- **Left 45%:** Oscilloscope (neonCyan 1.5px stroke + glow)
- **Right 55%:** FFT spectrum (neonPurple→neonCyan gradient bars, 20Hz-22kHz log scale)

---

### 3.2 DRUMS Panel (DrumSequencer)

**Layout:** Vertical stack

#### Transport Row [28px]
```
BPM [===|___] [▶ Play] [■ Stop] Step: 5  [Copy] [Paste]
```

#### Swing/Quant Row [24px]
```
Swing [===|___]  Quant [1/16▼]  [Chain ON]  [1][2][3][4][5][6][7][8]
```

#### Pattern Row [22px]
```
[1][2][3][4][5][6][7][8]  (toggle buttons, active = neonCyan)
```

#### Track Rows + Step Grid
**8 Tracks, each with:**
```
Track 1: "Kick"  Vol[===|___] [M] [S] [Load]  [■■■■ □□□□□□□□□□□□] (16 steps)
Track 2: "Snare" Vol[===|___] [M] [S] [Load]  [□□□■ ■□□□□□□□□□□□] (16 steps)
...
```
- **Step buttons:** DrumStepButton, velocity shown via color intensity (dark blue = low, bright cyan = high)
- **Current step:** Highlighted amber
- **Right-click:** Velocity menu (25%, 50%, 75%, 100%, Custom)

---

### 3.3 MODULATION Panel (ModulationMatrix)

**Layout:** Dynamic rows

```
┌─────────────────────────────────────────────────────────────┐
│ MODULATION MATRIX                    [+ Add]                │
│ SOURCE          TARGET               AMOUNT                │
│ ─────────────────────────────────────────────────────────  │
│ [LFO1▼]   →   [Filter Cutoff▼]    [===|___] [X]        │
│ [Velocity▼] →  [Volume▼]            [===|___] [X]        │
│ [Random▼]  →  [Pitch▼]             [===|___] [X]        │
│ ... (add more rows dynamically)                             │
└─────────────────────────────────────────────────────────────┘
```

**Sources:** LFO1, LFO2, Envelope, Velocity, Random, Note Number, ModWheel, PitchBend, SustainPedal, Aftertouch, MPEPressure, MPESlide, EnvelopeFollower

**Targets (24+):** Pitch, Filter Cutoff, Resonance, Volume, Pan, All Effect Params, GranularPosition, GranularDensity, GranularGrainSize, GranularSpray, GranularPitchScatter, and more

---

### 3.4 SAMPLER Panel

**Layout:** 3 section cards

#### Drop Zone [54px]
```
┌─────────────────────────────────────────────────────────────┐
│  Drop audio files here (.wav/.aiff/.flac/.ogg/.mp3)      │
│  (or click to browse)                                      │
└─────────────────────────────────────────────────────────────┘
```

#### Zone List [110px]
- ListBox showing all loaded sample zones with name and note range

#### Controls Section
```
Loop: [None▼]  Root: [C3▼]  Start: [===|___]  End: [===|___]  Xfade: [===|___]
Global:  Vol: [===|___]  Pan: [===|___]  [Clear All]
```

---

### 3.5 SEQUENCER Panel (ProSequencer)

**Layout:** 4 sections

#### Transport Row [26px]
```
BPM [===|___] [▶ Play] [■ Stop]  [Export MIDI]
```

#### Lane/Mode Row [28px]
```
[LANE 1] [LANE 2] [LANE 3] [LANE 4]    Mode: [FWD▼]  Steps: [32▼]
```

#### Step Grid (2 rows × 16 = 32 steps)
```
Steps 1-16:  [C3][D3][■ ][E3][■ ][F3][G3][■ ][A3][B3][■ ][C4][D4][E4][F4][G4]
Steps 17-32: [■ ][A4][B4][C5][■ ][D5][E5][■ ][F5][G5][A5][B5][C6][D6][E6][F6]
```
- **Active step:** Shows note name + velocity bar (blue)
- **Current step:** Highlighted amber
- **Right-click:** Opens note selection menu

#### Step Editor [116px] — when step selected
```
Step 5, Lane 2
Note: [C3▼]  Velocity: [===|___]  Gate: [===|___]  Prob: [===|___]  Ratchet: [1×▼]
```

---

### 3.6 ARP Panel (Arpeggiator)

**Layout:** 3 sections

#### Controls Row [24px]
```
[ARP ON]  Mode: [Up▼]  Steps: [32▼]
```

#### Step Grid (2 rows × 16 = 32 steps)
```
[■][ ][ ][■][ ][ ][■][ ][ ][■][ ][ ][■][ ][ ][■][ ][ ]  (steps 1-16)
[ ][■][ ][ ][■][ ][ ][■][ ][ ][■][ ][ ][■][ ][ ][■][ ]  (steps 17-32)
```

#### Step Editor [100px] — when step selected
```
Step 12
Note: [+3▼]  Octave: [0▼]  Velocity: [===|___]  Gate: [===|___]  [Tie ON]
```

---

### 3.7 EFFECTS Panel

**Layout:** Chain strip + 2 columns of effect sections

#### Effect Chain Strip [56px]
```
┌─────────────────────────────────────────────────────────────┐
│ CHORUS → DIST → EQ → COMP → DELAY → REVERB              │
│ (drag tiles to reorder — order affects audio processing)     │
└─────────────────────────────────────────────────────────────┘
```

#### Left Column: Chorus, Distortion, 3-Band EQ
#### Right Column: Compressor, Delay, Reverb (2 rows)

**Chorus card:**
- Rate knob (0.1-5 Hz), Depth knob, Mix knob

**Distortion card:**
- Drive knob (1-100×), Tone knob, Mix knob

**3-Band EQ card:**
- Low knob (±12dB @ 250Hz), Mid knob (±12dB @ 1kHz), High knob (±12dB @ 4kHz)

**Compressor card:**
- Threshold knob (-60-0 dB), Ratio knob (1:1-20:1), Attack knob, Release knob, Makeup knob

**Delay card (Row 1):**
- Time knob (0-2s), Feedback knob, Mix knob

**Reverb card (Row 1):**
- Room knob, HF Damp knob, Wet knob

**Reverb card (Row 2):**
- Pre-Delay knob (0-200ms), LF Damp knob, Width knob, [Freeze] toggle

---

### 3.8 MACROS Panel

**Layout:** 2×4 grid of macro knobs

```
┌─────────────────────────────────────────────────────────────┐
│ MACRO CONTROLS                                           │
│ (right-click any knob/slider in the plugin to assign it     │
│  to a macro)                                             │
│                                                           │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐                  │
│  │ K1  │  │ K2  │  │ K3  │  │ K4  │                  │
│  │Macro1│  │Macro2│  │Macro3│  │Macro4│                  │
│  │ 3    │  │ 5    │  │ 2    │  │ 8    │                  │
│  └─────┘  └─────┘  └─────┘  └─────┘                  │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐                  │
│  │ K5  │  │ K6  │  │ K7  │  │ K8  │                  │
│  │Macro5│  │Macro6│  │Macro7│  │Macro8│                  │
│  │ 1    │  │ 4    │  │ 6    │  │ 2    │                  │
│  └─────┘  └─────┘  └─────┘  └─────┘                  │
└─────────────────────────────────────────────────────────────┘
```
- **Knob:** Rotary, APVTS param macro1-macro8 (0→1)
- **Name label:** Double-click to rename
- **Assign button:** Shows count of assigned params, click to view/remove

---

### 3.9 GRANULAR Panel

**Layout:** 3 section cards

#### Source Section [34px]
```
[LOAD GR]  file: "sine_sweep.wav"  (or "(built-in default)")
```

#### Grain Controls [2 rows]
**Row 1:**
- Position knob, Grain Size knob, Spray knob, Density knob

**Row 2:**
- Pitch Scatter knob, Stereo Spread knob, Env Shape [Gaussian▼], [Reverse] toggle

#### Voice Envelope [ADSR]
- Attack, Decay, Sustain, Release knobs

---

### 3.10 LAYERS Panel

**Layout:** 8 layer rows

```
┌─────────────────────────────────────────────────────────────┐
│ LAYERS                                                    │
│                                                           │
│ Layer 1 [Synth▼] Vol[=|__] Pan[=|__] [M][S] Note[C3-C5▼] │
│           Vel[0-127▼] Ch[All▼] [FX] [Preset]  [L/R meters]│
│                                                           │
│ Layer 2 [Off▼]   Vol[=|__] Pan[=|__] [M][S] Note[---▼]  │
│           Vel[--] Ch[--] [FX] [Preset]  [L/R meters]      │
│ ... (6 more rows)                                          │
└─────────────────────────────────────────────────────────────┘
```

**Engine types:** Off / Synth / Granular / Sampler

**FX Popup (when FX button clicked):**
```
┌─────────────────────────────────────────┐
│ Layer Effects                          │
│ [Chorus ON] [Dist OFF] [EQ ON]         │
│ [Comp OFF] [Delay ON] [Reverb OFF]     │
│ Chorus Mix: [=|__]  Dist Mix: [=|__]  │
│ EQ Mix: [=|__]  Comp Mix: [=|__]      │
│ Delay Mix: [=|__]  Reverb Mix: [=|__] │
└─────────────────────────────────────────┘
```

---

## 4. Figma AI / Design Tool Prompt

Use this prompt to generate the Figma design:

```
Create a cyberpunk/neon themed VST synth plugin UI (1200×800px) called "Multiverse Ultimate".

COLOR PALETTE:
- Background: #0A0A12 (bgVoid), panels: #0D0D1A (bgBase)
- Cards/surfaces: #1A1A2E (bgRaised), insets: #12121F (bgDeep)
- Neon Cyan: #00F0FF (primary), Neon Pink: #FF2A6D (macro), Neon Purple: #B026FF (FM)
- Text: #E8E8FF (primary), #8888AA (secondary), #44445A (muted)

TYPOGRAPHY: Inter or system sans-serif, 9-12px

COMPONENTS TO DESIGN (all in cyberpunk/neon style):
1. Neumorphic cards (8px radius, dual shadows, optional neon border)
2. Rotary knobs with 270° arc, neon glow, LED dot center, value pill on hover
3. Horizontal/vertical sliders with neon fill, neumorphic thumbs
4. LED pill toggles (7px LED dot + text)
5. ComboBox dropdowns with neon-highlighted rows
6. Text buttons with neumorphic style + neon active state
7. Step sequencer grid (16/32 steps, velocity coloring, current step highlight)
8. Oscilloscope + FFT spectrum display (neon glow on waveforms)

TAB LAYOUT (10 tabs):
Synth | Drums | Modulation | Sampler | Sequencer | Arp | Effects | Macros | Granular | Layers

EACH PANEL MUST HAVE:
- Neumorphic section cards with 8px radius
- Section titles (10px bold, textSecondary, X=8 Y=5)
- Precise component layout matching the specs in this document
- All knobs must show the parameter name below and current value on hover
- Active states must use neonCyan (#00F0FF) with glow effects
- Macro-assigned controls use neonPink (#FF2A6D) instead of cyan

REFERENCE QUALITY: Serum 2, Nexus 5, Avenger 2, Diva, Zebra 3
The design must feel dark, futuristic, with glowing neon elements on dark neumorphic surfaces.
Include scanline overlay option (1px lines every 4px, bgVoid 3% opacity) for CRT/cyberpunk feel.
```

---

## 5. Component Count Summary

| Panel | Section Cards | Knobs/Sliders | Buttons | ComboBoxes | Special Displays |
|-------|---------------|----------------|---------|-------------|------------------|
| Synth | 5-6 | ~25 | ~16 | 8 | SynthDisplay, WavetableEditor ×3 |
| Drums | 4 | 8 | ~26 | 2 | 128 step buttons (8×16) |
| Modulation | Dynamic | 1/row | 1/row + add | 2/row | Dynamic row list |
| Sampler | 3 | 7 | 3 | 2 | Zone ListBox |
| Sequencer | 4 | 4 | ~38 | 2 | 32 step buttons + note display |
| Arp | 3 | 4 | ~34 | 2 | 32 step buttons |
| Effects | 6+1 | 18 | 1 | 0 | Effect chain strip |
| Macros | 1 | 8 | 8 | 0 | Macro assignment list |
| Granular | 3 | 10 | 1 | 1 | None |
| Layers | 1 | 40 | ~40 | 16 | 16 level meters |

**Total UI components:** 350+ interactive elements

---

## 6. File Structure for Implementation

After Figma design is complete, implement in this order:

1. `Source/CyberpunkTheme.h/.cpp` — **ALREADY DONE** (palette + draw methods)
2. Update all panel `paint()` methods to match new Figma design
3. Update `NeuKnob.cpp` to match new knob design spec
4. Update `Source/PluginEditor.cpp` header layout
5. Rebuild and test VST3 + AU

Current state: Theme class exists with correct palette. Panels need visual redesign to match Serum 2 / Avenger 2 quality.

---

**End of Design Brief**
