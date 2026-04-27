# MultiverseUltimate — AI HANDOFF

## Project Identity
- **Name:** MultiverseUltimate
- **Type:** Hybrid Synthesizer (FM + Subtractive + Sampler + Drum Sequencer)
- **Format:** VST3 / AU (macOS)
- **Framework:** JUCE 8
- **Language:** C++17
- **Build:** Projucer + Xcode → `Builds/MacOSX/MultiverseUltimate.xcodeproj`
- **Install:** `~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3` (~37 MB, arm64+x86_64)
- **Status:** Builds successfully, loads in DAW, fully functional

---

## Core Architecture

| Engine | Description |
|--------|-------------|
| SynthEngine | 16-voice classic / 8-voice FM |
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

- SynthEngine (Classic + FM), DrumSequencer, SamplerEngine — all produce audio ✅
- All 6 effects with full parameter control and MIDI Learn ✅
- Effect chain reordering — drag-to-reorder strip, order persists in presets ✅
- ModulationMatrix — LFO → pitch/cutoff/volume/effects ✅
- Melodic Sequencer, DAW transport sync ✅
- Full state persistence (XML) including effect chain order ✅
- MIDI Learn on all effect panel knobs ✅
- Undo/Redo (Cmd+Z / Cmd+Shift+Z) ✅
- Filter oversampling (Off/2x/4x/Auto) ✅
- Preset system (XML) ✅

## What Is Broken / Unconnected
- None

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

---

## Completed Phases

| Phase | What |
|-------|------|
| Integration | All engines wired, signal flow complete |
| 3.1 | Filter oversampling (Off/2x/4x/Auto) |
| 3.2 | Reverb pre-delay, LF damp, width, freeze |
| 3.3 | Chorus, Distortion, EQ, Compressor + drag-to-reorder chain |

## Next Steps
- Adopt MidiLearnSlider in remaining panels (SynthPanel, ModulationMatrixPanel, SamplerPanel)
- Final regression testing across DAW environments
