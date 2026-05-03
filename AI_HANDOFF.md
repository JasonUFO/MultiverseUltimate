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
| SynthEngine | 16-voice classic (3 osc/voice: Classic or Wavetable) / 8-voice FM |
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

- SynthEngine (16-voice classic with 3 osc/voice + FM 8-voice), DrumSequencer, SamplerEngine — all produce audio ✅
- GranularEngine (16-voice × 32 grains, 4 env shapes, pitch scatter, spray, density, stereo spread, file loading) ✅
- 3 oscillators per voice: Classic (math-based) or Wavetable (2048-sample table, 4 waves, position scan) ✅
- Voice modes: Poly / Mono / Legato with portamento (per-sample semitone-space glide) ✅
- All 6 effects with full parameter control and MIDI Learn ✅
- Effect chain reordering — drag-to-reorder strip, order persists in presets ✅
- ModulationMatrix — LFO → pitch/cutoff/volume/effects ✅
- Melodic Sequencer, DAW transport sync ✅
- Full state persistence (XML) including 3 oscillators + effect chain order + granular source path ✅
- MIDI Learn on all effect panel knobs ✅
- Undo/Redo (Cmd+Z / Cmd+Shift+Z) ✅
- Filter oversampling (Off/2x/4x/Auto) ✅
- Preset system (XML) with Factory/User banks ✅
- Dark Forge UI theme (`MultiverseTheme`) — neumorphic knobs, sliders, buttons, tabs, menus ✅
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
| UI-1 | MultiverseTheme LookAndFeel — Dark Forge design system; neumorphic knobs, sliders, buttons, tabs, menus |
| UI-2 | NeuKnob — extends MidiLearnSlider; value pill on hover/drag; amber arc when macro-assigned |
| UI-3 | SynthDisplay — real-time oscilloscope + FFT spectrum; lock-free FIFO in PluginProcessor |
| UI-4 | PresetBrowserPanel — Dark Forge redesign, 220px, search bar, category pills, neumorphic cards |
| UI-5 | **COMPLETE** — EffectsPanel, ModulationMatrixPanel, SamplerPanel, SequencerPanel, DrumSequencerPanel, ArpeggiatorPanel, ProSequencerPanel all updated to Dark Forge palette |
| UI-6 | **COMPLETE** — Section card system: neumorphic cards via `drawNeumorphicRect()` applied to all 7 panels (Effects, ModMatrix, Sampler, Seq, Arp, ProSeq, DrumSeq) |
| UI-7 | **COMPLETE** — SynthPanel, GranularPanel, MacroPanel neumorphic section cards |
| WavetableEditor | Visual wavetable editor: draw tools, formula generators, normalize/fade/reverse/import, per-osc "EDIT WT" button in SynthPanel |
| Layers | 8-layer engine: Synth/Granular/Sampler per layer, level/pan/mute/solo, full MIDI + audio + state wiring, "Layers" tab |

## Next Steps

### Cyberpunk UI (Figma-First Approach)
**Plan saved:** `AI_CYBERPUNK_PLAN.md`  
**Scope:** Full UI replacement (MultiverseTheme → CyberpunkTheme)  
**Style:** Neumorphic + Cyberpunk (neon cyan/pink/purple on deep void backgrounds)  
**Figma assets:** `Figmacomponents/` — 10 panel SVGs + 3 component SVGs + `FIGMA_BUILD_GUIDE.md`  
**Status:** Awaiting visual sign-off. Review SVGs → approve → implement C++.

