# Multiverse Ultimate — Project Status & Technical Specification

**Company:** Multiphase Audio  
**Format:** VST3 / AU (macOS)  
**Build system:** JUCE 8.x + Projucer + Xcode  
**Last updated:** 2026-04-25

---

## 1. What Has Been Accomplished

### 1.1 Full Codebase Audit & Bug Fixes (14 bugs fixed)

| # | File | Bug | Fix Applied |
|---|------|-----|-------------|
| 1 | `Synth/Envelope.h/.cpp` | ADSR used raw time values (0.01, 0.1, 0.3 s) as per-sample increments — produced inaudible envelope | Added `setSampleRate()`, `recomputeIncrements()`, and pre-computed `attackInc`, `decayInc`, `releaseInc` members |
| 2 | `Synth/Voice.cpp` | `setSampleRate()` called oscillator and filter but never called `envelope.setSampleRate()` | Added `envelope.setSampleRate(sr)` call |
| 3 | `Synth/Filter.cpp` | `setSampleRate()` never called `filter.prepare(spec)` — `juce::dsp::StateVariableTPTFilter` would crash on first `processSample()` | Added `juce::dsp::ProcessSpec` construction and `filter.prepare(spec)` call |
| 4 | `PluginProcessor.cpp` | `synthEngine.process()` was called once per audio block, not once per sample — produced single-sample output at block rate | Moved `process()` inside the per-sample loop |
| 5 | `Effects/Delay.cpp` | Constructor called `delayLine.setMaximumDelayInSamples()` before `sampleRate` was known — always set max delay to 0 | Moved max-delay calculation into `prepare()` after `sampleRate` is assigned |
| 6 | `Synth/ModulationMatrix.h/.cpp` | `getActiveConnectionsForTarget()` returned `const std::vector<ModConnection>*` to a static local — thread-unsafe and UB on re-entry | Changed to return by value (`std::vector<ModConnection>`) |
| 7 | `DrumSequencer/DrumSequencer.cpp` | `getTrackLevel()` always returned 0 — peak level tracking was never updated | Added peak tracking and slow exponential decay per sample in `process()` |
| 8 | `DrumSequencer/DrumSequencerPanel.cpp` | `resized()` removed `trackRowHeight` from `area` but never advanced `stepArea` — all 8 track step rows were stacked at y=0 | Added `stepArea.removeFromTop(trackRowHeight)` per track |
| 9 | `Synth/ModulationMatrixPanel.cpp` | `setInterceptsMouseClicks(false, true)` on `SourceComponent` — drag initiation impossible | Changed to `(true, true)` |
| 10 | `Synth/ModulationMatrixPanel.cpp` | `onDragEnded` callback fired in `mouseDrag` instead of `mouseUp` — callback triggered on every drag pixel | Moved to `mouseUp` |
| 11 | `Sequencer/SequencerPanel.cpp` | Used `juce::Font::Typeface::bold` (non-existent in JUCE 8) — compile error | Changed to `juce::Font(size, juce::Font::bold)` (2 instances) |
| 12 | `Sampler/SamplerPanel.cpp` | Same `juce::Font::Typeface::bold` error | Fixed to `juce::Font(13.0f, juce::Font::bold)` |
| 13 | `DrumSequencer/DrumSequencerPanel.cpp` | Same `juce::Font::Typeface::bold` error | Fixed to `juce::Font(12.0f, juce::Font::bold)` |
| 14 | `Presets/Preset.cpp` | No negative-index guard on `setParameter()` — could trigger `resize()` on negative index → UB | Added `if (index < 0) return;` guard |

---

### 1.2 Build System Fixed & VST3 Built and Installed

**Root cause:** `Multiverse.jucer` was severely malformed — it was missing `juce_audio_plugin_client` (the module that provides VST3 wrapper code), had wrong FILE element attribute names (`path=` instead of `file=`), and had no `pluginCode`/`pluginManufacturerCode` fields.

**All issues resolved:**

