# MultiverseUltimate — AI STATE

## Completed

### Foundation
- Full tabbed UI: Synth, Drums, Modulation, Sampler, Sequencer, Pro Seq, Arp, Effects
- Audio routing: MIDI → Sequencer → SynthEngine → Effect Chain → Output
- Audio routing: MIDI → DrumSequencer → Output
- Audio routing: MIDI → SamplerEngine → Output
- ModulationMatrix: LFO phase, computeModulationSums applied to all synth/sampler params
- Melodic Sequencer + Pattern Engine generating MIDI
- DAW transport sync (BPM, playing, PPQ)
- Full state persistence: APVTS + all engine states (XML)
- MIDI handling: note on/off, pitch bend, sustain CC64, sostenuto CC66, mod wheel CC1→filter, all-notes-off CC123
- MIDI Learn: CC/pitch-bend/channel-pressure → any APVTS param, orange "L" badge, mappings persist
- Undo/Redo: Cmd+Z / Cmd+Shift+Z; clears on preset load
- FM Operator UI: 4 operators with ADSR/ratio/level/feedback, algorithm selector 1–8
- Fixed MIDI learn silent cancellation: handleMidiForLearn no longer calls stopMidiLearn() on note/clock/sysex messages

### Phase 3.1 — Filter Oversampling
- OversamplingMode enum (Off/2x/4x/Auto); Auto activates 2× above 5kHz
- "OS" ComboBox in SynthPanel (Classic mode only)

### Phase 3.2 — Reverb Quality
- Pre-delay 0–200ms circular buffer
- LF damping one-pole highpass on wet signal only
- Stereo width via params.width + processStereo()
- Freeze mode (infinite reverb)
- Second reverb row in EffectsPanel (PreDelay, LFDamp, Width, Freeze)

### Phase 3.3 — New Effects + Chain Ordering
- **Chorus** (Rate/Depth/Mix): LFO-modulated delay line, stereo L/R instances
- **Distortion** (Drive/Tone/Mix): tanh soft-clip + one-pole LP tone filter
- **3-Band EQ** (Low/Mid/High ±12dB): Audio EQ Cookbook biquads — low shelf 250Hz, peak 1kHz, high shelf 4kHz
- **Compressor** (Threshold/Ratio/Attack/Release/Makeup): dB-domain feed-forward envelope follower
- **Effect chain ordering**: atomic uint32 packs 6 nibbles; drag-to-reorder strip; order persists in preset state
- EffectsPanel: 2-column layout (L: Chorus/Distortion/EQ, R: Compressor/Delay/Reverb)

### Phase 4.1 — SynthPanel MIDI Learn
- Replaced juce::Slider with MidiLearnSlider in SynthPanel; .init() calls for all sliders

### Phase 4.3 — Tooltips
- juce::TooltipWindow in PluginEditor (700ms delay)
- setTooltip() on all controls across all 8 panels
- "?" help button in header

