# MultiverseUltimate — AI RULES

## Current Phase
**Avenger-Style Redesign** — Phase 1 (knob redesign, EnvelopeDisplay, FilterDisplay, layout restructure) shipped. Next: Phase 2 — Tab-Based Oscillator System. All gap-fill phases (1–9) and UI overhaul (1–5) also shipped.

---

## Core Behaviour

1. Do NOT scan the entire repository unless explicitly instructed.
2. Only inspect files relevant to the task.
3. Do NOT rewrite large systems unnecessarily.
4. Do NOT introduce new architecture without justification.
5. Keep changes minimal and targeted.

---

## Coding Rules

- Minimal changes — don't refactor beyond the task
- Preserve existing structure and naming
- Keep DSP real-time safe (no alloc, no locks in processBlock)
- New effects follow the `Effect` base class pattern
- New stereo effects: use `effect[2]` (L/R instances)
- After editing `.jucer`, run Projucer `--resave` before building
- After **every** Projucer `--resave`, run `Scripts/fix_instrument_classification.sh` — Projucer resets plugin classification to "effect" defaults; the script enforces Instrument/Synth classification. A build phase runs it automatically, but always verify after resave.
- `Builds/` directory is gitignored — Xcode project changes (build phases, etc.) don't persist in git. The fix script compensates for this.

---

## Task Execution

1. Read relevant files first
2. Identify exact change points
3. Modify only those files
4. Build and verify — fix errors before reporting done

---

## Performance Rules

- No heap allocations in audio thread
- No locks in `processBlock`
- One-time setup in `prepare()`, not per-sample
- Coefficient updates per-block (setters called from processBlock) are acceptable

---

## Priority

1. Audio output works and is stable
2. Signal flow correct
3. Modulation applies correctly
4. UI controls are wired to DSP
5. State persistence (presets save/load correctly)
6. Feature quality and polish
