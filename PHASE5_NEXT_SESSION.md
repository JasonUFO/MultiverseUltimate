# Phase 5 — Next Session Start Guide

## Status at Session Start
- **Phases 0–4 complete**, VST3 + AU building, pushed to GitHub (`44bb766`)
- **Phase 5 is next**: Modulation Upgrades

---

## What Phase 5 Covers (`AI_GAP_FILL_PLAN.md`)

```
5.1  Unlimited LFOs/envelopes
     - Replace fixed 4 LFOs with dynamic std::vector<LFOState>
     - Add UI to add/remove LFOs in ModulationMatrixPanel

5.2  Drawable custom LFO shapes
     - LFO shape editor (reuse WavetableEditor logic)
     - Added to ModulationMatrixPanel

5.3  DAW-synced LFO rates
     - Sync LFO phase/rate to DAW transport (extend existing DAW sync)

5.4  Unlimited envelopes
     - Dynamic envelope support per modulation target
```

---

## Session Startup Checklist

1. Read `AI_RULES.md` — no repo scans, minimal changes
2. Read `AI_HANDOFF.md` — architecture facts, conventions, all completed phases
3. Read `AI_STATE.md` — last completed work, what was shipped
4. Read `AI_GAP_FILL_PLAN.md` — Phase 5 spec
5. Read relevant source files before coding (ModulationMatrix.h/.cpp, ModulationMatrixPanel.h/.cpp)

---

## Key Files for Phase 5

| File | Purpose |
|------|---------|
| `Source/Synth/ModulationMatrix.h/.cpp` | LFO engine — currently 4 fixed LFOs |
| `Source/Modulation/ModulationMatrixPanel.h/.cpp` | UI for mod routing |
| `Source/PluginProcessor.cpp` | `advanceLFOs()` call, `createParameterLayout()` |
| `Source/Synth/WavetableEditor.h/.cpp` | Reference for drawable shape editor |

---

## Architecture Notes for Phase 5

### Current LFO system (to be extended)
- `ModulationMatrix` owns 4 fixed `LFOState` structs
- `advanceLFOs(numSamples)` called in `processBlock`
- LFO rates are `baseLfoRates[4]` floats in PluginProcessor
- Mod sources: `ModSourceType::LFO1..LFO4` (enum slots 0–3)

### Design question to ask user before coding
1. **How many LFOs max?** (8? 16? Truly unlimited vector?)
2. **Drawable shapes**: full WavetableEditor reuse, or simpler point-drag?
3. **DAW sync rates**: musical values (1/4, 1/8, 1/16…) or free Hz?
4. **Envelopes (5.4)**: ADSR per mod slot, or separate envelope objects?

---

## Competitive Context
Goal: match/surpass Serum 2, Nexus 5, Avenger 2, Diva, Zebra 3.
- Serum 2: unlimited LFOs with drawable shapes, tempo sync
- Nexus 5: 4 LFOs + 4 envelopes per layer
- Avenger 2: unlimited LFOs, drawable, MIDI-triggerable
