# Phase 9 — Next Session Guide (Performance View)

## Session Goal
Implement **Performance View** — a full-screen overlay panel showing 8 large macro knobs for live performance use.

## What Was Just Shipped (this session)
- ✅ Drawable LFO Shapes (`LFOShape::Custom`, `LFOShapeEditor`, DRAW button)
- ✅ Chord/Strum Mode (12 shapes, 0–200ms strum delay, `PendingNote[64]` queue)

## Next Feature: Performance View

### Goal
A full-screen overlay accessible from the main header. Shows 8 large macro knobs + an XY pad. No new DSP — purely UI wired to existing `macro1`–`macro8` APVTS params.

### Architecture (proposed)
- New `PerformancePanel` component (`Source/Performance/PerformancePanel.h/.cpp`)
- "PERF" ToggleButton in PluginEditor header — shows/hides the panel as a full-area overlay on top of the tab content
- 8 large NeuKnobs (120px diameter) wired to `macro1`–`macro8` APVTS params
- Macro names shown below each knob (pulled from `MacroManager::getMacroName(i)`)
- Optional XY pad (bottom strip, 300×120px): X axis → macro1 param, Y axis → macro2 param; neumorphic inset rect with crosshair cursor
- Dark Forge palette: `bgBase` background, neumorphic cards per macro group, accent glow on active knob
- Panel lives inside `PluginEditor` as a direct child (on top of `tabbedComponent`), sized to match the tab area

### Key Files to Modify
1. `Source/PluginEditor.h` — add `PerformancePanel performancePanel` member + `TextButton perfButton`
2. `Source/PluginEditor.cpp` — add PERF toggle in header, show/hide overlay in `resized()`
3. `Source/Performance/PerformancePanel.h/.cpp` — new component (add to Multiverse.jucer)
4. `Multiverse.jucer` — add new source files, run `--resave`

### XY Pad Detail
- Mouse-drag sets X (left–right → 0–1) and Y (bottom–top → 0–1)
- Writes directly to `apvts.getParameter("macro1")->setValueNotifyingHost(x)` etc.
- Draws a neumorphic inset rect (dark, 4px corner) with crosshair (accentBlue lines)
- Label "XY → MACRO 1 / 2" in top-left of pad

### APVTS Params Needed
None — wired directly to existing `macro1`–`macro8` params.

### State Persistence
None needed — the panel is purely a view into existing params.

### Layout Sketch (1200×700 tab area)
```
┌─────────────────────────────────────────────────────────┐
│  PERFORMANCE VIEW                              [× CLOSE] │
├──────────┬──────────┬──────────┬──────────────────────── │
│ MACRO 1  │ MACRO 2  │ MACRO 3  │ MACRO 4                 │
│  (knob)  │  (knob)  │  (knob)  │  (knob)                 │
│  name    │  name    │  name    │  name                   │
├──────────┴──────────┴──────────┴────────────────────────┤
│ MACRO 5  │ MACRO 6  │ MACRO 7  │ MACRO 8                 │
│  (knob)  │  (knob)  │  (knob)  │  (knob)                 │
│  name    │  name    │  name    │  name                   │
├─────────────────────────────────────────────────────────┤
│              XY PAD (X→MACRO1, Y→MACRO2)                │
└─────────────────────────────────────────────────────────┘
```

## After Performance View
- Programmatic preset generation (100+ factory presets across Bass/Lead/Pad/Drums/FX)

## Build Command
```bash
xcodebuild -project Builds/MacOSX/MultiverseUltimate.xcodeproj \
  -scheme "MultiverseUltimate - VST3" -configuration Release build 2>&1 | tail -5
```

Install path: `/Users/jason/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3`
