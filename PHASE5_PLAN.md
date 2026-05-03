# Phase 5: Remaining UI Panels — Dark Forge Redesign

## Goal
Redesign all remaining UI panels to match the Dark Forge design system established in Phases 1-4.

## Reference Implementation
- **PresetBrowserPanel** — Neumorphic cards, search bar, category pills (Phase 4 complete)
- **MultiverseTheme** — `Source/MultiverseTheme.h/.cpp` with Dark Forge palette
- **NeuKnob** — `Source/NeuKnob.h/.cpp` for rotary sliders

## Panels to Update

### 1. EffectsPanel
**File:** `Source/Effects/EffectsPanel.h/.cpp`
- Convert to neumorphic section cards (Chorus, Distortion, EQ, Compressor sections)
- Use NeuKnob for all rotary sliders
- 2-column layout with neumorphic card backgrounds
- Add subtle dividers between effect sections

### 2. ModulationMatrixPanel
**File:** `Source/Modulation/ModulationMatrixPanel.h/.cpp`
- Neumorphic card for each modulation route
- Source/target dropdowns styled with MultiverseTheme
- Amount sliders: use NeuKnob or styled linear sliders
- Add/Remove buttons with neumorphic styling

### 3. DrumPanel
**File:** `Source/Drums/DrumPanel.h/.cpp`
- Neumorphic card for each drum track
- Sample load buttons with Dark Forge styling
- Trigger pads (if applicable) with LED-style feedback

### 4. SamplerPanel
**File:** `Source/Sampler/SamplerPanel.h/.cpp`
- Zone display with neumorphic card background
- Sample load/info section with Dark Forge styling
- All sliders use NeuKnob

### 5. SequencerPanel
**File:** `Source/Sequencer/SequencerPanel.h/.cpp`
- Pattern grid with neumorphic card background
- Transport controls with Dark Forge styling
- Step indicators with LED-style feedback

### 6. ArpPanel
**File:** `Source/Arp/ArpPanel.h/.cpp`
- Pattern selector with neumorphic styling
- Rate/Range controls using NeuKnob
- Arp mode selector with Dark Forge combo box

### 7. ProSeqPanel
**File:** `Source/ProSeq/ProSeqPanel.h/.cpp`
- Advanced sequencer UI with neumorphic cards
- Track lanes with Dark Forge styling
- Transport/pattern controls

## Design Patterns to Apply

### Neumorphic Cards
```cpp
// In paint():
auto cardBounds = getLocalBounds().toFloat().reduced(4, 2);
drawNeumorphicRect(g, cardBounds, 8.0f, 3.0f);
g.setColour(bgRaised);
g.fillRoundedRectangle(cardBounds, 8.0f);
```

### NeuKnob Usage
```cpp
// Replace:
juce::Slider someSlider;
// With:
NeuKnob someSlider;
// Call init() after adding to panel
```

### Section Headers
```cpp
g.setColour(textLabel);
g.setFont(juce::Font(11.0f, juce::Font::plain));
g.drawText("SECTION NAME", bounds, juce::Justification::centredLeft);
```

## Color Palette (from MultiverseTheme)
- `bgBase` — #171720 (panel background)
- `bgRaised` — #1e1e2c (cards, raised elements)
- `bgDeep` — #111119 (inset elements)
- `accentBlue` — #5b8def (active states, highlights)
- `textPrimary` — #e4e8f0 (main text)
- `textSecondary` — #7a8499 (secondary text)
- `textMuted` — #3d4358 (muted text)

## Success Criteria
- [ ] All panels use consistent Dark Forge styling
- [ ] All rotary sliders use NeuKnob
- [ ] Neumorphic cards for logical sections
- [ ] Build succeeds with no warnings
- [ ] Visual consistency across all 8 panels
- [ ] Commit and push to GitHub

## Next Session Start
1. Read `AI_STATE.md` (Phase 5 section)
2. Read `AI_HANDOFF.md` (updated conventions)
3. Read `PHASE5_PLAN.md` (this file)
4. Pick one panel, apply redesign, build, verify
5. Repeat for all panels
6. Commit and push when complete