| Issue | Resolution |
|-------|-----------|
| `juce_audio_plugin_client` module missing | Added to `MODULES` and `MODULEPATHS` in `.jucer` |
| FILE elements used `path=` attribute | Corrected to `file=` (Projucer's actual attribute name) |
| FILE `id` fields used filenames as IDs | Changed to short alphanumeric IDs (e.g. `Aa0001`) |
| FILE `name` fields were empty | Set to actual filenames |
| No `pluginCode` / `pluginManufacturerCode` | Added `pluginCode="MvUl"` and `pluginManufacturerCode="MpAu"` |
| VST2/VST3 parameter ID warning (compile error) | Added `JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1` to `JUCEOPTIONS` (correct for new plugin with no released VST2) |
| Source files used `"JuceLibraryCode/JuceHeader.h"` | Changed to `<JuceHeader.h>` throughout all headers |
| `SamplerVoice` name conflict with `juce::SamplerVoice` | Renamed project class to `MvSamplerVoice` |
| `ModulationMatrixPanel.h` missing `#include "ModulationMatrix.h"` | Include added |
| `FMOperator.h` missing `#include <JuceHeader.h>` | Include added |
| `PluginEditor.cpp` used CMake macros (`PRODUCT_NAME_WITHOUT_VERSION`, `VERSION`, `CMAKE_BUILD_TYPE`) | Replaced with `JucePlugin_Name` |

**Result:**  
`~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3` — 12 MB universal binary (arm64 + x86_64), Release build.

---

## 2. What Is Still To Do

### 2.1 Critical — Audio Routing (Plugin is Silent for Some Paths)

- [ ] **Delay and Reverb are instantiated in `PluginProcessor` but never called in `processBlock`.** `delay.process()` and `reverb.process()` are never invoked. The effects exist but produce no audio.
- [ ] **`SamplerEngine` is not wired into `processBlock` at all.** Sampler MIDI note-on/off goes to `synthEngine` only. Sampler engine produces no audio output.
- [ ] **`Sequencer` (melodic step sequencer) is not instantiated in `PluginProcessor`** and generates no MIDI output that drives the synth. It exists as a standalone object in the UI panel only.
- [ ] **`ModulationMatrix` is instantiated in `PluginEditor`, not `PluginProcessor`** — it lives on the UI thread and is disconnected from the audio engine. Modulation values never reach oscillators, filters, or envelopes.

### 2.2 UI — Most Panels Are Not Shown

- [ ] **`PluginEditor` only shows `DrumSequencerPanel` and `ModulationMatrixPanel`**. The following panels exist in source but are never added to the editor:
  - `SamplerPanel`
  - `SequencerPanel` (melodic sequencer / arpeggiator)
  - Any synth parameter UI (ADSR sliders, filter, oscillator, FM operator controls)
- [ ] **No tab/navigation system** — with 5+ panels, the UI needs a tab bar or sidebar.
- [ ] **`PluginEditor::paint()` draws a text label over the UI** — leftover debug text `g.drawText(JucePlugin_Name, ...)` renders at the top 150px over any content.
- [ ] **Layout in `PluginEditor::resized()`** splits area exactly in half between drum sequencer and modulation matrix — no consideration for other panels.

### 2.3 State Management

- [ ] **`getStateInformation` / `setStateInformation` only saves `masterVolume`** — nothing else (drum patterns, synth params, preset index, sequencer patterns, effects params) is serialised. Closing and reopening a DAW session resets everything.
- [ ] **`PresetManager` is instantiated in `PluginProcessor` but never used** — `loadPreset()` / `savePreset()` are never called; no preset UI exists.

### 2.4 Synth Engine — Missing Connections

- [ ] **No parameter controls wired to audio:** `SynthEngine::setEnvelopeParams()`, `setFilterParams()`, `setWaveform()`, `setFMAlgorithm()`, `setFMOperatorParams()` all exist but nothing in the UI calls them.
- [ ] **Pitch bend** — `SynthEngine::setPitchBend()` exists but `processBlock` never reads pitch-bend MIDI messages.
- [ ] **All-notes-off** — MIDI all-notes-off (CC 123) and sustain pedal (CC 64) are not handled in `processBlock`.
- [ ] **`SynthMode` switching** — FM vs Classic mode can be set programmatically but there is no UI control for it.

### 2.5 Drum Sequencer — Minor Issues

- [ ] **Synth-based drum voices are not implemented.** `DrumVoice` plays back an audio sample from `DrumTrack::sampleBuffer`. If no sample is loaded, the track is silent. There is no synthesised fallback (e.g. a sine click or noise burst).
- [ ] **Pattern save/load is in-memory only** — patterns reset on plugin reload (tied to state serialisation issue above).
- [ ] **`DrumSequencerPanel::showVelocityMenu` "Custom..." item** does nothing useful — the lambda reads the current velocity and sets it back without asking for user input (needs a text input dialog).

### 2.6 Modulation Matrix — Not Functional

- [ ] **Modulation values are not applied to anything.** `ModulationMatrix::getModulationValue()` exists but no audio-rate code reads from it to modulate oscillator pitch, filter cutoff, etc.
- [ ] **LFO oscillators exist as phase state variables** (`lfo1Phase` etc.) but are never advanced or output to sources.
- [ ] **`ModulationMatrix` is on the UI thread** (see §2.1) — needs to be moved to `PluginProcessor` and be shared thread-safely with the editor.

### 2.7 Code Quality / Robustness

- [ ] **`PluginProcessor.h` still has `#if (MSVC)` / `#include "ipps.h"`** — Intel IPP is an Intel-specific SIMD library, not present on ARM or standard macOS. This include will error on non-MSVC compilers unless the guard is correct (it should be `#ifdef _MSC_VER`).
- [ ] **`PluginProcessor` has a `fprintf(stderr, ...)` debug print in the constructor** — should be removed before release.
- [ ] **`SamplerEngine` uses `juce::CriticalSection zoneLock`** but `process()` never acquires it — the lock is declared but provides no actual thread protection.
- [ ] **`Sequencer::exportMidi()`** is declared but the implementation may be a stub — not verified.
- [ ] **No JUCE `AudioProcessorValueTreeState`** — without APVTS, DAW automation lanes, parameter recall, and proper undo/redo are impossible. This is the standard approach for production JUCE plugins.

### 2.8 Testing

- [ ] No unit tests exist for DSP components (envelope, oscillator, filter, FM operator).
- [ ] Not validated in a DAW (Logic Pro, Ableton, etc.) — plugin loads but audio output and parameter recall need end-to-end verification.
- [ ] No AU validation (`auval -v aufx MvUl MpAu`) — the AU target has not been built or tested.

---

## 3. Technical Specification

### 3.1 Project Overview

| Field | Value |
|-------|-------|
| Plugin name | MultiverseUltimate |
| Company | Multiphase Audio |
| Plugin type | Instrument (synthesizer) |
| Formats | VST3, AU |
| Manufacturer code | `MpAu` |
| Plugin code | `MvUl` |
| Polyphony (synth) | 16 voices (classic), 8 voices (FM) |
| Polyphony (sampler) | 16 voices |
| MIDI in | Yes |
| MIDI out | No |
| Outputs | Stereo |
| Inputs | None (instrument) |
| Tail length | 2.0 seconds |
| Sample rates | Any (validated ≤ 1 MHz) |
| Platform | macOS (Universal Binary: arm64 + x86_64) |
| Deployment target | macOS 10.13 |
| JUCE version | 8.x (`JUCE_PROJUCER_VERSION 0x8000c`) |
| Build system | Projucer + Xcode |
| Project file | `Multiverse.jucer` |
| Build output | `Builds/MacOSX/MultiverseUltimate.xcodeproj` |
| Install path | `~/Library/Audio/Plug-Ins/VST3/MultiverseUltimate.vst3` |

---

### 3.2 Source File Map

```
source/
├── PluginProcessor.h/.cpp       Audio processor — top-level audio + MIDI routing
├── PluginEditor.h/.cpp          Plugin GUI — top-level editor component

├── Synth/
│   ├── SynthEngine.h/.cpp       Polyphonic voice manager (classic + FM modes)
│   ├── Voice.h/.cpp             Classic subtractive voice (OSC + ENV + FILTER)
│   ├── Oscillator.h/.cpp        Wavetable/analogue oscillator (Sine/Saw/Square/Tri/Noise/Sample)
│   ├── Envelope.h/.cpp          ADSR envelope with sample-rate-corrected increments
│   ├── Filter.h/.cpp            SVF filter (juce::dsp::StateVariableTPTFilter)
│   ├── FMVoice.h/.cpp           4-operator FM voice
│   ├── FMOperator.h/.cpp        Single FM operator with independent ADSR and feedback
│   ├── FMAlgorithm.h            8 hard-coded 4-op FM routing algorithms
│   ├── ModulationMatrix.h/.cpp  Modulation routing (11 sources × 15 targets, max 32 connections)
│   ├── ModulationMatrixPanel.h/.cpp  Modulation matrix GUI (connection items with amount sliders)

├── DrumSequencer/
│   ├── DrumSequencer.h/.cpp     Beat sequencer: 8 tracks × 16 steps × 8 patterns, sample playback
│   ├── DrumSequencerPanel.h/.cpp  Drum sequencer GUI with step buttons, track rows, BPM slider

├── Sampler/
│   ├── SamplerEngine.h/.cpp     Sample playback engine, zone-based mapping
│   ├── SamplerVoice.h/.cpp      Per-voice sample playback with ADSR, loop modes, crossfade
│   ├── SamplerZone.h            Zone data (root note, loop points, crossfade, loop mode)
│   ├── SamplerPanel.h/.cpp      Sampler GUI (zone list, waveform display, parameter editors)

├── Sequencer/
│   ├── Sequencer.h/.cpp         Melodic step sequencer + arpeggiator, MIDI output
│   ├── SequencerPanel.h/.cpp    Step grid GUI with note/velocity per step

├── Effects/
│   ├── Effect.h                 Abstract base class (prepare/process/reset)
│   ├── Delay.h/.cpp             Stereo delay (juce::dsp::DelayLine, feedback + mix)
│   ├── Reverb.h/.cpp            Reverb (juce::Reverb, room size / damping / wet/dry)

├── Presets/
│   ├── Preset.h/.cpp            Preset data container (name, category, author, parameter array)
│   ├── PresetManager.h/.cpp     Preset collection with load/save (XML serialisation)
```

---

### 3.3 DSP Signal Path (Current State)

```
MIDI Input
    │
    ├──► SynthEngine (Classic / FM)
    │       ├── 16× Voice (Oscillator → Filter → Envelope)   ──┐
    │       └──  8× FMVoice (4× FMOperator per algorithm)    ──┤
    │                                                           │
    ├──► DrumSequencer (internal clock @ BPM)                  │
    │       └── 32× DrumVoice (sample playback, 8 tracks)  ──► + ──► Output
    │                                                           │
    │    [NOT CONNECTED]                                        │
    ├──►  SamplerEngine ─────────────────── (silent, not mixed)┘
    ├──►  DelayEffect   ─────────────────── (silent, not called)
    └──►  ReverbEffect  ─────────────────── (silent, not called)
```

**Intended signal path:**
```
MIDI → Sequencer → SynthEngine → Effects (Delay → Reverb) → Output
MIDI → DrumSequencer → DrumVoices → Output
MIDI → SamplerEngine → Output
Modulation Matrix → SynthEngine params (pitch, cutoff, volume, etc.)
```

---

### 3.4 Synth Engine Detail

#### Classic Mode (16 voices)
| Stage | Implementation |
|-------|---------------|
| Oscillator | `Oscillator` — analogue models: Sine, Saw, Square, Triangle, Noise, Sample |
| Pulse width | Configurable (Square waveform) |
| Filter | `juce::dsp::StateVariableTPTFilter<float>` (lowpass mode) |
| Envelope | ADSR with time-based per-sample increments (fixed in audit) |
| Voice stealing | Steals least-recently-used voice |
| Polyphony | 16 simultaneous voices |

#### FM Mode (8 voices)
| Field | Value |
|-------|-------|
| Operators per voice | 4 |
| Algorithms | 8 (Series Chain, Twin Mod, Two Stacks, Mod Fork, Additive, Stack+Solo, Chain Fork, Diamond) |
| Operator envelope | Independent ADSR per operator |
| Feedback | Per-operator self-feedback (0–1) |
| Frequency | Ratio × base note frequency |
| Processing order | Op 3→2→1→0 (upstream always resolved first) |

---

### 3.5 Drum Sequencer Detail

| Field | Value |
|-------|-------|
| Tracks | 8 |
| Steps per track | 16 |
| Saved patterns | 8 |
| BPM range | 40–240 BPM (0.5 step) |
| Voice pool | 32 (4 simultaneous voices per track) |
| Per-step data | active (bool), velocity (float 0–1) |
| Per-track data | name, volume (0–1), muted, solo, root note, sample buffer |
| Sample loading | WAV, AIFF, FLAC, OGG, MP3 (via file chooser or drag-and-drop) |
| Level metering | Atomic peak tracking with exponential decay (~0.5 s) |
| UI clock | 50 ms timer for step highlight |

---

### 3.6 Modulation Matrix Detail

| Field | Value |
|-------|-------|
| Sources | 11: LFO1–4, Envelope, Velocity, NoteNumber, Aftertouch, ModWheel, PitchBend, Random |
| Targets | 15: OSC Pitch/Level/Wave, Filter Cutoff/Resonance, Amp Volume/Pan, LFO1–4 Rate, Effect Param 1–3, Effect Mix |
| Max connections | 32 |
| Amount range | –1.0 to +1.0 |
| Per connection | source, target, amount, enabled, sourceIndex, targetIndex |
| LFO state | 4× phase floats (lfo1Phase–lfo4Phase), not yet advanced |

---

### 3.7 Sampler Engine Detail

| Field | Value |
|-------|-------|
| Voices | 16 (`MvSamplerVoice`) |
| Zone model | `SamplerZone` (root note, loop start/end, crossfade length, loop mode) |
| Loop modes | None, Forward, PingPong |
| Playback | Linear interpolation, pitch-shifted by MIDI note vs root note |
| Envelope | Inline ADSR per voice (attack 5 ms, decay 100 ms, sustain 0.7, release 300 ms) |
| Crossfade | Sample-level crossfade at loop boundary |
| Thread safety | `juce::CriticalSection` on zone list (not yet acquired in `process()`) |

---

### 3.8 Effects Detail

#### Delay (`DelayEffect`)
| Parameter | Default | Range |
|-----------|---------|-------|
| Time | 0.5 s | 0–2 s |
| Feedback | 0.3 | 0–1 |
| Mix | 0.5 | 0–1 |
| Implementation | `juce::dsp::DelayLine<float>` (no interpolation) | — |

#### Reverb (`ReverbEffect`)
| Parameter | Default | Notes |
|-----------|---------|-------|
| Room size | — | `juce::Reverb::Parameters` |
| Damping | — | — |
| Wet level | — | — |
| Dry level | — | — |
| Implementation | `juce::Reverb` | Schroeder allpass/comb |

---

### 3.9 Sequencer (Melodic) Detail

| Field | Value |
|-------|-------|
| Modes | Sequencer, Arpeggiator |
| Steps | 1–16 (configurable) |
| Saved patterns | 8 |
| Per-step data | noteNumber (MIDI 0–127), velocity (0–1), active (bool) |
| MIDI output | Generates `noteOn`/`noteOff` into a `juce::MidiBuffer` |
| MIDI export | `exportMidi()` returns `juce::MidiFile` |
| Arpeggiator | Up to 16 held notes, cycles in order |

---

### 3.10 State Serialisation (Current)

`getStateInformation` writes only:
- `float masterVolume` (4 bytes)

`setStateInformation` reads only:
- `float masterVolume` (if `sizeInBytes >= 4`)

Everything else (patterns, presets, synth parameters, effects, sequencer) is lost on plugin reload.

---

### 3.11 Build Configuration

| Setting | Value |
|---------|-------|
| Architectures | arm64, x86_64 (Universal Binary) |
| Optimisation (Release) | `-O3` (maximum speed) |
| LTO | Enabled |
| C++ standard | C++17 (JUCE 8 requirement) |
| macOS deployment | 10.13 (High Sierra) |
| Code signing | Ad-hoc (no developer certificate configured) |
| Frameworks linked | Accelerate, AudioToolbox, Cocoa, CoreAudio, CoreAudioKit, CoreMIDI, DiscRecording, Foundation, IOKit, QuartzCore, Security, WebKit, Metal, MetalKit |
| JUCE options | `JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1` |

---

## 4. Recommended Next Steps (Priority Order)

1. **Wire delay and reverb into `processBlock`** — 30 minutes, high impact.
2. **Wire `SamplerEngine` into `processBlock`** (MIDI routing + mix to output buffer).
3. **Move `ModulationMatrix` to `PluginProcessor`** and pass a reference to the editor.
4. **Implement full `getStateInformation`/`setStateInformation`** using `juce::ValueTree` or at minimum raw binary for all parameters.
5. **Add a synth parameter UI** (ADSR, filter, waveform, FM operator sliders).
6. **Add a tab/navigation bar** to the editor to expose all panels.
7. **Connect LFO advancement in `ModulationMatrix`** and route modulation values to voice parameters.
8. **Migrate to `AudioProcessorValueTreeState`** for proper automation, undo, and recall.
9. **Add AU build and run `auval`** validation.
10. **Remove debug `fprintf` from constructor.**
