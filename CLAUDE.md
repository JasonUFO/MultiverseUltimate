# Multiverse Ultimate — AI Handoff Context

**Company:** Multiphase Audio | **Plugin:** MultiverseUltimate | **Format:** VST3 / AU (macOS)  
**Build system:** JUCE 8.x + Projucer + Xcode  
**Full spec:** `MULTIVERSE_ULTIMATE_SPEC.md` (read this for deep detail on any subsystem)

---

## Project State Summary

A 14-bug audit has been completed and all bugs are fixed. The VST3 builds and installs successfully:

```
~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3  (12 MB, Universal arm64+x86_64)
```

The plugin **loads in a DAW but is mostly silent and non-functional** beyond basic synth notes. Most subsystems are unconnected. See below.

---

## What Works

- **SynthEngine** (Classic + FM modes): 16-voice classic / 8-voice FM — produces audio ✅
- **DrumSequencer**: sequences and plays back loaded samples ✅
- **Build system**: `Multiverse.jucer` → Projucer → Xcode builds cleanly ✅
- **ADSR envelope**, **SVF filter**, **oscillator** DSP: all fixed and correct ✅

---

## What Is Broken / Unconnected (Priority Order)

### 🔴 CRITICAL — Silent Audio Paths

1. **Delay + Reverb not called in `processBlock`**  
   `DelayEffect` and `ReverbEffect` are instantiated in `PluginProcessor` but `delay.process()` / `reverb.process()` are never invoked. ~30 min fix.

2. **SamplerEngine not wired into `processBlock`**  
   Sampler MIDI routing goes to `synthEngine` only. `SamplerEngine::process()` is never called and its output is never mixed. Files: `PluginProcessor.cpp`, `Sampler/SamplerEngine.h/.cpp`.

3. **ModulationMatrix lives on the UI thread**  
   It's instantiated in `PluginEditor`, not `PluginProcessor`. Modulation values never reach oscillators/filters/envelopes. Needs to move to `PluginProcessor` and be shared with the editor via a reference or `std::shared_ptr`.

4. **Melodic Sequencer not instantiated in `PluginProcessor`**  
   `Sequencer` exists only as a UI-side object and generates no MIDI output that drives the synth.

### 🔴 CRITICAL — State Is Lost on Every Reload

5. **`getStateInformation` / `setStateInformation` saves only `masterVolume`**  
   Everything else (drum patterns, synth params, effects, sequencer patterns, preset index) is lost when the DAW session is closed. Target: migrate to `juce::ValueTree` or `AudioProcessorValueTreeState`.

### 🟠 HIGH — UI Is Incomplete

6. **Only `DrumSequencerPanel` and `ModulationMatrixPanel` shown**  
   `SamplerPanel`, `SequencerPanel`, and all synth param controls exist in source but are never added to the editor.

7. **No tab/navigation bar** — 5+ panels need a tab bar or sidebar to be navigable.

8. **Debug paint call in `PluginEditor::paint()`**  
   `g.drawText(JucePlugin_Name, ...)` renders over the top 150px of the UI. Remove it.

### 🟡 MEDIUM — Synth Parameters Not Wired

9. `SynthEngine::setEnvelopeParams()`, `setFilterParams()`, `setWaveform()`, `setFMAlgorithm()`, `setFMOperatorParams()` all exist but nothing calls them from any UI control.

10. **Pitch bend / all-notes-off / CC 64 (sustain)** not handled in `processBlock`.

### 🟡 MEDIUM — Modulation Matrix Not Functional

11. LFO phases (`lfo1Phase`–`lfo4Phase`) are never advanced — LFOs produce no output.  
12. `ModulationMatrix::getModulationValue()` exists but nothing reads it at audio rate.

### 🟢 LOW — Code Quality

13. `PluginProcessor.h` has `#if (MSVC)` (should be `#ifdef _MSC_VER`) guarding an Intel IPP include.
14. `fprintf(stderr, ...)` debug print in `PluginProcessor` constructor — remove before release.
15. `SamplerEngine::process()` never acquires `zoneLock` despite declaring it — lock is dead code.
16. No `AudioProcessorValueTreeState` — DAW automation and undo/redo are impossible without it.

