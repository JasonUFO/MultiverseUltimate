# Multiverse Ultimate — AI Handoff Context

**Company:** Multiphase Audio | **Plugin:** MultiverseUltimate | **Format:** VST3 / AU (macOS)  
**Build system:** JUCE 8.x + Projucer + Xcode  
**Full spec:** `MULTIVERSE_ULTIMATE_SPEC.md` (read this for deep detail on any subsystem)

---

## Project State Summary

All subsystems are now integrated and functional. The VST3 builds and installs successfully:

```
~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3  (12 MB, Universal arm64+x86_64)
```

The plugin loads in a DAW and is fully functional with all subsystems connected and operational.

---

## What Works

- **SynthEngine** (Classic + FM modes): 16-voice classic / 8-voice FM — produces audio ✅
- **DrumSequencer**: sequences and plays back loaded samples ✅
- **SamplerEngine**: zone-based sample playback with full MIDI routing ✅
- **DelayEffect**: Stereo delay processed in audio path with parameter control ✅
- **ReverbEffect**: Reverb processed in audio path with parameter control ✅
- **ModulationMatrix**: Fully functional with LFO phase advancement and audio-rate modulation application ✅
- **Melodic Sequencer**: Generates MIDI output that drives synth and sampler engines ✅
- **Build system**: `Multiverse.jucer` → Projucer → Xcode builds cleanly ✅
- **ADSR envelope**, **SVF filter**, **oscillator** DSP: all fixed and correct ✅
- **UI**: Full tabbed interface with all panels (Synth, Drums, Modulation, Sampler, Sequencer, Pro Seq, Arp, Effects) ✅
- **State persistence**: Complete state save/restore via APVTS including all engine states ✅
- **MIDI handling**: note on/off, pitch bend, sustain (CC64), sostenuto (CC66), mod wheel (CC1 → filter), all-notes-off (CC123) ✅
- **DAW transport sync**: BPM, playing, PPQ connected to sequencers ✅
- **Preset system**: XML-based preset loading/saving functional ✅

---

## What Is Broken / Unconnected

None - all systems are integrated and functional.

---

## Current System State

### Integration Phase Complete
All previously disconnected systems are now properly wired:
- Audio routing: MIDI → Sequencer → SynthEngine → Delay → Reverb → Output
- Audio routing: MIDI → DrumSequencer → Output
- Audio routing: MIDI → SamplerEngine → Output
- Modulation: ModulationMatrix → SynthEngine/SamplerEngine parameters (pitch, cutoff, volume…)
- UI: All DSP parameters controllable via interface
- State: Full persistence across DAW sessions

### Key Technical Facts

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

### Conventions & Gotchas

- Include JUCE headers as `<JuceHeader.h>` (angle brackets), not `"JuceLibraryCode/JuceHeader.h"` (quotes) — this was a fixed bug, don't revert it.
- The sampler class is `MvSamplerVoice` (not `SamplerVoice`) — renamed to avoid conflict with `juce::SamplerVoice`.
- Font construction must use `juce::Font(size, juce::Font::bold)` — `juce::Font::Typeface::bold` does not exist in JUCE 8.
- `ModulationMatrix::getActiveConnectionsForTarget()` returns by value (intentionally) — was a thread-safety fix, do not change to pointer/reference return.
- `DrumSequencer::getTrackLevel()` uses atomic peak tracking with exponential decay — check this before any threading changes.

### Next Steps
- Perform final regression testing across DAW environments
- Verify all automation parameters are exposed and functional
- Validate preset interchange with other formats if needed
- Optimize any remaining performance bottlenecks