### Phase 5.1 — Preset Browser + Banks
- PresetManager: saveState/loadState/deletePreset/scanPresetsDirectory
- Presets saved to ~/Library/Audio/Presets/MultiphaseAudio/MultiverseUltimate/*.mvpreset
- PresetBrowserPanel: name TextEditor + Save, ListBox + Load/Delete, double-click to load
- PluginEditor: "Presets" toggle in header; 160px collapsible browser above tabs
- Factory/User banks, category subfolders (Init/Bass/Lead/Pad/Drums/FX), bank selector dropdown
- Import/Export buttons using FileChooser (launchAsync API)

### Phase 3 (Wavetable) — 3 Oscillators + Wavetable File Loading
- Voice class: 3 oscillators per voice (OscState: type/classicOsc/wavetableOsc/level/detune)
- WavetableOscillator: 2048-sample table, 4 standard waves, linear interpolation, wave position scanning
- SynthEngine: OscSettings[3] array, 15 APVTS params (type/level/detune/waveform/wavePos per osc)
- SynthPanel UI: 3 oscillator strips, type selector, level/detune/waveform/wavePos controls
- "LOAD WT" button per oscillator strip (Wavetable mode only); FileChooser for .wav/.aif/.aiff
- Wavetable file path persisted in preset XML; distributed to all 16 voices on load
- Unison: stacked voices with detune spread and stereo width

### Phase B — Macro Controls (2026-05-01)
- MacroManager: 8 macros, CriticalSection-protected targets array, getState/setState XML persistence
- 8 APVTS params `macro1`–`macro8` (Float 0→1), fully DAW-automatable
- MacroPanel: new "Macros" tab, 4×2 grid of rotary knobs, double-click name labels, assignment count buttons
- Assignment applied on message thread at 30 Hz (setValueNotifyingHost on target params)
- Right-click any NeuKnob/MidiLearnSlider → "Assign to Macro" submenu → tick-marked menu, toggle to unassign
- Cyan "M" badge on assigned sliders (top-left); orange "L" badge for MIDI-learned (top-right)
- MacroPanel assignment list popup: shows assigned paramIDs with per-target "Remove" submenu
- State persistence: MacroManager XML block in getStateInformation/setStateInformation

### Phase A — Voice Modes (2026-05-01)
- VoiceMode enum (Poly/Mono/Legato) in SynthEngine
- Mono note stack (int monoNoteStack[16]) — held-note tracking, no heap alloc, cleared in allNotesOff()
- Portamento: per-sample glide in processBuffer using exp2 semitone interpolation (pitch-linear, no warp)
- Mono: always retriggers envelope; returns to last held key on release (retriggers on return)
- Legato: no retrigger when gate open; smooth pitch update only; no retrigger on return to held key
- "Porta Always" toggle: glide even on non-overlapping notes
- 3 new APVTS params: voiceMode (Choice), portamento (Float 0–2s, skew 0.4), portaAlways (Bool)
- UI: VOICE ComboBox + PORTA horizontal slider + "Always" toggle in SynthPanel header row (zero height cost)

### Granular Engine (2026-05-01)
- GranularEngine: standalone top-level engine, member of PluginProcessor alongside SynthEngine/SamplerEngine
- 16 voices × 32 grains/voice = 512 grain objects pre-allocated (no heap alloc in processBlock)
- GrainState: POD struct (readPos, readSpeed, durationSamples, phase, phaseInc, pan, reverse)
- GranularVoice: per-voice ADSR + grain pool + spawn timer; processBlock accumulates L/R grain output
- GranularEngine: source buffer (pre-allocated), built-in 2s sine-sweep default, file loading via AudioFormatManager + LagrangeInterpolator resampling, stereo normalization
- 12 APVTS params: granularPosition, granularGrainSize, granularSpray, granularDensity, granularPitchScatter, granularEnvShape (Choice), granularReverse (Bool), granularStereoSpread, granularAttack/Decay/Sustain/Release
- 4 grain envelope shapes: Gaussian, Hann, Trapezoid, Triangle
- Pitch: readSpeed = pitchRatio * exp2(scatter/12), pitchRatio = exp2((midiNote-60)/12)
- GranularPanel: new "Granular" tab — LOAD GR button, 6 grain control knobs + env shape combo + reverse toggle + ADSR strip; all sliders are NeuKnob
- State persistence: GranularEngine XML block (source file path) in getStateInformation/setStateInformation
- MIDI routing: noteOn/noteOff/allNotesOff wired in processBlock alongside synth/sampler

### Feature 2 + 5 — New Modulation Sources & Granular Targets (2026-05-01)
- **Velocity**: wired to `ModSourceType::Velocity` on each noteOn (0→1, most-recent note)
- **Note Number**: wired to `ModSourceType::NoteNumber` on each noteOn (note/127)
- **Random/S&H**: wired to `ModSourceType::Random` on each noteOn (new random per note)
- **Envelope Follower**: new `ModSourceType::EnvelopeFollower` — tracks peak amplitude of output buffer, 300ms decay, pushed to matrix each block
- **Granular targets**: 5 new `ModTargetType` values — `GranularPosition`, `GranularDensity` (×32 scale), `GranularGrainSize` (×0.25), `GranularSpray`, `GranularPitchScatter` (×12 semitones)
- `MAX_MOD_TARGETS` raised from 16 → 24 to accommodate new targets
- Modulation Matrix panel source/target dropdowns extended to show all new entries

### MPE Support (2026-05-03)
- `mpeEnabled` APVTS Bool param; "MPE" ToggleButton in SynthPanel header row
- Lower Zone: ch 1 = master (global pitch bend ±2 st, pedals, mod wheel); ch 2–15 = member (per-note)
- Per-note pitch bend: ±48 semitones (MPE standard), additive with master bend
- Per-note pressure (channel aftertouch on member ch) → `ModSourceType::MPEPressure` mod source
- Per-note slide CC74 (member ch, neutral 63) → `ModSourceType::MPESlide` mod source (-1..+1)
- `VoiceInfo::midiChannel` tracks which MIDI channel owns each voice
- `MpeChannelState[16]` per-channel pitch/pressure/slide in SynthEngine
- Channel state reset on noteOn (prevents stale bend bleed on channel reuse)
- `allNotesOff()` resets all MPE channel states
- ModulationMatrixPanel source dropdown extended to show MPE Pressure + MPE Slide

### UI Redesign — Phase 1: CyberpunkTheme LookAndFeel (2026-05-03)
- `Source/CyberpunkTheme.h/.cpp` — LookAndFeel_V4 subclass, Dark Forge design system
- Overrides: drawRotarySlider (neumorphic knob, 270° arc, accent glow tip, pointer line, LED dot),
  drawLinearSlider (inset track, gradient fill, neumorphic thumb),
  drawToggleButton (LED pill), drawButtonBackground/Text, drawComboBox,
  drawTabButton / drawTabAreaBehindFrontButton, drawLabel,
  drawPopupMenuBackground / drawPopupMenuItem, drawScrollbar
- Full color palette set via setColour() in constructor (all standard component IDs)
- Installed in PluginEditor (setLookAndFeel / cleared in destructor)
- Tab content colors updated to bgBase (#171720) in setupTabs()

### UI Redesign — Phase 2: NeuKnob (2026-05-03)
- `Source/NeuKnob.h/.cpp` — extends MidiLearnSlider; drop-in replacement
- Value pill: rounded rect centered above knob (bgDeep fill, accentBlue border, 9px text),
  appears while mouse is over or button down, rotary style only
- Amber arc: arcTimer (10 Hz) watches isMacroAssigned(); sets rotarySliderFillColourId to
  accentAmber on component when assigned, removes override when unassigned
- CyberpunkTheme::drawRotarySlider now uses slider.findColour(rotarySliderFillColourId)
  instead of hardcoded accentBlue — enabling per-component arc color
- Deployed to: SynthPanel, EffectsPanel, GranularPanel, ModulationMatrixPanel, SamplerPanel

### UI Redesign — Phase 3: SynthDisplay (2026-05-03)
- `Source/Synth/SynthDisplay.h/.cpp` — real-time oscilloscope + FFT spectrum analyzer
- Lock-free ring buffer: `PluginProcessor::displayFifo` (AbstractFifo, 4096 samples) + `displayFifoBuffer[4096]`
- Audio thread pushes post-effect stereo mono-mix via `pushDisplaySamples()` (no allocation, no locks)
- UI thread pulls via `pullDisplaySamples()` at 30 Hz (juce::Timer)
- Oscilloscope: left 45% of display — 512-sample ring view, accentBlue 1.5px stroke + glow layer
- FFT spectrum: right 55% — 1024-point FFT (order 10), Hann window, log-frequency x-axis (20Hz–22kHz), accentPurple→accentBlue gradient bars, peak-hold dots
- Fast attack / slow release smoothing (fftMag[] *= 0.92 on decay), peak hold decays at 0.998/frame
- Inset neumorphic frame: dark offset shadow + light rim, #0D0D18 background, 8% white grid
- SynthPanel: LFODisplay hidden; SynthDisplay shown (90px height) in Classic mode, hidden in FM mode
- Both SynthDisplay.h/.cpp added to Multiverse.jucer (Gg0022/Gg0023); Projucer --resave run; build OK

### UI Redesign — Phase 4: Preset Browser Redesign (2026-05-03)
- `Source/Presets/PresetBrowserPanel.h/.cpp` — fully redesigned for Dark Forge design system
- 220px slide-in panel, #171720 background
- Search bar: neumorphic inset input with real-time filtering (`TextEditor::Listener`)
- Category: horizontal pill tabs (All/Init/Bass/Lead/Pad/Drums/FX), accent on active (radio group)
- Filtered preset indices: `filteredPresetIndices` array rebuilt on search/category change
- Preset list: neumorphic raised cards (32px row height); active = accent border + glow + bright text
- `rebuildFilter()` filters by category + search text; `setActiveCategory()` for pill toggle
- PluginEditor: preset browser height updated from 160px → 220px
- Build verified OK

## Completed (Phase 5 — UI Redesign Phases 1–5)
- Phase 5: Remaining UI Panels — Dark Forge redesign **COMPLETE**
  - EffectsPanel: paint(), EffectChainStrip, labels → CyberpunkTheme palette
  - ModulationMatrixPanel: paint(), title, row colours → CyberpunkTheme palette
  - SamplerPanel: paint(), ZoneListModel, waveform colours → CyberpunkTheme palette
  - SequencerPanel: paint(), StepButton, labels → CyberpunkTheme palette
  - DrumSequencerPanel: paint(), DrumStepButton, TrackRow, labels/buttons → CyberpunkTheme palette
  - ArpeggiatorPanel: paint(), ArpStepButton, labels → CyberpunkTheme palette (`.h` + `.cpp`)
  - ProSequencerPanel: paint(), ProStepButton, labels → CyberpunkTheme palette
  - All hardcoded `juce::Colour (0xff...)` replaced with `CyberpunkTheme::bgBase/bgRaised/accentBlue/textSecondary/etc.`
  - Build verified ✅ (VST3 + AU both build and install successfully)

## Broken
- None

## Completed (Phase 6 — Section Card System)

**Phase 6 COMPLETE** — Neumorphic section cards applied across all panels using `CyberpunkTheme::drawNeumorphicRect()`:

- **EffectsPanel** — 6 neumorphic cards: Chorus, Distortion, EQ, Compressor, Delay, Reverb sections
- **ModulationMatrixPanel** — each modulation row wrapped in a neumorphic card
- **SamplerPanel** — 3 neumorphic cards: drop zone, zone list, controls section
- **SequencerPanel** — 4 neumorphic cards: transport, pattern slots, step grid, export button
- **ArpeggiatorPanel** — 3 neumorphic cards: controls, step grid (2 rows), step editor
- **ProSequencerPanel** — 4 neumorphic cards: transport, lane/mode controls, step grid, step editor
- **DrumSequencerPanel** — 4 neumorphic cards: transport, swing/quant/chain, pattern row, step grid + track rows

**Build verified:** VST3 + AU both build and install successfully ✅

---

## Completed (Phase 7 — Polish & Details)

**Phase 7 COMPLETE** — Neumorphic section cards applied to remaining panels:

- **SynthPanel** — 5 neumorphic cards: OSC, UNISON, FILTER, ENV (Classic mode), FM OPERATORS (FM mode), VOICE MODE (header strip)
- **GranularPanel** — 3 neumorphic cards: SOURCE (file load), GRAIN (grain controls), VOICE ENVELOPE (ADSR)
- **MacroPanel** — 1 neumorphic card: MACRO CONTROLS (all 8 macro knobs)
- All cards use `CyberpunkTheme::drawNeumorphicRect()` with consistent `cr=8.0f, offset=3.0f`
- Section titles drawn via `drawSection()` helper (SynthPanel) or inline (GranularPanel, MacroPanel)

**Build verified:** VST3 + AU both build and install successfully ✅

---

### WavetableEditor (2026-05-03)
- `Source/Synth/WavetableEditor.h/.cpp` — full visual wavetable editor component
- Draw tools: Pencil (freehand), Line (click-drag), Curve (same as pencil, future: quadratic)
- Frame selector: ComboBox selects which wavetable frame to edit
- Process buttons: Normalize, Fade (1→0), Reverse, FFT (placeholder = normalize), Clear
- Formula generators: Sin, Saw, Square, Tri — fills selected frame with standard waveform
- Import: single-cycle WAV (loads frame 0), multi-cycle WAV (splits into up to 4 frames)
- `onWavetableChanged` callback — called after every edit; SynthPanel wires it to `SynthEngine::distributeWavetable(oscIndex)`
- `SynthEngine::distributeWavetable(oscIndex)` — copies voice 0 wavetable to all 16 voices
- `SynthEngine::getWavetableOscillator(oscIndex)` — returns voice 0's WavetableOsc for editing
- `Voice::getWavetableOsc(oscIndex)` — public accessor added to Voice
- **"EDIT WT"** button added to each osc strip in SynthPanel (visible in Wavetable mode, next to "LOAD WT")
- Clicking "EDIT WT" shows the editor as a full-panel overlay; clicking again hides it
- Fixed brace syntax error in `WavetableOscillator.cpp` (`normalizeFrame` / stray `}`)

### Layers System (2026-05-03)
- `Source/Layers/LayerEngine.h/.cpp` — wraps Synth/Granular/Sampler engine per layer with level/pan/mute/solo
- `Source/Layers/LayerManager.h/.cpp` — manages 8 LayerEngines, processes/mixes, handles MIDI routing + solo logic
- `Source/Layers/LayersPanel.h/.cpp` — UI: 8 rows, each with engine type selector, level/pan sliders, mute/solo, preset button, L/R meter labels
- **"Layers" tab** added to PluginEditor (10th tab)
- `LayerManager` wired into `PluginProcessor`:
  - `prepare()` in `prepareToPlay`
  - MIDI: `noteOn`, `noteOff`, `allNotesOff` (both CC123 and stop paths)
  - Audio: `processBlock` → mixed into main signal pre-effects (alongside Synth/Granular/Sampler)
  - State: `getState`/`setState` in `getStateInformation`/`setStateInformation`
- State persistence uses `juce::ValueTree` throughout (GranularEngine + SamplerEngine sub-state per layer)

**Build verified:** VST3 + AU both build and install successfully ✅

---

### Track B — Feature Gaps (2026-05-03)

**All 7 features shipped:**

- **Filter types LP/HP/BP/Notch** — `Filter::FilterType` enum + `setFilterType`; SVF for LP/HP/BP, IIR biquad for Notch; `filterType` APVTS param; TYPE ComboBox in FILTER card (SynthPanel)
- **Sub oscillator** — dedicated `SubOscState` per voice, 1 oct below base freq, Sine/Square waveform, level control; `subOscEnable/Level/Wave` APVTS params; SUB/NOISE card in SynthPanel
- **Noise oscillator** — `NoiseOscState` per voice, one-pole LP color filter (200–20kHz), level control; `noiseOscEnable/Level/Color` APVTS params; in SUB/NOISE card
- **Unison spread modes** — `UnisonSpreadMode {Stacked, Chord, Random}` in SynthEngine; Chord uses fixed major chord intervals [0,4,7,12,16,19,24,28] semitones; Random uses PRNG per noteOn; `unisonSpreadMode` APVTS param; SPREAD ComboBox in UNISON card
- **Layer key ranges** — `loNote/hiNote` (0-127) per LayerEngine; filter in `noteOn`; sliders in LayersPanel row
- **Layer velocity ranges** — `loVel/hiVel` (0-127) per LayerEngine; filter in `noteOn`; sliders in LayersPanel row
- **Layer MIDI channel filter** — `midiChannelFilter` (0=all, 1-16) per LayerEngine; filter in `noteOn`; `LayerManager::noteOn` now takes midiChannel; ComboBox in LayersPanel row
- **Per-layer independent effect chain** — `LayerEffectChain.h/.cpp` (new): owns Chorus/Distortion/EQ/Compressor/Delay[2]/Reverb per layer, per-effect enable+mix, XML state; `LayerEngine` runs effectChain after pan in processBlock; "FX" button in LayersPanel row opens CallOutBox with 6 effect toggles + mix sliders; state persists in preset

**Build verified:** VST3 + AU both build and install successfully ✅

---

## Completed (Phase 0 — Existing Functionality Verification) (2026-05-04)

**All three verification items confirmed — no fixes required:**

1. **ModulationMatrix is fully functional:**
   - `advanceLFOs(numSamples)` called at `PluginProcessor.cpp:1062` — LFO phases advanced per block, sine values stored to `sourceValues[]`
   - `computeModulationSums()` called at lines 887 and 919 — results applied to all 20 targets (osc pitch/level/wave, filter cutoff/resonance, amp volume/pan, 4× LFO rates, 3× effect params, effect mix, 5× granular params)
   - `ModulationMatrix` owned by `PluginProcessor` (not UI thread), shared via `getModulationMatrix()` — thread-safe via `std::atomic<float>` for source values and `juce::CriticalSection` for connection vector

2. **SamplerEngine/DrumSequencer audio output wiring confirmed:**
   - `drumSequencer.process(buffer, numSamples)` at line 636
   - `samplerEngine.processBuffer(samplerBuffer, numSamples)` at line 985
   - `granularEngine.processBuffer(granularBuffer, numSamples)` at line 986
   - `layerManager.processBlock(layerBuffer, numSamples)` at line 987
   - All four engines mixed into output buffer at lines 1013–1017

3. **Per-step velocity works in both Sequencer and DrumSequencer:**
   - `Sequencer::Step::velocity` field, `setStepVelocity()`, used in `triggerNoteOn()` → `midiVel = velocity * 127`
   - `DrumSequencer::DrumStep::velocity` field, `setStepVelocity()`, used in `triggerTrack()` → `voice.trigger(track, velocity)`

**SPEC.md §2.6/§2.7 updated** to reflect verified state (was outdated).

---

## Completed (Track A — Cyberpunk UI)

**Track A COMPLETE** — `CyberpunkTheme` renamed to `CyberpunkTheme` (2026-05-04):
- `CyberpunkTheme.h/.cpp` created (identical palette to Figma SVGs — `neonCyan`, `neonPink`, `neonPurple` on `bgVoid`)
- `PluginEditor.h`: `mvTheme` type changed from `CyberpunkTheme` → `CyberpunkTheme`
- `PluginEditor.cpp`: tabs already reference `CyberpunkTheme::bgBase`
- `MacroPanel.h`: include updated to `CyberpunkTheme.h`
- `CyberpunkTheme.h/.cpp` deleted
- Xcode project (`project.pbxproj`): `CyberpunkTheme` references replaced with `CyberpunkTheme`
- Build verified: VST3 + AU both build and install successfully ✅

**Note:** Figma SVGs in `Figmacomponents/` already matched the CyberpunkTheme palette exactly — the rename to `CyberpunkTheme` is complete with no visual changes.

---

## Brief Comparison Report (2026-05-04)

Compared existing plugin features against `MULTIVERSE SYNTH BREIF.txt`:

**Fully Implemented:**
- Subtractive, Wavetable, Granular, FM synthesis
- Sampler (zone-based, loops, crossfade)
- Drum Sequencer (8 tracks, 16 steps, 32 voices)
- Modulation Matrix (LFOs, envelopes, drag-drop routing)
- Preset Browser (Factory/User banks, search, categories)
- Cyberpunk UI (Dark Forge neumorphic theme)
- MPE support, Macro controls, Layers system
- Filter types (LP/HP/BP/Notch), Sub/Noise osc, Unison spread modes

**Missing:**
- Additive, Phase Distortion, Analog Modeling, Digital synthesis engines
- Unlimited oscillators (fixed 3/voice), wave shaping, self-oscillation feedback
- Unlimited audio outputs, individual outs per osc/voice
- Sampler: time-stretch, MIDI drag/drop, multi-sampling (velocity/key layers)
- Sequencer: polyrhythm/polymeter, probability triggers, smart chord tracking, audio-rate step mod
- Modulation: unlimited LFOs/envelopes, drawable LFO shapes, DAW-synced LFOs
- UI: resizable/DPI scaling, built-in keyboard, 1000+ presets
- Effects: return/aux sends, standalone mode
- Additional: quick randomization, CPU voice limiting, zero-latency mode, audio effect mode, tuner/metronome

**Full gap analysis:** See `AI_GAP_FILL_PLAN.md`

---

## Completed (Phase 1 — Core Synthesis & Oscillator Upgrades) (2026-05-04)

**All 4 sub-tasks shipped:**

- **1.1 Dynamic oscillator count** — pre-allocated 8 slots per-voice; `oscCount` APVTS Choice param (1–8, default 3); `+ OSC` / `- OSC` buttons in SynthPanel; `SynthEngine::setOscCount()` propagates to all voices; 2-row layout (≤4 strips = 1 row, 5–8 = 2 rows); inactive slots have level 0 and are skipped in `Voice::process()`
- **1.2 New oscillator types** — `OscillatorType` enum extended to 6 values: Classic / Wavetable / Additive / PhaseDist / Analog / Digital; DSP in `Voice.cpp`: Additive = 8-harmonic Fourier series (1/n falloff), PhaseDist = sinusoidal phase bending, Analog = Classic + LCG micro-drift, Digital = Classic + 16-level bit reduction; all render real-time safe (no alloc)
- **1.3 Per-oscillator wave shaping + self-oscillation** — `OscShapeType` enum {Off, Drive, Fold, Clip}; Drive = normalized tanh(x·drive), Fold = wavefold with threshold, Clip = hard limit normalised; self-osc = additive feedback of prior sample; `shapeType/shapeAmt/selfOscFeedback/phaseDistAmount` fields in `OscState`; UI: SHAPE ComboBox + Shape/Self/PD knobs per strip (shape amt hidden when Off; PD Amt only visible for PhaseDist type)
- **1.4 ModulationMatrix targets** — added `OscShapeAmount` + `OscPhaseDistAmount` to `ModTargetType`; `MAX_MOD_TARGETS` raised 24→26; wired in `PluginProcessor::processBlock` (osc 0); target dropdown in ModulationMatrixPanel extended

**Architecture:**
- `APVTS`: 8 × 9 osc params (type/level/detune/waveform/wavePos/shapeType/shapeAmt/selfOsc/phaseDist) + `oscCount` = 73 new params (fully backward-compatible: old presets default oscs 4–8 to level 0)
- State persistence: `oscCount` + 8 osc nodes with all new properties saved/restored in XML
- Build verified: VST3 + AU both build and install successfully ✅

---

---

## Completed (Phase 2 — Sampler Enhancements) (2026-05-04)

**All sub-tasks shipped:**

- **2.1 Drag-and-drop** — was already complete (FileDragAndDropTarget in SamplerPanel); verified ✅
- **2.2 Tune + Speed per zone** — `SamplerZone::tuning` (float ±24 semitones, 0.1 resolution) + `speed` (float 0.25–4.0); folded into `playbackRate` in `MvSamplerVoice::noteOn()` as `pitchRatio * (fileSR/sr) * speed` with tuning added to the semitone offset; both persisted in getState/setState XML
- **2.3 Zone range editing UI** — Lo Key / Hi Key combos + Lo Vel / Hi Vel sliders added to SamplerPanel controls section; all wired to SamplerZone fields (already existed in struct, just not editable)
- **2.4 Auto Map button** — distributes n zones evenly across 0–127 key range (128/n notes per zone, last zone takes remainder); defaults to full 0–127 per zone on load; button sits alongside Clear All
- **2.4 MIDI drag/sync** — deferred to Phase 3 (Sequencer upgrades)

**Build verified:** VST3 builds successfully ✅

---

---

## Completed (Phase 3 — Sequencer & Arpeggiator Upgrades) (2026-05-04)

**All 5 sub-tasks shipped (3.6 deferred):**

- **3.1 MIDI drag-and-drop import** — `SequencerPanel` inherits `FileDragAndDropTarget`; `.mid`/`.midi` files dropped onto step grid are parsed via `juce::MidiFile`; note-on events quantized to 16-step grid; cyan border glow while dragging over
- **3.2 Smart chord tracking** — active step notes collected every 50ms, pitch classes matched against 10-chord table (major/minor/maj7/dom7/m7/dim/aug/m7b5/sus4/sus2); detected chord name shown in `chordLabel` in the export row
- **3.3 Per-pattern step length (polyrhythm)** — `SeqPattern::stepLengthMultiplier` (default 1.0 = 16th); `updateSamplesPerStep()` multiplied by it; STEP ComboBox in transport row: 32nd/16th/8th/Qtr/8T (8th triplet); per-pattern, saved/loaded in preset XML; `loadPattern()` recomputes timing
- **3.4 Per-step probability** — `Step::probability` (0–1, default 1.0); skipped in `process()` via xorshift RNG; right-click step → Probability sub-menu (100%/75%/50%/25%/0%); coloured dot indicator on step button (blue/amber/red); persisted in state XML
- **3.5 Sequencer Step mod source** — `ModSourceType::SequencerStep` (slot 14); current step normalized 0→1 set in `PluginProcessor::processBlock()` after `sequencer.process()`; visible in ModulationMatrix source dropdown as "Seq Step"

**Build verified:** VST3 + AU both build and install successfully ✅

---

## Completed (Phase 4 — Audio Outputs & Drum Per-Track FX) (2026-05-04)

**All 3 sub-tasks shipped:**

- **4.1 + 4.2 — Multi-output buses (17 total)**: Bus 0 = main mix (always active); Buses 1–8 = Layer 1–8 individual outs (disabled by default); Buses 9–16 = Drum Track 1–8 individual outs (disabled by default)
  - `PluginProcessor` constructor declares all 17 buses via chained `.withOutput()` calls (Logic Pro / all-DAW compatible — fixed layout)
  - `isBusesLayoutSupported()` accepts any config where bus 0 is stereo and all others are stereo-or-disabled
  - **Layer routing**: `LayerEngine::outputBusIndex` (0=main, 1–8); `LayerManager::processBlock` skips individual-bus layers; `PluginProcessor::processBlock` routes them via `getBusBuffer()` after the effects chain; state persisted in preset XML
  - **Drum routing**: `DrumSequencer::trackOutputBus[t]` (0=main, 9–16); per-track buffers filled in `process()`; bus-0 tracks mixed to main; PluginProcessor routes non-zero-bus tracks via `getBusBuffer()`
  - **LayersPanel**: "BUS" ComboBox per row (Main / Bus 1–8); wired to `LayerEngine::setOutputBusIndex()`
  - **DrumSequencerPanel**: "BUS" ComboBox per track row (Main / Out 1–8); wired to `DrumSequencer::setTrackOutputBus()`

- **4.3 — Drum Sequencer per-track FX**: `LayerEffectChain trackFX[8]` in DrumSequencer (Chorus/Distortion/EQ/Compressor/Delay/Reverb per track); applied after per-track voice accumulation, before bus routing; "FX" button in each drum track row opens same CallOutBox as layer FX; state persisted in preset XML alongside trackOutputBus

**Architecture:**
- `DrumSequencer::process()` refactored: voices now accumulate into `trackBufs[DRUM_TRACK_COUNT]` (pre-allocated in `prepare()`); FX applied per track; bus-0 tracks mixed to main; non-zero-bus tracks available via `getTrackBuffer(t)`
- `LayerManager::processBlock` skips `outputBusIndex > 0` layers (processor handles those)
- All new state (outputBusIndex per layer, trackOutputBus + trackFX per drum track) persisted in preset XML

**Build verified:** VST3 + AU both build and install successfully ✅

---

## Next Session

**Phase 4 COMPLETE** — 17-bus multi-out, layer routing, drum routing, per-track drum FX all shipped.

**Ready for Phase 5** (Modulation Upgrades — unlimited LFOs, drawable shapes, DAW sync) per `AI_GAP_FILL_PLAN.md`.

**Competitive brief reminder:** Goal is to match/surpass Serum 2, Nexus 5, Avenger 2, Diva, Zebra 3.

**Remaining questions (for later phases):**
4. 1000+ presets: programmatic generation or curated?
5. Standalone mode: separate executable or audio effect mode?
6. 3.6 (Drag MIDI to audio / offline render) — deferred, complex feature.
