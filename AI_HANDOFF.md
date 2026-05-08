# MultiverseUltimate — AI HANDOFF

## Project Identity
- **Name:** MultiverseUltimate
- **Type:** Hybrid Synthesizer (FM + Subtractive + Granular + Sampler + Drum Sequencer)
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
| SynthEngine | 16-voice classic (1–8 osc/voice: Classic/Wavetable/Additive/PhaseDist/Analog/Digital) / 8-voice FM |
| GranularEngine | 16-voice granular, 32 grains/voice (512 total), file or built-in source |
| DrumSequencer | Sample-based, 32 voices |
| SamplerEngine | Zone-based playback, 16 voices |
| LayerManager | 8 independent layers, each Synth/Granular/Sampler/Off, level/pan/mute/solo |
| Effects | Chorus → Distortion → EQ → Compressor → Delay → Reverb (reorderable chain) |
| ModulationMatrix | LFO + envelope routing to all DSP params |
| Sequencer / ProSequencer / Arpeggiator / PatternEngine | MIDI generation |

---

## Signal Flow

```
MIDI → Sequencer → SynthEngine   ─┐
MIDI → GranularEngine            ─┤
MIDI → DrumSequencer             ─┼─► Effect Chain ──► Output
MIDI → SamplerEngine             ─┘
ModulationMatrix → all DSP parameters (pitch, cutoff, volume, …)
```

**Effect chain** (default order, drag-to-reorder in UI):
`Chorus → Distortion → EQ → Compressor → Delay → Reverb`
Reverb is always applied as a stereo block op; the chain correctly splits pre/post around it.

---

## What Works

- SynthEngine (16-voice classic with 1–8 osc/voice + FM 8-voice), DrumSequencer, SamplerEngine — all produce audio ✅
- GranularEngine (16-voice × 32 grains, 4 env shapes, pitch scatter, spray, density, stereo spread, file loading) ✅
- Up to 8 oscillators per voice: Classic / Wavetable / Additive / PhaseDist / Analog / Digital; per-osc wave shaping (Drive/Fold/Clip) and self-oscillation feedback ✅
- Voice modes: Poly / Mono / Legato with portamento (per-sample semitone-space glide) ✅
- All 6 effects with full parameter control and MIDI Learn ✅
- Effect chain reordering — drag-to-reorder strip, order persists in presets ✅
- ModulationMatrix — LFO → pitch/cutoff/volume/effects ✅
- Melodic Sequencer, DAW transport sync ✅
- Full state persistence (XML) including 8 oscillators (type/level/detune/waveform/wavePos/shapeType/shapeAmt/selfOsc/phaseDist) + oscCount + effect chain order + granular source path ✅
- MIDI Learn on all effect panel knobs ✅
- Undo/Redo (Cmd+Z / Cmd+Shift+Z) ✅
- Filter oversampling (Off/2x/4x/Auto) ✅
- Preset system (XML) with Factory/User banks ✅
- State-of-the-art preset browser: 8-color favorites, #hashtags, auto-preview, metadata detail strip, back/forward history, save dialog, right-click context menu ✅
- Flat UI theme (`MultiverseFlatTheme`) — clean Nexus 5-inspired design, no neumorphic shadows ✅
- NeuKnob (`Source/NeuKnob.h/.cpp`) — value pill on hover/drag, amber arc when macro-assigned ✅
- SynthDisplay (`Source/Synth/SynthDisplay.h/.cpp`) — real-time oscilloscope (left) + FFT spectrum (right), 30 Hz, lock-free FIFO ✅
- WavetableEditor (`Source/Synth/WavetableEditor.h/.cpp`) — draw/edit wavetable frames, formula gen, import ✅
- LFOShapeEditor (`Source/Synth/LFOShapeEditor.h/.cpp`) — drawable custom LFO shapes; pencil/line tools, fill buttons, normalize; launched as CallOutBox from DRAW button in each LFO row ✅
- LayerManager (`Source/Layers/`) — 8 independent layers (Synth/Granular/Sampler), MIDI + audio + state ✅