### Feature Gaps vs Target Synths
See `AI_STATE.md` "Track B" for full list. Key items:
- Layer key/velocity ranges (Nexus 5 / Avenger 2 parity)
- Sub oscillator + noise oscillator (Serum 2 parity)
- Filter topology selector LP/HP/BP/Notch (Diva parity)
- Layer per-effects send (Avenger 2 parity)

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
- `MultiverseTheme` (`Source/MultiverseTheme.h/.cpp`) is the global LookAndFeel installed in `PluginEditor`. Dark Forge palette constants (`bgBase`, `bgRaised`, `bgDeep`, `accentBlue`, etc.) are `static const` members — include `MultiverseTheme.h` to access them from panels.
- `MultiverseTheme::drawNeumorphicRect()` is `public static` — callable from panel paint() for section card borders without subclassing.
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
- `NeuKnob` extends `MidiLearnSlider` — use NeuKnob everywhere you'd use MidiLearnSlider; it's a drop-in. Value pill fires from paint() (isMouseOver/isMouseButtonDown check). Amber arc managed by ArcTimer (inner juce::Timer struct, 10 Hz), calls setColour/removeColour only on state change to avoid repaint storms. `MultiverseTheme::drawRotarySlider` uses `slider.findColour(rotarySliderFillColourId)` — component-level setColour overrides the LookAndFeel color.
- `juce::Font::getStringWidthFloat` does not exist in this project's JUCE version — use character-count estimation or `getStringWidth` (int) for pill sizing.
- `SynthDisplay`: lives at `Source/Synth/SynthDisplay.h/.cpp`. Ring buffer: `RING_SIZE=2048`, `ringWritePos` (masked with `& (RING_SIZE-1)`). FFT: `juce::dsp::FFT fft{FFT_ORDER}` (order 10, 1024-point). Audio data arrives via `PluginProcessor::pullDisplaySamples(float*, int)` (public, message-thread safe). `pushDisplaySamples` is private, called at end of processBlock after pan.
- `PresetBrowserPanel` (`Source/Presets/PresetBrowserPanel.h/.cpp`): inherits `juce::ListBoxModel` + `Button::Listener` + `ComboBox::Listener`. Uses `juce::ListBox presetList`. Shown as 160px collapsible panel above tabs in `PluginEditor` (toggled by "Presets" button in header). `refresh()` calls `presetList.updateContent()`. Phase 4 redesign: keep all logic, only change `paint()`, `resized()`, `paintListBoxItem()`, and visual styling.
- `WavetableEditor` (`Source/Synth/WavetableEditor.h/.cpp`): takes `WavetableOscillator&` (voice 0 master). `onWavetableChanged` callback must be set by caller to `synthEngine.distributeWavetable(oscIndex)`. Shown as full-panel overlay inside SynthPanel; toggled by "EDIT WT" button per osc strip. FFT button is a placeholder (calls normalizeFrame).
- `WavetableOscillator` new public API: `setSample(frame,i,v)`, `getSample(frame,i)`, `clearFrame`, `normalizeFrame`, `fadeFrame`, `reverseFrame`, `generateFormula(frame, fn)`, `loadMultiCycleWavetable`, `processFFT` (placeholder), `getTableCount()`, `getTableSize()`.
- `SynthEngine::getWavetableOscillator(oscIndex)` returns `voices[0].voice.getWavetableOsc(oscIndex)` — voice 0 is the edit master. `distributeWavetable(oscIndex)` copies frame data from voice 0 to voices 1–15 via `setSample`.
- `LayerManager` (`Source/Layers/`) manages `std::array<unique_ptr<LayerEngine>, 8>`. Each `LayerEngine` owns its own `SynthEngine`, `GranularEngine`, `SamplerEngine` (all three always allocated; only active engine processes). `LayerManager::processBlock` mixes all non-muted layers (or only soloed layers) into the output buffer. MIDI goes to all active layers. State: `juce::ValueTree` — GranularEngine + SamplerEngine sub-states only (SynthEngine has no standalone state API).
- `LayersPanel` (`Source/Layers/LayersPanel.h/.cpp`): `rows` is `std::array<unique_ptr<LayerRow>, 8>`. `LayerRow` is a struct (not a Component) holding JUCE controls as direct members. Lambdas capture `[this, index, row]` — `row` is a raw pointer to the unique_ptr content, valid for the panel's lifetime.
