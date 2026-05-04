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
- Dark Forge UI theme (`CyberpunkTheme`) — neumorphic knobs, sliders, buttons, tabs, menus ✅
- NeuKnob (`Source/NeuKnob.h/.cpp`) — value pill on hover/drag, amber arc when macro-assigned ✅
- SynthDisplay (`Source/Synth/SynthDisplay.h/.cpp`) — real-time oscilloscope (left) + FFT spectrum (right), 30 Hz, lock-free FIFO ✅
- WavetableEditor (`Source/Synth/WavetableEditor.h/.cpp`) — draw/edit wavetable frames, formula gen, import ✅
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
| UI-1 | CyberpunkTheme LookAndFeel — Dark Forge design system; neumorphic knobs, sliders, buttons, tabs, menus |
| UI-2 | NeuKnob — extends MidiLearnSlider; value pill on hover/drag; amber arc when macro-assigned |
| UI-3 | SynthDisplay — real-time oscilloscope + FFT spectrum; lock-free FIFO in PluginProcessor |
| UI-4 | PresetBrowserPanel — Dark Forge redesign, 220px, search bar, category pills, neumorphic cards |
| UI-5 | **COMPLETE** — EffectsPanel, ModulationMatrixPanel, SamplerPanel, SequencerPanel, DrumSequencerPanel, ArpeggiatorPanel, ProSequencerPanel all updated to Dark Forge palette |
| UI-6 | **COMPLETE** — Section card system: neumorphic cards via `drawNeumorphicRect()` applied to all 7 panels (Effects, ModMatrix, Sampler, Seq, Arp, ProSeq, DrumSeq) |
| UI-7 | **COMPLETE** — SynthPanel, GranularPanel, MacroPanel neumorphic section cards |
| WavetableEditor | Visual wavetable editor: draw tools, formula generators, normalize/fade/reverse/import, per-osc "EDIT WT" button in SynthPanel |
| Layers | 8-layer engine: Synth/Granular/Sampler per layer, level/pan/mute/solo, full MIDI + audio + state wiring, "Layers" tab |
| Track B | Filter LP/HP/BP/Notch; Sub+Noise osc; Unison Chord/Random spread; Layer key/vel/MIDI-ch ranges; Per-layer independent effect chain (LayerEffectChain) |
| Phase 1 | Dynamic 1–8 osc count; 4 new types (Additive/PhaseDist/Analog/Digital); per-osc wave shaping + self-osc; 2 new mod targets (OscShapeAmount, OscPhaseDistAmount) |
| Phase 2 | Sampler: per-zone `tuning` (±24 st) + `speed` (0.25–4×) controls; Lo/Hi Key + Lo/Hi Vel range editing UI; Auto Map button (distributes zones evenly across 0–127) |

## Next Steps

### Gap-Fill Implementation (Current Focus)
**Plan:** `AI_GAP_FILL_PLAN.md`
**Phases 0, 1, and 2 complete.** Next: Phase 3 (Sequencer upgrades) or Phase 5 (Modulation Upgrades — unlimited LFOs).
**Priority order (remaining):**
1. ~~Phase 0: Verify ModulationMatrix/SamplerEngine wiring~~ ✅
2. ~~Phase 1: Core synthesis & oscillator upgrades~~ ✅
3. ~~Phase 2: Sampler enhancements (tune/speed/key-vel ranges/auto-map)~~ ✅
4. Phase 3: Sequencer upgrades (polyrhythm, probability, smart chord)
4. Phase 3: Sequencer upgrades (polyrhythm, probability, smart chord)
5. Phase 4-7: Audio outputs, modulation, UI, effects, additional features

**Remaining clarifications (for later phases):**
3. Multi-output: osc-level, voice-level, or layer-level?
4. 1000+ presets: programmatic or curated?
5. Standalone mode: separate executable or audio effect?

### Cyberpunk UI
Complete — `CyberpunkTheme` renamed to `CyberpunkTheme`, all panels updated, neumorphic section cards applied.

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
- `CyberpunkTheme` (`Source/CyberpunkTheme.h/.cpp`) is the global LookAndFeel installed in `PluginEditor`. Dark Forge palette constants (`bgBase`, `bgRaised`, `bgDeep`, `accentBlue`, etc.) are `static const` members — include `CyberpunkTheme.h` to access them from panels.
- `CyberpunkTheme::drawNeumorphicRect()` is `public static` — callable from panel paint() for section card borders without subclassing.
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
- `PresetBrowserPanel` (`Source/Presets/PresetBrowserPanel.h/.cpp`): inherits `juce::ListBoxModel` + `Button::Listener` + `ComboBox::Listener`. Uses `juce::ListBox presetList`. Shown as 160px collapsible panel above tabs in `PluginEditor` (toggled by "Presets" button in header). `refresh()` calls `presetList.updateContent()`. Phase 4 redesign: keep all logic, only change `paint()`, `resized()`, `paintListBoxItem()`, and visual styling.
- `WavetableEditor` (`Source/Synth/WavetableEditor.h/.cpp`): takes `WavetableOscillator&` (voice 0 master). `onWavetableChanged` callback must be set by caller to `synthEngine.distributeWavetable(oscIndex)`. Shown as full-panel overlay inside SynthPanel; toggled by "EDIT WT" button per osc strip. FFT button is a placeholder (calls normalizeFrame).
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