## What Is Broken / Unconnected
- None (WavetableEditor FFT button is a placeholder — just calls normalizeFrame; real FFT not yet implemented)

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
| Granular | Granular engine — 16 voices × 32 grains, file loading, 4 env shapes, full ADSR, new "Granular" tab |
| 2+5 | Velocity/NoteNumber/Random/EnvelopeFollower sources wired; 5 granular mod targets added; MAX_MOD_TARGETS=24 |
| MPE | Per-note pitch bend (±48 st), pressure, slide; MPE Pressure + MPE Slide mod sources; "MPE" toggle in SynthPanel |
| UI-1 | MultiverseFlatTheme LookAndFeel — Nexus 5-inspired flat design system; clean knobs, sliders, buttons, tabs, menus, `drawCard()` flat cards |
| UI-2 | NeuKnob — extends MidiLearnSlider; value pill on hover/drag; amber arc when macro-assigned |
| UI-3 | SynthDisplay — real-time oscilloscope + FFT spectrum; lock-free FIFO in PluginProcessor |
| UI-4 | PresetBrowserPanel — 280px, search bar, category pills, tag filter, metadata strip, flat cards |
| UI-5 | **COMPLETE** — All panels updated to MultiverseFlatTheme flat palette |
| UI-6 | **COMPLETE** — Flat card system: `MultiverseFlatTheme::drawCard()` applied to all panels (replaces neumorphic `drawNeumorphicRect()`) |
| UI-7 | **COMPLETE** — SynthPanel, GranularPanel, MacroPanel flat section cards |
| UI-8 | **COMPLETE** — Phase 1 of Nexus 5 UI overhaul: MultiverseFlatTheme replaces CyberpunkTheme; 5-phase plan (Phase 1 done, Phases 2–5 next) |
| WavetableEditor | Visual wavetable editor: draw tools, formula generators, normalize/fade/reverse/import, per-osc "EDIT WT" button in SynthPanel |
| Layers | 8-layer engine: Synth/Granular/Sampler per layer, level/pan/mute/solo, full MIDI + audio + state wiring, "Layers" tab |
| Track B | Filter LP/HP/BP/Notch; Sub+Noise osc; Unison Chord/Random spread; Layer key/vel/MIDI-ch ranges; Per-layer independent effect chain (LayerEffectChain) |
| Phase 1 | Dynamic 1–8 osc count; 4 new types (Additive/PhaseDist/Analog/Digital); per-osc wave shaping + self-osc; 2 new mod targets (OscShapeAmount, OscPhaseDistAmount) |
| Phase 2 | Sampler: per-zone `tuning` (±24 st) + `speed` (0.25–4×) controls; Lo/Hi Key + Lo/Hi Vel range editing UI; Auto Map button (distributes zones evenly across 0–127) |
| Phase 3 | Sequencer: MIDI drag-drop import (FileDragAndDropTarget); smart chord tracking (10-chord table, 50ms); per-pattern step length (SeqPattern::stepLengthMultiplier, STEP ComboBox); per-step probability (Step::probability, dot indicator, right-click submenu); ModSourceType::SequencerStep mod source (normalized 0→1) |
| Phase 4 | 17-bus multi-output (all-DAW); per-drum-track FX + individual bus routing |
| Phase 5 | 8 LFOs (LFO5-8 enum slots 15-18); LFOShape enum (Sine/Tri/Saw/Square/S&H); DAW-sync per LFO (lfoXSync Bool + lfoXSyncDiv Choice); Env2/Env3 mod envelopes (juce::ADSR); LFO Banks section in ModulationMatrixPanel |
| Phase 6 | Resizable UI (setResizable + setResizeLimits 800-1920); Scale combo 75-150% in header; built-in keyboard (keyboardState in processor, MidiKeyboardComponent 64px in editor); RAND button with randomizeParams() |
| Phase 7 (partial) | CPU voice limit: setVoiceLimit(int) in SynthEngine, maxVoices APVTS Choice, findFreeVoice limited to voiceLimit slots; Metronome: PPQ beat detection, 25ms decaying sine click at 1200/900Hz into buffer |
| Phase 7 (more) | Aux sends (7.1): `auxSendDelay`/`auxSendReverb` APVTS params; `auxDelay`/`auxReverb` parallel effect instances; dry mix captured pre-chain; sends mixed post-pan; SENDS card in EffectsPanel. Tuner (7.7 UI): FFT peak detection with parabolic interpolation; MIDI note + cents readout; green/amber cents bar; drawn in SynthDisplay scope area. |
| Drawable LFO | LFOShape::Custom (index 5); LFOShapeEditor component (pencil+line+fill+norm); DRAW button per LFO row; 256-pt table lookup in advanceLFOs(); per-LFO tables persist in preset XML |
| Chord/Strum | chordModeEnabled/chordShape/chordStrumDelay APVTS params; 12 shapes (Major/Minor/Maj7/Min7/Dom7/Dim/Aug/Sus2/Sus4/Power/Octave/Root); PendingNote[64]+ActiveChord[32] pre-alloc queue; chord tones fire at block start, strum delay = ni×strumSamples; CHORD/STRUM card in SynthPanel (Classic mode only); MPE bypasses chord mode |
| Performance View | `Source/Performance/PerformancePanel.h/.cpp` — "Perf" tab; 4×2 grid of large macro rotary knobs (SliderAttachment to macro1-8) with editable name labels; XYPad inner class (maps macro1=X, macro2=Y, mouse drag calls setValueNotifyingHost, timer updates dot from APVTS at 30 Hz); BPM readout from getPlayHead(); neumorphic section cards; macro names sync from MacroManager at 30 Hz |
| Factory Presets | `Source/Presets/FactoryPresets.h/.cpp` — 100 presets (Init×5, Bass×20, Lead×20, Pad×25, Drums×15, FX×15); `PresetData` struct with helpers (`setAdsr/setFilter/setReverb/setChorus/setDelay/setUnison/setMono/applyOscs`); full APVTS XML (all 220+ params) per preset for clean load; normalization helpers `nLin/nSkw/nCh` + shorthands `A/D/R/FC/FR/LR/PT/DT`; `PresetManager::createFactoryPresetsIfNeeded` calls `FactoryPresets::writeToDirectory` when < 10 presets found (recursive scan) |
| Preset Browser 2.0 | `PresetManager` extended with metadata cache, 8-color favorites JSON, tag index, back/forward history; `PresetBrowserPanel` redesigned — 280px, category pills, tag filter, auto-preview, save dialog, right-click context menu; header navigation bar (prev/next/name/fav/back/forward); all 100 factory presets have author/description/tags metadata |
| Plugin Classification | `JucePluginDefines.h` corrected: IsSynth=1, WantsMidiInput=1, Vst3Category="Instrument", AUMainType='aumu' (was incorrectly set to effect) |

