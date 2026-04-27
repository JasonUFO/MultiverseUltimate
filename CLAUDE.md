# Multiverse Ultimate — AI Handoff Context

**Company:** Multiphase Audio | **Plugin:** MultiverseUltimate | **Format:** VST3 / AU (macOS)  
**Build system:** JUCE 8.x + Projucer + Xcode  
**Full spec:** `MULTIVERSE_ULTIMATE_SPEC.md` (read this for deep detail on any subsystem)

---

## Project State Summary

All subsystems are integrated and functional. Phase 3.3 complete. The VST3 builds and installs successfully:

```
~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3  (~37 MB, Universal arm64+x86_64)
```

---

## What Works

- **SynthEngine** (Classic + FM modes): 16-voice classic / 8-voice FM ✅
- **DrumSequencer**: sequences and plays back loaded samples ✅
- **SamplerEngine**: zone-based sample playback with full MIDI routing ✅
- **Effects** (full chain): Chorus, Distortion, EQ, Compressor, Delay, Reverb ✅
- **Effect chain ordering**: drag-to-reorder strip; order persists in presets ✅
- **ModulationMatrix**: LFO → pitch/cutoff/volume/effects ✅
- **Melodic Sequencer**: generates MIDI into synth/sampler ✅
- **Build system**: `Multiverse.jucer` → Projucer → Xcode builds cleanly ✅
- **UI**: Full tabbed interface (Synth, Drums, Modulation, Sampler, Sequencer, Pro Seq, Arp, Effects) ✅
- **State persistence**: XML save/restore including effect chain order ✅
- **MIDI handling**: note on/off, pitch bend, sustain CC64, sostenuto CC66, mod wheel CC1→filter, all-notes-off CC123 ✅
- **MIDI Learn**: CC/pitch-bend/channel-pressure → any APVTS param, orange "L" badge, persists in presets ✅
- **Undo/Redo**: Cmd+Z / Cmd+Shift+Z; clears on preset load ✅
- **Filter oversampling**: Off/2x/4x/Auto (Classic mode) ✅
- **Reverb quality**: pre-delay, LF damp, stereo width, freeze ✅
- **DAW transport sync**: BPM, playing, PPQ connected to sequencers ✅
- **Preset system**: XML-based preset loading/saving ✅

---

## What Is Broken / Unconnected

None.

---

## Signal Flow

```
MIDI → Sequencer → SynthEngine  ─┐
MIDI → DrumSequencer            ─┼─► Effect Chain ──► Output
MIDI → SamplerEngine            ─┘
```

**Default effect chain order** (drag-to-reorder in UI):
`Chorus → Distortion → EQ → Compressor → Delay → Reverb`

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
| Build output | `Builds/MacOSX/MultiverseUltimate.xcodeproj` |

---

## Conventions & Gotchas

- Include JUCE headers as `<JuceHeader.h>` (angle brackets) — do not revert to quoted path.
- The sampler class is `MvSamplerVoice` — renamed to avoid conflict with `juce::SamplerVoice`.
- Font: use `juce::Font(size, juce::Font::bold)` — `juce::Font::Typeface::bold` does not exist in JUCE 8.
- `ModulationMatrix::getActiveConnectionsForTarget()` returns by value (thread-safety fix — do not change).
- `DrumSequencer::getTrackLevel()` uses atomic peak tracking with exponential decay.
- Effect chain: packed as 6 nibbles in `std::atomic<uint32_t> effectChainOrder`; use `getChainSlot(pos)` / `swapChainSlots(a,b)`.
- New effects (Chorus/Distortion/EQ/Compressor) have stereo instances `effect[2]`; Delay is a single shared instance.
- After modifying `.jucer`, always run Projucer `--resave` to regenerate the Xcode project before building.

---

## Next Steps
- Adopt MidiLearnSlider in remaining panels (SynthPanel, ModulationMatrixPanel, SamplerPanel)
- Final regression testing across DAW environments