---

## Source File Map

```
source/
├── PluginProcessor.h/.cpp           Top-level audio + MIDI routing
├── PluginEditor.h/.cpp              Top-level GUI component

├── Synth/
│   ├── SynthEngine.h/.cpp           Polyphonic voice manager (classic + FM)
│   ├── Voice.h/.cpp                 Classic subtractive voice
│   ├── Oscillator.h/.cpp            Wavetable/analogue oscillator
│   ├── Envelope.h/.cpp              ADSR (sample-rate corrected)
│   ├── Filter.h/.cpp                SVF filter (juce::dsp::StateVariableTPTFilter)
│   ├── FMVoice.h/.cpp               4-operator FM voice
│   ├── FMOperator.h/.cpp            Single FM operator with ADSR + feedback
│   ├── FMAlgorithm.h                8 hard-coded 4-op FM routing algorithms
│   ├── ModulationMatrix.h/.cpp      Mod routing — 11 sources × 15 targets, max 32 connections
│   └── ModulationMatrixPanel.h/.cpp Mod matrix GUI

├── DrumSequencer/
│   ├── DrumSequencer.h/.cpp         8 tracks × 16 steps × 8 patterns, sample playback
│   └── DrumSequencerPanel.h/.cpp    Drum sequencer GUI

├── Sampler/
│   ├── SamplerEngine.h/.cpp         Zone-based sample playback engine
│   ├── SamplerVoice.h/.cpp          Per-voice playback (ADSR, loop modes, crossfade)
│   ├── SamplerZone.h                Zone data struct
│   └── SamplerPanel.h/.cpp          Sampler GUI (not yet shown in editor)

├── Sequencer/
│   ├── Sequencer.h/.cpp             Melodic step sequencer + arpeggiator, MIDI output
│   └── SequencerPanel.h/.cpp        Step grid GUI (not yet shown in editor)

├── Effects/
│   ├── Effect.h                     Abstract base (prepare/process/reset)
│   ├── Delay.h/.cpp                 Stereo delay (juce::dsp::DelayLine, feedback + mix)
│   └── Reverb.h/.cpp                Reverb (juce::Reverb, room/damping/wet/dry)

└── Presets/
    ├── Preset.h/.cpp                Preset data container
    └── PresetManager.h/.cpp         Load/save presets (XML) — instantiated but never used
```

---

## DSP Signal Path (Current vs Intended)

**Current (broken):**
```
MIDI → SynthEngine (Classic / FM)  →  Output   ✅
MIDI → DrumSequencer               →  Output   ✅
       SamplerEngine                            ❌ (not called)
       DelayEffect                              ❌ (not called)
       ReverbEffect                             ❌ (not called)
       ModulationMatrix                         ❌ (UI thread only)
```

**Intended:**
```
MIDI → Sequencer → SynthEngine → Delay → Reverb → Output
MIDI → DrumSequencer → Output
MIDI → SamplerEngine → Output
ModulationMatrix → SynthEngine params (pitch, cutoff, volume…)
```

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

- Include JUCE headers as `<JuceHeader.h>` (angle brackets), not `"JuceLibraryCode/JuceHeader.h"` (quotes) — this was a fixed bug, don't revert it.
- The sampler class is `MvSamplerVoice` (not `SamplerVoice`) — renamed to avoid conflict with `juce::SamplerVoice`.
- Font construction must use `juce::Font(size, juce::Font::bold)` — `juce::Font::Typeface::bold` does not exist in JUCE 8.
- `ModulationMatrix::getActiveConnectionsForTarget()` returns by value (intentionally) — was a thread-safety fix, do not change to pointer/reference return.
- `DrumSequencer::getTrackLevel()` uses atomic peak tracking with exponential decay — check this before any threading changes.

---

## Recommended Next Task

**Start here → Wire delay and reverb into `processBlock` in `PluginProcessor.cpp`.**  
They are instantiated, `prepare()` is called, but `process()` is never invoked. Should take ~30 minutes and immediately unblocks effects testing.