## Next Steps

### Post-Gap Feature Roadmap (Current Focus)
All gap-fill phases (0–7) complete. Now in competitive feature expansion.
**Priority order:**
1. ~~Drawable LFO Shapes~~ ✅ (2026-05-05)
2. ~~Chord/Strum Mode~~ ✅ (2026-05-05)
3. ~~Performance View~~ ✅ (2026-05-05)
4. ~~Programmatic preset generation~~ ✅ (2026-05-05) — 100 factory presets, 6 categories
5. ~~Preset browser 2.0~~ ✅ (2026-05-08) — metadata, favorites, tags, auto-preview, history, save dialog

**Next:** UI Overhaul Phase 2 — Layout Restructure (see Nexus 5 plan)

**Deferred (need decisions or Projucer GUI action):**
- 7.2 Standalone mode — enable in Projucer GUI (File Formats → Standalone Plugin)

### Nexus 5 UI Overhaul
**Phase 1 (Flat Theme) COMPLETE** — `MultiverseFlatTheme` replaces `CyberpunkTheme`. All panels render with flat cards via `drawCard()`. No neumorphic shadows anywhere.

**Remaining phases** (see plan at `/Users/jason/.claude/plans/kind-popping-nygaard.md`):
- Phase 2: Layout Restructure (permanent left sidebar 280px, right FX strip 200px, bottom bar 88px with 8 macros + keyboard, compact header)
- Phase 3: Librarian (replace preset browser overlay with permanent sidebar: category tree, character tags, bookmarks, search)
- Phase 4: Quick FX Strip (right-side panel: Filter/Amp Modifier, Delay, Reverb, Main Filter — bipolar offset controls)
- Phase 5: Visual Routing (ROU tab with signal flow graph: generator→layer→FX blocks, draggable connections)

