# MultiverseUltimate — AI HANDOFF

## Project Identity
- **Name:** MultiverseUltimate
- **Type:** Hybrid Synthesizer (FM + Subtractive + Sampler + Drum Sequencer)
- **Format:** VST3 / AU (macOS)
- **Framework:** JUCE 8
- **Language:** C++17
- **Build:** Projucer + Xcode → `Builds/MacOSX/MultiverseUltimate.xcodeproj`
- **Install:** `~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3` (arm64+x86_64)
- **Status:** Builds successfully, loads in DAW, fully functional

---

## Core Architecture

| Engine | Description |
|--------|-------------|
| SynthEngine | 16-voice classic (3 osc/voice: Classic or Wavetable) / 8-voice FM |
| DrumSequencer | Sample-based, 32 voices |
| SamplerEngine | Zone-based playback, 16 voices |
| Effects | Chorus → Distortion → EQ → Compressor → Delay → Reverb (reorderable chain) |
| ModulationMatrix | LFO + envelope routing to all DSP params |
| Sequencer / ProSequencer / Arpeggiator / PatternEngine | MIDI generation |

---

## Signal Flow

```
MIDI → Sequencer → SynthEngine  ─┐
MIDI → DrumSequencer            ─┼─► Effect Chain ──► Output
MIDI → SamplerEngine            ─┘
ModulationMatrix → all DSP parameters (pitch, cutoff, volume, …)
```

**Effect chain** (default order, drag-to-reorder in UI):
`Chorus → Distortion → EQ → Compressor → Delay → Reverb`
Reverb is always applied as a stereo block op; the chain correctly splits pre/post around it.

---

## What Works

- SynthEngine (16-voice classic with 3 osc/voice + FM 8-voice), DrumSequencer, SamplerEngine — all produce audio ✅
- 3 oscillators per voice: Classic (math-based) or Wavetable (2048-sample table, 4 waves, position scan) ✅
- Voice modes: Poly / Mono / Legato with portamento (per-sample semitone-space glide) ✅
- All 6 effects with full parameter control and MIDI Learn ✅
- Effect chain reordering — drag-to-reorder strip, order persists in presets ✅
- ModulationMatrix — LFO → pitch/cutoff/volume/effects ✅
- Melodic Sequencer, DAW transport sync ✅
- Full state persistence (XML) including 3 oscillators + effect chain order ✅
- MIDI Learn on all effect panel knobs ✅
- Undo/Redo (Cmd+Z / Cmd+Shift+Z) ✅
- Filter oversampling (Off/2x/4x/Auto) ✅
- Preset system (XML) with Factory/User banks ✅

## What Is Broken / Unconnected
- None

---

## Completed Phases

| Phase | What |
|-------|------|
| Integration | All engines wired, signal flow complete |
| 3.1 | Filter oversampling (Off/2x/4x/Auto) |
| 3.2 | Reverb pre-delay, LF damp, width, freeze |
| 3.3 | Chorus, Distortion, EQ, Compressor + drag-to-reorder chain |
| 4.1 | SynthPanel MIDI Learn |
| 4.2 | ModulationMatrixPanel MIDI Learn |
| 4.3 | SamplerPanel MIDI Learn + Tooltips across all panels |
| 5.1 | Preset Browser + Factory/User banks with category subfolders |
| Phase 3 | Wavetable file loading per oscillator strip |
| Phase A | Voice Modes (Poly/Mono/Legato) + Portamento + "Porta Always" |
| Phase B | Macro Controls (8 macros, DAW-automatable, nameable, right-click assign, preset-persistent) |

## Next Steps
All planned features complete. Candidate directions:
- Polish pass (visualizers, labels, color-coded tabs)
- More modulation sources (step seq mod, random/S&H, velocity, envelope follower)
- MPE support (per-note pitch/pressure/slide)
- Granular engine (new synthesis mode)
- Chord/strum mode

---

## Key Technical Facts

| Detail | Value |
|--------|-------|
| Polyphony (classic) | 16 voices |
| Polyphony (FM) | 8 voices |
| Polyphony (sampler) | 16 voices |
| Drum voices | 32 (4/track) |
| Manufacturer code | `MpAu` |
| Plugin code | `MvUl` |
| C++ standard | C++17 |
| JUCE version | 8.x |
| macOS target | 10.13+ |

---

## Conventions & Gotchas

- Include JUCE as `<JuceHeader.h>` (angle brackets) — do not revert to quoted path.
- Sampler class is `MvSamplerVoice` — renamed to avoid conflict with `juce::SamplerVoice`.
- Font: use `juce::Font(size, juce::Font::bold)` — `juce::Font::Typeface::bold` does not exist in JUCE 8.
- `ModulationMatrix::getActiveConnectionsForTarget()` returns by value (thread-safety fix — do not change to ref/pointer).
- Effect chain order is packed as 6 nibbles in `std::atomic<uint32_t> effectChainOrder` — `getChainSlot(pos)` / `swapChainSlots(a,b)` are the only API.
- New effects (Chorus/Distortion/EQ/Compressor) have stereo L/R instances (`chorus[2]`, etc.); Delay is shared mono.
- `EQEffect::updateCoeffs()` uses biquad math per setter call (once per block from processBlock) — no heap allocation.
- Voice mode glide runs **per-sample** in `processBuffer` using `std::exp2((semitone - 69) / 12)` — only active in Mono/Legato, skipped entirely in Poly.
- Mono note stack is `int monoNoteStack[16]` + `int monoNoteCount` in SynthEngine (no heap alloc). Stack is cleared in `allNotesOff()`.
- `Voice::setFrequencyDirect(hz)` sets `baseFrequency` and calls `updateOscillatorFrequencies()` without touching the envelope — used by the glide loop.
- `Voice::setNoteLegato(note)` updates `midiNote` + `baseFrequency` only — no envelope retrigger. Used in Legato mode on gate-open note changes.
- APVTS params added by Phase A: `voiceMode` (Choice 0–2), `portamento` (Float 0–2s), `portaAlways` (Bool).
