# Figma Sign-Off Checklist — Cyberpunk UI

**Date:** 2026-05-03  
**Status:** Awaiting visual approval

---

## SVGs to Review

Open these files in a browser or import into Figma:

### Components (3 files)
- [ ] `Figmacomponents/neumorphic_card.svg` — Card with dual shadows + neon border
- [ ] `Figmacomponents/neuknob.svg` — NeuKnob with 270° arc, neon glow, value pill
- [ ] `Figmacomponents/toggle_pill.svg` — LED toggle in ON state

### Panel Mockups (10 files)
- [ ] `Figmacomponents/synth_panel.svg` — SynthPanel (OSC, UNISON, FILTER, ENV, VOICE MODE)
- [ ] `Figmacomponents/drums_panel.svg` — Drums (Transport, Swing/Quant, Patterns, 8-track grid)
- [ ] `Figmacomponents/modulation_panel.svg` — Modulation Matrix (header + dynamic rows)
- [ ] `Figmacomponents/sampler_panel.svg` — Sampler (Drop zone, Zone list, Controls)
- [ ] `Figmacomponents/sequencer_panel.svg` — Sequencer (Transport, Patterns, 16-step grid)
- [ ] `Figmacomponents/proseq_panel.svg` — Pro Seq (Transport, Lanes, 4-lane grid, Editor)
- [ ] `Figmacomponents/arp_panel.svg` — Arp (Controls, 2×16 step grid, Editor)
- [ ] `Figmacomponents/effects_panel.svg` — Effects (Chain strip + 6 effects)
- [ ] `Figmacomponents/granular_panel.svg` — Granular (Source, Grain controls, ADSR)
- [ ] `Figmacomponents/macros_panel.svg` — Macros (2×4 grid, 8 knobs)

### Reference Docs
- [ ] `AI_CYBERPUNK_PLAN.md` — Full spec (colors, typography, components, layouts)
- [ ] `FIGMA_BUILD_GUIDE.md` — Exact values for Figma (colors, shadows, glows)

---

## Sign-Off Criteria

- [ ] **Color palette** — `bgVoid`/`bgRaised`/`neonCyan`/`neonPink`/`neonPurple` look correct
- [ ] **Neumorphic shadows** — Dual shadows (dark + light) visible on cards/knobs
- [ ] **Neon glow effects** — `neonCyan` arcs/glows have correct opacity and blur
- [ ] **NeuKnob design** — 270° arc, tip dot, value pill, center cap all present
- [ ] **Panel layouts** — Cards positioned correctly per `AI_CYBERPUNK_PLAN.md` §4
- [ ] **Typography** — Correct sizes (9px-12px) and colors (`textPrimary`/`textSecondary`)

---

## After Sign-Off

1. **Approve** → Proceed to Step 3 (C++ implementation)
2. **Request changes** → Update SVGs → Re-submit for sign-off
3. **Major redesign** → Go back to Figma (or create new SVGs)

---

**Sign-off:** _____________________ (Date: _________)

**Next step after sign-off:**  
Implement `CyberpunkTheme.h/.cpp` + update all 10 panel `paint()` methods to match SVGs exactly.