---

## Key Technical Facts

| Detail | Value |
|--------|-------|
| Polyphony (classic) | 16 voices |
| Polyphony (FM) | 8 voices |
| Polyphony (granular) | 16 voices × 32 grains |
| Polyphony (sampler) | 16 voices |
| Drum voices | 32 (4/track) |
| Manufacturer code | `MpAu` |
| Plugin code | `MvUl` |
| Plugin type | Instrument (VST3 category: "Instrument", AU: 'aumu') |
| C++ standard | C++17 |
| JUCE version | 8.x |
| macOS target | 10.13+ |

---

## Conventions & Gotchas

- Include JUCE as `<JuceHeader.h>` (angle brackets) — do not revert to quoted path.
- Sampler class is `MvSamplerVoice` — renamed to avoid conflict with `juce::SamplerVoice`.
- Font: `juce::Font(size, style)` still compiles in JUCE 8 but is deprecated — use `juce::Font(juce::FontOptions{}.withHeight(size))` for new code. Old form raises `-Wdeprecated-declarations` but does not break the build.
- `juce::TextEditor::caretColourId` does not exist in JUCE 8 — do not use it.
- `juce::TableHeaderComponent::separatorColourId` does not exist in JUCE 8 — do not use it.
- `MultiverseFlatTheme` (`Source/MultiverseFlatTheme.h/.cpp`) is the global LookAndFeel installed in `PluginEditor`. Flat palette constants (`bgBase`, `bgRaised`, `bgDeep`, `accentCyan`, `accentPink`, etc.) are `static const` members — include `MultiverseFlatTheme.h` to access them from panels. Backward-compat aliases: `accentBlue`→`accentCyan`, `neonCyan`→`accentCyan`, `neonPink`→`accentPink`, `neonPurple`→`accentPurple`, `neonGreen`→`accentGreen`.
- `MultiverseFlatTheme::drawCard()` is `public static` — callable from panel paint() for flat section card borders (bgRaised fill + borderLight/borderActive stroke). Replaces old `drawNeumorphicRect()`.
- `CyberpunkTheme.h/.cpp` still exist in the project but are NO LONGER used by any panel. They can be safely deleted when convenient.
- `ModulationMatrix::getActiveConnectionsForTarget()` returns by value (thread-safety fix — do not change to ref/pointer).
- Effect chain order is packed as 6 nibbles in `std::atomic<uint32_t> effectChainOrder` — `getChainSlot(pos)` / `swapChainSlots(a,b)` are the only API.
- New effects (Chorus/Distortion/EQ/Compressor) have stereo L/R instances (`chorus[2]`, etc.); Delay is shared mono.
- `EQEffect::updateCoeffs()` uses biquad math per setter call (once per block from processBlock) — no heap allocation.
- Voice mode glide runs **per-sample** in `processBuffer` using `std::exp2((semitone - 69) / 12)` — only active in Mono/Legato, skipped entirely in Poly.
- Mono note stack is `int monoNoteStack[16]` + `int monoNoteCount` in SynthEngine (no heap alloc). Stack is cleared in `allNotesOff()`.
- `Voice::setFrequencyDirect(hz)` sets `baseFrequency` and calls `updateOscillatorFrequencies()` without touching the envelope — used by the glide loop.
- `Voice::setNoteLegato(note)` updates `midiNote` + `baseFrequency` only — no envelope retrigger. Used in Legato mode on gate-open note changes.
- APVTS params added by Phase A: `voiceMode` (Choice 0–2), `portamento` (Float 0–2s), `portaAlways` (Bool).
- GranularEngine source buffer: `juce::ScopedTryLock` in processBlock (non-blocking); if lock fails, silence for that block — safe for audio thread.
- GranularVoice grain pool: `GrainState grains[32]` — stack-allocated, no heap. Grain steal: first inactive slot, fallback index 0.
- Granular pitch ratio: `exp2f((midiNote - 60) / 12.0f)` × `exp2f(scatterSemitones / 12.0f)`.
- GranularEngine file loading: `juce::LagrangeInterpolator` for resampling, stereo conversion, writes under `juce::ScopedLock`.
- MPE Lower Zone: ch 1 = master (global bend ±2 st, CC64/66/1/123), ch 2–15 = member (per-note bend ±48 st, channel pressure, CC74 slide).
- `VoiceInfo::midiChannel` (0 = unassigned; 2–15 for MPE). `MpeChannelState[16]` in SynthEngine holds pitchBend/pressure/slide per channel.
- MPE channel state reset on `noteOnMPE()` — prevents stale pitch bend bleed when a channel is recycled.
- CC74 neutral value is 63 (not 0); normalised to -1..+1 before storing in `mpeChannels[].slide`.
- `ModSourceType::MPEPressure` and `ModSourceType::MPESlide` are the two new mod sources; fed from most-recent member-channel message.
- `NeuKnob` extends `MidiLearnSlider` — use NeuKnob everywhere you'd use MidiLearnSlider; it's a drop-in. Value pill fires from paint() (isMouseOver/isMouseButtonDown check). Amber arc managed by ArcTimer (inner juce::Timer struct, 10 Hz), calls setColour/removeColour only on state change to avoid repaint storms. `CyberpunkTheme::drawRotarySlider` uses `slider.findColour(rotarySliderFillColourId)` — component-level setColour overrides the LookAndFeel color.
- `juce::Font::getStringWidthFloat` does not exist in this project's JUCE version — use character-count estimation or `getStringWidth` (int) for pill sizing.
- `SynthDisplay`: lives at `Source/Synth/SynthDisplay.h/.cpp`. Ring buffer: `RING_SIZE=2048`, `ringWritePos` (masked with `& (RING_SIZE-1)`). FFT: `juce::dsp::FFT fft{FFT_ORDER}` (order 10, 1024-point). Audio data arrives via `PluginProcessor::pullDisplaySamples(float*, int)` (public, message-thread safe). `pushDisplaySamples` is private, called at end of processBlock after pan.
- `PresetBrowserPanel` (`Source/Presets/PresetBrowserPanel.h/.cpp`): inherits `juce::ListBoxModel` + `Button::Listener` + `ComboBox::Listener` + `TextEditor::Listener` + `juce::Timer`. 280px collapsible panel with search bar, 8 category pills (All/Init/Bass/Lead/Pad/Drums/FX/Favs), tag filter pills, preset list with favorite color dots, metadata detail strip. Auto-preview (50ms timer poll), save dialog (CallOutBox), right-click context menu. Tag filtering uses AND logic. Favorites stored in `favorites.json` (8 colors: Red/Orange/Yellow/Green/Cyan/Blue/Purple/Pink). `PresetManager` has `PresetMetadata` cache, `tagIndex`, favorites, history stack.
- `WavetableEditor` (`Source/Synth/WavetableEditor.h/.cpp`): takes `WavetableOscillator&` (voice 0 master). `onWavetableChanged` callback must be set by caller to `synthEngine.distributeWavetable(oscIndex)`. Shown as full-panel overlay inside SynthPanel; toggled by "EDIT WT" button per osc strip. FFT button is a placeholder (calls normalizeFrame).
- `LFOShapeEditor` (`Source/Synth/LFOShapeEditor.h/.cpp`): standalone component, no external dependencies. `setTable(array<float,256>)` to pre-populate; `onTableChanged` callback fires after every edit. Fixed ±1 canvas with center-line grid; pencil (freehand drag), line (mouseDown→mouseUp), SIN/SAW/SQR/TRI fill, NORM. Launched via `CallOutBox::launchAsynchronously` from `LFORow::drawButton`. Size 310×160. `LFOShape::Custom` in advanceLFOs uses linear interpolation: `idx0=(int)(phase/2π*255)&255`, frac blend with `idx1=(idx0+1)&255`. Tables persisted as `lfo1CustomTable`–`lfo8CustomTable` comma-separated float properties in ModulationMatrix XML state.
- `WavetableOscillator` new public API: `setSample(frame,i,v)`, `getSample(frame,i)`, `clearFrame`, `normalizeFrame`, `fadeFrame`, `reverseFrame`, `generateFormula(frame, fn)`, `loadMultiCycleWavetable`, `processFFT` (placeholder), `getTableCount()`, `getTableSize()`.
- `SamplerZone` has `tuning` (float, ±24 semitones) and `speed` (float, 0.25–4.0) fields. Both are persisted in getState/setState XML with defaults 0.0/1.0. `MvSamplerVoice::noteOn` folds them in: `playbackRate = exp2((midiNote-rootNote+tuning)/12) * (fileSR/sr) * speed`.
- `SynthEngine::getWavetableOscillator(oscIndex)` returns `voices[0].voice.getWavetableOsc(oscIndex)` — voice 0 is the edit master. `distributeWavetable(oscIndex)` copies frame data from voice 0 to voices 1–15 via `setSample`.
- `LayerManager` (`Source/Layers/`) manages `std::array<unique_ptr<LayerEngine>, 8>`. Each `LayerEngine` owns its own `SynthEngine`, `GranularEngine`, `SamplerEngine` (all three always allocated; only active engine processes). `LayerManager::processBlock` mixes all non-muted layers (or only soloed layers) into the output buffer. MIDI goes to all active layers. State: `juce::ValueTree` — GranularEngine + SamplerEngine sub-states only (SynthEngine has no standalone state API).
- `LayersPanel` (`Source/Layers/LayersPanel.h/.cpp`): `rows` is `std::array<unique_ptr<LayerRow>, 8>`. `LayerRow` is a struct (not a Component) holding JUCE controls as direct members. Lambdas capture `[this, index, row]` — `row` is a raw pointer to the unique_ptr content, valid for the panel's lifetime.
- `Filter::FilterType {LP, HP, BP, Notch}` — LP/HP/BP use `StateVariableTPTFilter::Type`; Notch uses `juce::dsp::IIR::Filter<float>` biquad (JUCE 8 SVF has no `notch` type). `setFilterType` / `setCutoff` / `setResonance` all update notch coeffs when type == Notch. Three IIR instances for 1x/2x/4x oversampling.
- `SubOscState` in Voice: `enabled`, `level`, `waveform`, `Oscillator osc` — always at `baseFreq * 0.5f`, updated in `updateOscillatorFrequencies()`. `NoiseOscState`: `enabled`, `level`, `colorCutoffHz`, one-pole LP (`colorCoeff = 1 - exp(-2π*fc/sr)`). Both mixed into `Voice::process()` before envelope.
- `SynthEngine::UnisonSpreadMode {Stacked, Chord, Random}` — Chord uses fixed semitone array `[0,4,7,12,16,19,24,28]` (major chord); Random uses `juce::Random::getSystemRandom()` per voice per noteOn.
- `LayerEffectChain` (`Source/Layers/LayerEffectChain.h/.cpp`): owns `ChorusEffect chorus[2]`, `DistortionEffect distortion[2]`, `EQEffect eq[2]`, `CompressorEffect compressor[2]`, `DelayEffect delay[2]`, `ReverbEffect reverb`. Per-effect `Slot {bool enabled; float mix}`. `processBlock` wet/dry blends each enabled effect; Reverb uses stereo `processBlock(L,R,n)`. State saved as `LayerFX` XML child of LayerEngine. "FX" button in LayersPanel row opens `juce::CallOutBox` with 6 enable toggles + mix sliders (heap-allocated Component, owned by CallOutBox).
- `LayerEngine::noteOn(int note, float vel, int midiChannel)` — filters on `loNote/hiNote`, `loVel/hiVel` (vel×127), `midiChannelFilter` (0=all). `LayerManager::noteOn` passes `midiChannel` (from `message.getChannel()` in processBlock).
- New APVTS params: `filterType` (Choice), `unisonSpreadMode` (Choice), `subOscEnable/Level/Wave` (Bool/Float/Choice), `noiseOscEnable/Level/Color` (Bool/Float/Float 200-20000Hz skew 0.3).
- **Phase 1 — Oscillator system:** `OscillatorType` enum has 6 values: Classic(0), Wavetable(1), Additive(2), PhaseDist(3), Analog(4), Digital(5). `OscShapeType` enum: Off(0), Drive(1), Fold(2), Clip(3). `OscState` struct now has `phaseDistAmount`, `shapeType`, `shapeAmount`, `selfOscFeedback`, `selfOscPrev`, `analogLCG` fields. `std::array<OscState,8>` in Voice; `std::array<OscSettings,8>` in SynthEngine; `int activeOscs` in Voice; `int oscCount` in SynthEngine.
- `Voice::process()` iterates `0..activeOscs-1`; skips slots with `level <= 0`. Additive: 8-harmonic sum via `classicOsc.getPhase()` after `classicOsc.process()`. PhaseDist: `sin(phase + amount * sin(2π*phase))`. Analog: Classic + LCG jitter `±0.015`. Digital: Classic + 16-level quantise. Wave shaping applied inline via `applyOscShaping()` (file-static helper, no heap alloc).
- APVTS: `osc1`–`osc8` each have 9 params (`Type/Level/Detune/Waveform/WavePos/ShapeType/ShapeAmt/SelfOsc/PhaseDist`). `oscCount` Choice param (0-indexed, +1 = actual count, default index 2 = 3 oscs). Old presets: oscs 4–8 default to level 0, fully backward-compatible. `osc1Type`–`osc3Type` old choices (2 options) safely extended to 6 options (JUCE stores raw index).
- `SynthEngine::setOscCount(n)` clamps to 1–8 and calls `voice.setActiveOscs(n)` on all 16 voices. All noteOn paths (Poly/Mono/Legato/MPE) propagate `setActiveOscs(oscCount)` + all 9 per-osc setter calls.
- Mod targets `OscShapeAmount` (index 20) and `OscPhaseDistAmount` (index 21) added. `MAX_MOD_TARGETS` raised from 24 to 26. Wired in `processBlock` on osc 0 only (base param `osc1ShapeAmt`/`osc1PhaseDist` + mod sum).
- SynthPanel: `oscControls[8]`, `wavetableEditors[8]`. Layout: ≤4 strips = 1 row (220px); 5–8 strips = 2 rows (460px). `+ OSC`/`- OSC` buttons at top of OSC section. Shape controls (shapeTypeSelector, shapeAmtSlider, selfOscSlider, phaseDistSlider) added to each strip; shapeAmt hidden when Off; phaseDistSlider hidden unless PhaseDist type selected. `updateVisibility()` reads `oscCount` APVTS value to determine which strips are shown.
- **Phase 3 — Sequencer:** `Step` struct now has `float probability = 1.0f`; `SeqPattern` now has `float stepLengthMultiplier = 1.0f` (0.5=32nd, 1=16th, 2=8th, 4=quarter, 1.333=8th-triplet). `updateSamplesPerStep()` multiplies base 16th-note duration by `stepLengthMultiplier`. `loadPattern()` calls `updateSamplesPerStep()` after copy. `setState()` calls `updateSamplesPerStep()` at end. `setStepLengthMultiplier()` + `setStepProbability()` are the setters. Probability check uses xorshift32 `fastRand()`: skip if `fastRand() >= (uint32_t)(prob * UINT32_MAX)`. `ModSourceType::SequencerStep` is enum slot 14 (within MAX_MOD_SOURCES=16); wired in processBlock after `sequencer.process()` as `currentStep / (numSteps-1)`. `SequencerPanel` inherits `FileDragAndDropTarget`; `filesDropped()` opens first .mid/.midi via `juce::MidiFile`, converts timestamps to seconds, quantizes note-ons to the 16-step grid — slot = `round(t / maxTime * numSteps)`. `dragOver` bool triggers cyan border glow in `paint()`. `detectChord()` collects pitch classes from active steps and calls file-static `detectChordFromClasses()` which tries all 12 roots against 10 chord patterns; result shown in `chordLabel` (right of export button).
- **Phase 5 — Modulation upgrades:** `ModSourceType` now has 21 values (0-20): LFO1-LFO4=0-3, Envelope=4, Velocity=5, NoteNumber=6, Aftertouch=7, ModWheel=8, PitchBend=9, Random=10, EnvelopeFollower=11, MPEPressure=12, MPESlide=13, SequencerStep=14, LFO5-LFO8=15-18, Envelope2=19, Envelope3=20. `MAX_MOD_SOURCES=21`. `ModTargetType::LFO5Rate-LFO8Rate` = indices 22-25 (MAX_MOD_TARGETS=26 unchanged). `LFOShape` enum: Sine/Triangle/Saw/Square/SampleAndHold. `lfoPhase[8]`/`lfoRate[8]`/`lfoShape[8]`/`lfoSHValue[8]` arrays replace old individual members. `setLFOShape(int, LFOShape)` API. `advanceLFOs()` maps i=0-3 to sourceValues[0-3], i=4-7 to sourceValues[15-18] (formula: srcIdx = i < 4 ? i : 11+i). Triangle: `1 - 2*|p/π - 1|`; Saw: `p/π - 1`; Square: `p < π ? 1 : -1`; S&H: new random on phase wrap. DAW sync params: `lfoXSync` (Bool) + `lfoXSyncDiv` (Choice 0-7: 1/32,1/16,1/8,1/4,1/2,1/1,2/1,4/1); synced rate = `bpm/60 / divisor`. Env2/3: `juce::ADSR modEnv2, modEnv3` in PluginProcessor, triggered from noteOn/noteOff in MIDI loop, `setSampleRate` in prepareToPlay, advanced with `getNextSample()` per block, value pushed to `setModulationValue(ModSourceType::Envelope2/3, 0, level)`. APVTS: `lfoXRate/Shape/Sync/SyncDiv` (1-8), `modEnv2/3Attack/Decay/Sustain/Release`. LFO Banks section in ModulationMatrixPanel: 8 `LFORow` components above connections, each with rate/shape/sync/syncDiv APVTS attachments.
- **Phase 4 — Multi-output buses:** `PluginProcessor` declares 17 fixed stereo output buses in constructor: Bus 0 = main (always active), Buses 1–8 = Layer 1–8 (disabled by default), Buses 9–16 = Drum Track 1–8 (disabled by default). `isBusesLayoutSupported()` requires bus 0 stereo; all others stereo-or-disabled. In `processBlock`, individual-bus layers are routed via `getBusBuffer(buffer, false, busIdx)` after the main effects chain; individual drum buses are routed from `drumSequencer.getTrackBuffer(t)` via the same mechanism. `LayerManager::processBlock` skips layers with `outputBusIndex > 0` to avoid double-processing. `LayersPanel` has a "BUS" ComboBox per row (Main / Bus 1–8); `DrumSequencerPanel::TrackRow` has "BUS" ComboBox (Main / Out 1–8) and "FX" button. Bus mapping: LayerEngine `outputBusIndex` 0=main, 1–8=buses 1–8; DrumSequencer `trackOutputBus[t]` 0=main, 9–16=buses 9–16.
- **Phase 4 — Drum per-track FX:** `DrumSequencer` now has `std::array<LayerEffectChain, 8> trackFX` and `std::array<juce::AudioBuffer<float>, 8> trackBufs` (pre-allocated in `prepare()`). `process()` refactored: voices accumulate into `trackBufs[av.trackIndex]` per sample; after the sample loop, `trackFX[t].processBlock(trackBufs[t], n)` is applied per track; bus-0 tracks are summed into `mainBuffer`; non-zero-bus tracks remain in `trackBufs[t]` for PluginProcessor routing. `getTrackBuffer(t)` / `getTrackFX(t)` / `setTrackOutputBus(t, bus)` / `getTrackOutputBus(t)` are the new public API. Track FX + outputBus persisted as XML children in the Track node of `DrumSequencer::getState()`. `showTrackFXPopup(int track)` in DrumSequencerPanel launches a `CallOutBox` with 6 enable toggles + mix sliders — identical pattern to `LayersPanel::showFXPopup`.
