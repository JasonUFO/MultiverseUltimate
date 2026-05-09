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

## Completed (Phase 5 — Modulation Upgrades) (2026-05-04)

**Phase 5 COMPLETE** — All 4 sub-tasks shipped:

- **5.1 — 8 LFOs**: `ModSourceType` enum expanded with `LFO5-LFO8` (indices 15-18); `lfoPhase/lfoRate/lfoShape/lfoSHValue` arrays (8 elements) replace the 4 individual private members; `advanceLFOs()` loops over all 8; `setLFORate/getLFORate` now accept indices 0-7; `MAX_MOD_SOURCES` 16→21
- **5.2 — LFO shapes**: `LFOShape` enum {Sine, Triangle, Saw, Square, SampleAndHold}; `setLFOShape/getLFOShape` API; shape-aware value computation per LFO in `advanceLFOs()`; S&H generates new random on phase wrap; `lfo1Shape`–`lfo8Shape` APVTS Choice params
- **5.3 — DAW sync**: `lfo1Sync`–`lfo8Sync` Bool APVTS params; `lfo1SyncDiv`–`lfo8SyncDiv` Choice params (1/32 to 4/1, 8 options); when synced, rate computed from `dawBPM/60 / divisor` in processBlock, overriding the manual rate slider; `static const float lfoSyncDivisors[8]` in processBlock
- **5.4 — Mod envelopes**: `ModSourceType::Envelope2` (19) and `::Envelope3` (20); `juce::ADSR modEnv2, modEnv3` in PluginProcessor; `modEnvXAttack/Decay/Sustain/Release` APVTS Float params; triggered from noteOn/noteOff in processBlock MIDI loop; advanced per block with `getNextSample()`, result pushed to `modulationMatrix.setModulationValue()`
- **UI — LFO Banks section**: `LFORow` struct in `ModulationMatrixPanel` (8 rows above connections); each row: LFO label, rate slider + APVTS attachment, shape combo + APVTS attachment, sync toggle + APVTS attachment, sync-div combo + APVTS attachment; all 4 APVTS params fully DAW-automatable
- **Connection dropdown fix**: source/target combo boxes now loop over `MAX_MOD_SOURCES/MAX_MOD_TARGETS` (was hardcoded to `MPESlide`/`OscPhaseDistAmount`), picking up LFO5-8, Env2/3, SequencerStep, LFO5-8 Rate targets automatically
- `ModTargetType::LFO5Rate–LFO8Rate` added (indices 22-25, filling the 4 spare slots up to MAX_MOD_TARGETS=26); wired in processBlock LFO rate loop

**Build verified:** VST3 builds and installs successfully ✅

---

## Completed (Phase 6 — UI & Presets) (2026-05-04)

**Phase 6 COMPLETE** — 3 sub-tasks shipped:

- **6.1 Resizable UI**: `setResizable(true, true)` + `setResizeLimits(800, 533, 1920, 1280)` in PluginEditor; free drag-resize enabled; **Scale combo** in header (75%/100%/125%/150%) calls `setSize(1200*f, 800*f)` — all child layout uses `getLocalBounds()` so scales naturally
- **6.2 Built-in keyboard**: `juce::MidiKeyboardState keyboardState` (public) in PluginProcessor; `keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true)` at top of processBlock (injects on-screen keyboard MIDI before the MIDI loop); `juce::MidiKeyboardComponent keyboard` in PluginEditor (64px tall strip at bottom, always visible, range C2-C8)
- **7.3 Quick randomization**: "RAND" button in header opens `PopupMenu` with 4 options: "OSC+Filter+Env", "Filter+Env", "LFOs", "Everything"; `randomizeParams(prefixes)` loops all APVTS `RangedAudioParameter`s, skips structural params (masterVolume, mpeEnabled, oscCount, etc.) and sync params; calls `setValueNotifyingHost(rng.nextFloat())` for each matching param

**Build verified:** VST3 builds and installs successfully ✅

---

## Completed (Phase 7 — Effects & Additional Features) (2026-05-04)

**Phase 7 PARTIAL** — 2 sub-tasks shipped:

- **7.4 CPU voice limiting**: `void setVoiceLimit(int)` in SynthEngine; `int voiceLimit = MAX_VOICES` private member; `findFreeVoice()` iterates only `voices[0..voiceLimit-1]`; `findFreeFMVoice()` same with `fmVoices`; `maxVoices` APVTS Choice param ("1/2/4/6/8/10/12/16", default 16=index 7); read once per block before MIDI loop, maps index to `voiceLimitValues[]` array
- **7.7 Metronome**: `metronomeEnabled` Bool + `metronomeVolume` Float APVTS params; `prevDawPpqPos` + `metClickSamplesLeft/SamplePos/Duration/IsDownbeat` in PluginProcessor; beat detection compares `floor(prevDawPpqPos)` vs `floor(dawPpqPos)` each block; click fires 25ms decaying sine (1200Hz downbeat, 900Hz off-beat) added directly to buffer; downbeat detection via `fmod(dawPpqPos, 4.0) < 1.0`
- **7.2 Standalone** — DEFERRED: `buildStandalonePlugin` in .jucer is not recognized by this Projucer version. Must be enabled via Projucer GUI (File Formats tab → check "Standalone Plugin"). The plugin code is already standalone-compatible.

**Build verified:** VST3 + AU both build and install successfully ✅

---

## Completed (Phase 7 — Aux Sends + Tuner) (2026-05-04)

- **7.1 Return/Aux Sends**: Two parallel send paths (Delay + Reverb) added to EffectsPanel:
  - `auxSendDelay` + `auxSendReverb` APVTS Float params (0-1, default 0)
  - `auxDelay` + `auxReverb` DelayEffect/ReverbEffect instances in PluginProcessor (fully prepared/reset)
  - `auxSendBuffer` + `auxWorkBuffer` pre-allocated members (set size in prepareToPlay)
  - processBlock: dry mix captured in `auxSendBuffer` after `layerManager.processBlock` but before effects chain; sends processed post-pan, output added to main buffer
  - Aux effects share same parameters as main chain (time/feedback/room/damp/etc.) but forced to 100% wet
  - EffectsPanel: "SENDS" neumorphic card with `→ Delay` + `→ Reverb` NeuKnobs with MIDI Learn + tooltips
  - Build verified ✅

- **7.7 Tuner UI**: Visual pitch detector overlay in SynthDisplay oscilloscope:
  - FFT peak bin detection (bins 2 to Nyquist/2, skipping DC/low harmonics)
  - Parabolic interpolation for sub-bin frequency accuracy
  - Maps Hz → MIDI note + cents deviation (±50 cents, green < 5 cents, amber otherwise)
  - Drawn as thin strip at bottom of scope area: note name (e.g. "A4"), cents bar, Hz readout
  - `tunerHz/tunerNote/tunerCents` state in SynthDisplay; updated at 30 Hz in timerCallback
  - Build verified ✅

## Completed (Drawable LFO Shapes — 2026-05-05)

- **LFOShape::Custom** added as 6th shape value
- **LFOShapeEditor** (`Source/Synth/LFOShapeEditor.h/.cpp`): pencil + line draw tools, SIN/SAW/SQR/TRI fill buttons, NORM button; 310×160 CallOutBox
- **DRAW button** in each LFO row (ModulationMatrixPanel), enabled only when Custom shape selected, dimmed otherwise
- **Custom table lookup** in `advanceLFOs()`: 256-pt linear interpolation, wrapping
- **State persistence**: 256 floats saved/restored as comma-separated string per LFO in preset XML
- **APVTS**: lfoShape StringArray extended to include "Custom" (index 5 = enum Custom)
- Build verified: VST3 builds and installs successfully ✅

## Completed (Chord/Strum Mode — 2026-05-05)

- **3 APVTS params**: `chordModeEnabled` (Bool), `chordShape` (Choice: 12 shapes), `chordStrumDelay` (Float 0–200ms)
- **12 chord shapes**: Root Only, Major, Minor, Maj7, Min7, Dom7, Dim, Aug, Sus2, Sus4, Power, Octave
- **PendingNote queue** (64 slots, pre-allocated, no heap): chord tones with strum delay fired at block start
- **ActiveChord tracker** (32 slots, pre-allocated): per-root chord state for accurate noteOff cleanup
- **processBlock integration**: pending queue advanced at top of block; root fires normally via existing path; chord tones 1..n scheduled with `ni × strumSamples` delay; noteOff cancels pending + fires noteOff for fired tones
- **CHORD / STRUM section card** in SynthPanel (Classic mode only, hidden in FM mode): CHORD toggle, Shape ComboBox (12 items), STRUM rotary knob (0–200ms)
- Real-time safe: no heap alloc, no locks; MPE-aware (chord mode bypassed when MPE enabled)
- Build verified: VST3 + AU both build and install successfully ✅

## Completed (Performance View — 2026-05-05)

- **"Perf" tab** added to PluginEditor (11th tab)
- **PerformancePanel** (`Source/Performance/PerformancePanel.h/.cpp`): 4×2 grid of large RotaryVerticalDrag knobs (macro1-8, APVTS SliderAttachment); editable name labels (double-click to rename, synced from MacroManager at 30 Hz); BPM readout from DAW playhead
- **XYPad inner class**: neumorphic card; maps macro1 (X-axis) and macro2 (Y-axis); mouse drag/down calls `setValueNotifyingHost`; 30 Hz timer syncs dot position from APVTS when external changes occur; grid + crosshair + glow dot drawn in paint()
- **Neumorphic section cards**: info strip card, macro area card (via `drawNeumorphicRect`); XY Pad draws its own card inline
- Build verified: VST3 builds and installs successfully ✅

## Completed (Factory Preset Library — 2026-05-05)

- **`Source/Presets/FactoryPresets.h/.cpp`** — programmatic factory preset generator
- **100 presets** across 6 categories: Init (5), Bass (20), Lead (20), Pad (25), Drums (15), FX (15)
- Each preset is a complete XML state (all 220+ APVTS params + SynthParams osc nodes) ensuring clean load from any prior state
- Normalization helpers (`nLin`, `nSkw`, `nCh`, `nB`) + shorthand (`A/D/R`, `FC`, `FR`, `LR`, `PT`, `DT`) for readable preset definitions
- `PresetData` struct with convenience setters: `setAdsr`, `setFilter`, `setReverb`, `setChorus`, `setDelay`, `setUnison`, `setMono`, `setOscApvts`, `applyOscs`
- `PresetManager::createFactoryPresetsIfNeeded()` updated: recursive scan (< 10 files triggers generation); calls `FactoryPresets::writeToDirectory()`
- Files added to Multiverse.jucer (Dd0007/Dd0008); Projucer --resave run
- Build verified: VST3 + AU both build and install successfully ✅

## Completed (Preset Browser Polish — 2026-05-06)

- **Category filtering fixed**: `PresetManager::getPresetCategory(int)` returns the parent directory name of each preset file (e.g. `Factory/Bass/*.mvpreset` → "Bass"); `rebuildFilter()` now compares actual file location instead of matching category name against preset name
- **Preset count label**: `countLabel` added to `PresetBrowserPanel`, right-aligned in the search bar row; shows "N presets" when unfiltered, "N / Total presets" when a category or search is active
- Build verified: VST3 builds and installs successfully ✅

## Completed (MIDI Drag-Out — 2026-05-06)

- **`DragMidiButton`** inner class in `SequencerPanel.h/.cpp`: inherits `juce::Component` + `juce::SettableTooltipClient`
- Mouse drag beyond 6px threshold writes current pattern to `$TMPDIR/multiverse_seq_drag.mid` (via `sequencer.exportMidi()`) then calls `juce::DragAndDropContainer::performExternalDragDropOfFiles()` — OS-native file drag accepted by all major DAWs
- `isDragStarted` flag prevents double-firing; reset on `mouseUp` and `mouseDown`
- **"^ DRAG" button** added to export row in `SequencerPanel`, left of chord label; cyan border, hover highlight
- Existing "Export MIDI" file-chooser button retained alongside it
- Build verified: VST3 builds and installs successfully ✅

## Completed (Phase 7.5 + 7.6 — Global Quality & FX Mode) (2026-05-06)

- **7.5 Global Quality (Oversampling)**: `globalQuality` APVTS Choice param (Off/2x High/4x Ultra); `activeOversampler` (unique_ptr, rebuilt in prepareToPlay); effects chain (Chorus/Distortion/EQ/Compressor/Delay/Reverb) prepared at oversampled SR; processBlock upsamps mixed signal via JUCE dsp::Oversampling, applies effects on upsampled AudioBlock, downsamples back; reports latency via `setLatencySamples()`; `QUAL` ComboBox in header bar with APVTS attachment; settings take effect on next play
- **7.6 FX Mode (Audio Input Bus)**: Audio input bus declared (`withInput("Audio In", stereo, false)`) in plugin constructor; `fxModeEnabled` Bool APVTS param; `isBusesLayoutSupported` accepts disabled or stereo input; processBlock skips `buffer.clear()` on channels 0-1 when FX mode is ON (input audio preserved); synth engines still add on top (can still play MIDI); `FX` ToggleButton in header with APVTS attachment

**Build verified:** VST3 builds and installs successfully ✅

## Completed (State-of-the-Art Preset Browser — 2026-05-08)

- **Preset metadata model**: `Preset` class extended with `description` (juce::String) and `tags` (juce::StringArray); `serialize()` writes `description` and `tags` as XML attributes; `deserialize()` reads them and auto-extracts #hashtags from description text
- **PresetManager backend**:
  - `PresetMetadata` struct (name, category, author, description, tags, index) — cached per preset via `scanPresetMetadata()`
  - `tagIndex` map and `getAllTags()` for tag discovery
  - `favorites.json` persistence: 8-color favorites (Zebra2-style), `isFavorite()`, `getFavoriteColor()`, `setFavoriteColor()`, `getFavoritesIndices()`
  - Back/forward history: `pushHistory()`, `canGoBack()`, `canGoForward()`, `goBack()`, `goForward()` — browser-style navigation
  - Bug fix: `saveState()` now writes `category` to XML root (was always "Init" before)
- **PluginProcessor extensions**:
  - `currentPresetCategory`, `currentPresetAuthor`, `currentPresetDescription`, `currentPresetTags` — persisted in `getState/setState`
  - Preview note system: `std::atomic<int> previewNote`, `previewSamplesLeft`, `previewPrevNote`, `previewNoteOn` — lock-free message→audio thread; `triggerPreviewNote()` / `cancelPreviewNote()` on message thread; processBlock handles noteOn/noteOff with 300ms hold at middle C
  - `loadPresetAtIndex()` now calls `pushHistory()`
- **Factory presets metadata**: All 100 presets have `author`, `description` (with #hashtags), and `tags` (comma-separated) fields in `FactoryPresets.cpp` PresetData struct; `buildPresetXml()` writes them as `<Multiverse category="..." author="..." description="..." tags="...">` root attributes
- **Preset browser panel redesign** (`PresetBrowserPanel.h/.cpp`):
  - 280px collapsible panel (was 220px)
  - Search bar row: TextEditor + count label + Preview toggle + Save/Import/Export buttons
  - Category pills: All/Init/Bass/Lead/Pad/Drums/FX/Favs (8 buttons, radio group)
  - Tag filter row: removable tag pills with × close buttons
  - Preset list: favorite color dot (8px circle), preset name, right-aligned category and author
  - Metadata detail strip: author, description, clickable tags
  - Auto-preview: 50ms timer polls selected row, triggers preview note on change
  - Save dialog (CallOutBox): name, category (ComboBox), author, description (multi-line), tags
  - Right-click context menu: Load, Delete, Favorite (8-color submenu), Export
  - #hashtag extraction from search text; tag autocomplete via PopupMenu
  - Filter logic: category AND search AND tags (AND logic for multiple tags)
- **Header navigation bar** (`PluginEditor.h/.cpp`):
  - `< prev` / `> next` preset navigation buttons
  - Preset name label (centered, 180px)
  - `★` favorite button — cycles through 8 colors then clears
  - `◀ back` / `▶ forward` history navigation
  - `Cmd+F` keyboard shortcut to focus preset search
  - `updatePresetNameLabel()`, `updateFavoriteButtonColor()`, `navigatePresetPrev()`, `navigatePresetNext()`, `cycleFavorite()`
- **Plugin classification fix**: `JucePluginDefines.h` corrected — `JucePlugin_IsSynth=1`, `JucePlugin_WantsMidiInput=1`, `JucePlugin_Vst3Category="Instrument"`, `JucePlugin_VSTCategory=kPlugCategSynth`, `JucePlugin_AUMainType='aumu'` (was incorrectly set to effect)
- **Backward compatibility**: Old `.mvpreset` files without metadata attributes load fine with defaults (category="Init", author="MultiphaseAudio", description="", tags="")

**Build verified:** VST3 + AU both build and install successfully ✅

## Next Session

**Phase 3 — Librarian (collapsible sections + bookmark folders):** Replace ListBox preset list with custom `LibrarianPresetList` using collapsible section headers (Factory > Init/Bass/Lead/Pad/Drums/FX, User Presets, Bookmarks). Add bookmark folder right-click menus (create/rename/delete folders, add/remove presets).

**Full plan:** `/Users/jason/.claude/plans/staged-snacking-taco.md`

**Remaining Phase 7 (deferred):** 7.2 (standalone via Projucer GUI — user action only).

## Completed (UI Overhaul Phase 1 — Flat Theme) (2026-05-08)

- **MultiverseFlatTheme** (`Source/MultiverseFlatTheme.h/.cpp`) — new LookAndFeel replacing CyberpunkTheme
  - Flat, clean design inspired by Nexus 5 (no neumorphic shadows, no gradients, no glow effects)
  - New color palette: `bgVoid` (#0F1014), `bgBase` (#15171C), `bgRaised` (#1E2028), `bgDeep` (#111318), `bgHover` (#262830)
  - Accents: `accentCyan` (#00D4FF), `accentPink` (#FF2A6D), `accentPurple` (#9B6DFF), `accentGreen` (#00FF87), `accentAmber` (#FFB800)
  - Borders: `borderLight` (#2A2D38), `borderActive` (#00D4FF)
  - Flat knob: outline disc, thin arc, indicator line, small tip dot (no gradient, no LED glow)
  - Flat toggle: pill with simple dot indicator (no shadow)
  - Flat buttons: solid fill + 1px border (no shadow)
  - Flat tabs: text-only with bottom accent line (no glow)
  - `drawCard()` static method replaces `drawNeumorphicRect()` — flat bgRaised fill + borderLight stroke
- **All panels updated** to use MultiverseFlatTheme colors and `drawCard()` instead of neumorphic shadows
- **PresetBrowserPanel** unified to global theme colors (removed local anonymous-namespace color constants)
- **Backward-compatible aliases**: `accentBlue`, `neonCyan`, `neonPink`, `neonPurple`, `neonGreen` map to new accent colors
- CyberpunkTheme.h/.cpp still exist in project (not compiled by any panel anymore)
- **Plan**: 5 phases total — Phase 1 (theme) ✅, Phase 2 (layout) ✅, Phase 3 (librarian), Phase 4 (quick FX), Phase 5 (routing)

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul Phase 2 — Layout Restructure) (2026-05-08)

- **Layout restructure**: PluginEditor now uses permanent 3-column layout:
  - **Left sidebar (280px)**: PresetBrowserPanel always visible (no more collapsible overlay)
  - **Right FX strip (200px)**: QuickFXStrip placeholder for Phase 4
  - **Bottom bar (88px)**: 8 macro knobs (32px, NoTextBox) + virtual keyboard
  - **Header (32px)**: Compact — menu button (☰), preset nav, randomize
- **Menu button (☰)**: Save/Import/Export, UI Scale submenu, Quality submenu, FX Mode toggle, MIDI Learn toggle (CallOutBox), Show Tooltips toggle
- **Hidden controls** (kept for APVTS attachments): `scaleCombo`, `qualCombo`, `fxModeButton`
- **BottomBar** (`Source/UI/BottomBar.h/.cpp`): 8 macro knobs with APVTS SliderAttachment + editable name labels + MidiKeyboardComponent; 30Hz timer pushes macro values to targets
- **QuickFXStrip** (`Source/UI/QuickFXStrip.h/.cpp`): Placeholder with section headers (FILTER/AMP/DELAY/REVERB/OUTPUT) — Phase 4 fills with controls
- **Tab changes**: 3-letter abbreviations (SYN/DRM/MOD/SMP/SEQ/ARP/FX/GRN/LYR/PRF); MAC tab removed (macros in bottom bar)
- **PresetBrowserPanel adapted** for 280px sidebar: compact action buttons, narrower category pills (28px, abbreviated labels: All/Ini/Bas/Led/Pad/Drm/FX/Fav)
- **MIDI Learn**: Moved from header toggle+combo to ☰ menu → CallOutBox with SafePointer
- **Cmd+F**: Now just focuses preset search (sidebar always visible, no toggle needed)

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul Phase 3 — Librarian Panel, partial) (2026-05-08)

- **Characters field** added to PresetMetadata, Preset class, PresetManager scan/build, and PluginProcessor state persistence
  - `juce::StringArray characters` in PresetMetadata, parsed from `characters` XML attribute (comma-separated)
  - `PresetManager::buildCharacterIndex()` + `getAllCharacters()` for character-based filtering
  - `PresetManager::currentPresetCharacters` persisted in getState/setState XML root attribute
- **Factory presets** all have character tags (1-2 per preset): dark/bright/active/spiky/wide/dirty
- **Bookmark persistence** added to PresetManager: `BookmarkFolder` struct, `bookmarks.json` file, full CRUD API
- **LibrarianPanel** (`Source/Presets/LibrarianPanel.h/.cpp`) replaces PresetBrowserPanel in PluginEditor
  - History navigation: ◀ ▶ buttons in sidebar (same PresetManager history stack as header)
  - Category pills: All/Bass/Lead/Pad/Drums/FX (full names, 36px each)
  - Character filter pills: Dark/Bright/Active/Spiky/Wide/Dirty — 3-state toggle (None→Include→Exclude→None)
  - Favorite filter: ♥ button toggles showing only favorited presets
  - Search bar with #tag extraction
  - Metadata strip: author, description, tags (accentCyan), characters (accentAmber)
  - Save dialog: now includes "Chars" field for character tags
  - Right-click context menu: Load/Save/Delete/Import/Export/Favorite + "Add to Bookmark" submenu
  - Auto-preview (50ms timer), import/export via FileChooser
- **PluginEditor** swapped from `PresetBrowserPanel` to `LibrarianPanel`
- **Multiverse.jucer** updated with LibrarianPanel.h/.cpp entries

**Completed sub-tasks 3.6 + 3.7 (2026-05-08):**
- **3.6 Collapsible section-based preset list**: `LibrarianPresetList` custom component replaces `juce::ListBox`
  - Factory section with collapsible sub-sections (Init/Bass/Lead/Pad/Drums/FX)
  - User Presets section
  - Bookmarks section with per-folder sub-sections
  - Section headers (22px, `bgDeep` fill, accentCyan disclosure triangle)
  - Sub-section headers (20px, `bgBase` fill, `textMuted` text, small disclosure triangle)
  - Preset rows (28px): favorite color dot + name + category right-aligned
  - `Viewport` for scrolling; `hitTest()` for click detection; hover highlighting via `mouseMove`
  - Category pills filter which sub-sections are shown; collapsed sub-sections hidden
- **3.7 Bookmark folder UI**: Right-click menus for bookmark management
  - Right-click Bookmarks section header → "New Folder" (promptNewBookmarkFolder CallOutBox)
  - Right-click bookmark sub-section header → "Rename Folder" / "Delete Folder"
  - Right-click preset in bookmark → "Remove from 'Folder Name'" option
  - "Add to Bookmark" submenu on all preset right-clicks (existing, preserved)
  - `bookmarkFolderIndex` tracked in `Section::SubSection` for hit testing

**Phase 3 COMPLETE.**

**Build verified:** VST3 builds and installs successfully ✅

## Completed (UI Overhaul Phase 4 — Quick FX Strip) (2026-05-08)

- **Filter Modifier** (bipolar offsets to per-voice filter):
  - `filterModEnabled` (Bool), `filterModCutoff` (Float -1..1), `filterModResonance` (Float -1..1), `filterModEnvDepth` (Float -1..1)
  - Cutoff offset maps ±1 → ±8000 Hz; Resonance offset maps ±1 → ±5.0; Env depth scales ModEnv2 to filter cutoff
  - Applied in processBlock: effCutoff/effRes modified before `synthEngine.setFilterParams()`
- **Amp Modifier** (bipolar offsets to amp volume/pan):
  - `ampModEnabled` (Bool), `ampModVolume` (Float -1..1), `ampModPan` (Float -1..1)
  - Volume offset maps ±1 → ±0.5; Pan offset adds directly to effPan
  - Applied in processBlock: effVol and effPan modified before panning
- **Delay & Reverb** (direct APVTS parameter access):
  - Delay: Mix, Time, Feedback knobs wired directly to `delayMix`, `delayTime`, `delayFeedback`
  - Reverb: Wet, Room, Damp knobs wired directly to `reverbWet`, `reverbRoom`, `reverbDamp`
  - No new APVTS params — existing params used
- **Main Filter** (post-effects global filter):
  - `mainFilterEnabled` (Bool), `mainFilterType` (Choice: LP/HP/BP/Notch), `mainFilterCutoff` (Float 20-20000, skew 0.3), `mainFilterResonance` (Float 0.1-10, skew 0.5)
  - Applied after pan, before aux sends in processBlock
  - `Filter mainFilter` member in PluginProcessor with dirty-check on cutoff/res/type
  - Prepared in `prepareToPlay()`
- **QuickFXStrip UI** (`Source/UI/QuickFXStrip.h/.cpp`):
  - 5 section cards: FILTER MOD, AMP MOD, DELAY, REVERB, MAIN FILTER
  - Compact layout: 28px NeuKnob with NoTextBox, short 3-char labels below each knob
  - Enable toggles on sections with modifiers (Filter Mod, Amp Mod, Main Filter)
  - ComboBox for Main Filter type (LP/HP/BP/Notch)
  - All knobs use NeuKnob with APVTS SliderAttachment + MidiLearnSlider.init()
  - Section cards drawn with `MultiverseFlatTheme::drawCard()`
- **11 new APVTS parameters** (filterModEnabled/FilterModCutoff/filterModResonance/filterModEnvDepth, ampModEnabled/ampModVolume/ampModPan, mainFilterEnabled/mainFilterType/mainFilterCutoff/mainFilterResonance)

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul Phase 5 — Visual Routing Page) (2026-05-08)

- **RoutingPanel** (`Source/Routing/RoutingPanel.h/.cpp`) — new "ROU" tab (11th tab)
- **LayerBlock** (×8 inner struct): engine type ComboBox (Off/Synth/Granular/Sampler), Mute/Solo toggles, bus selector (Main/Bus 1–8), level bar, bus indicator dot; all wired to `LayerManager` API directly
- **DrumsSummaryBlock**: "DRUMS — 8 tracks" summary card with "Edit" button → switches to DRM tab
- **ChainStrip** (inner struct): drag-reorder effect chain tiles (mirrors EffectChainStrip logic), reads `effectChainOrder` atomic, calls `swapChainSlots` on drag
- **Aux Send knobs**: `auxSendDelay` + `auxSendReverb` NeuKnob with APVTS SliderAttachment
- **Connection lines**: vertical flow lines in `paint()`, color-coded (accentCyan for Main, accentAmber for Bus, accentPink for soloed, textMuted for muted), small arrow heads at section transitions
- **Output terminus**: "→ OUTPUT" label at bottom of signal flow
- **Timer-based UI refresh**: 10 Hz `timerCallback()` syncs engine type, mute/solo, bus, level from `LayerManager`
- **Tab switch callback**: `onSwitchToTab(int)` set by PluginEditor; Layer name click → LYR tab, Drums Edit → DRM tab
- **Multiverse.jucer**: `GrpRou` group added with RoutingPanel.h/.cpp entries
- **PluginEditor.h/.cpp**: `#include "Routing/RoutingPanel.h"`, `RoutingPanel routingPanel` member, `tabs.addTab("ROU", ...)`, `routingPanel.onSwitchToTab` callback

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (Feature Expansion — 2026-05-08)

### Feature 1: Factory Presets 100 → 204
- **204 factory presets** across 8 categories: Init (5), Bass (35), Lead (36), Pad (47), Drums (25), FX (26), Keys (15), Arp (15)
- **New categories**: "Keys" and "Arp" added to all category string arrays (PresetManager, LibrarianPanel, save dialog)
- **Version-based refresh**: `factoryVersion = 3` in PresetManager triggers regeneration when stored version < 3
- **LibrarianPanel**: Added `catKeys` and `catArp` category pill buttons, updated `categoryButtons` array and `setActiveCategory()` mapping
- Presets use all oscillator types, wave shaping, FM mode, chord mode, effects chains; each has author/description/characters/tags metadata
- Build verified ✅

### Feature 2: Drag-Drop Modulation Assignment
- **PluginEditor** inherits `juce::DragAndDropContainer` — enables drag-and-drop hierarchy
- **LFORow::mouseDrag()** — dragging from LFO label area initiates drag with `modsrc:` description encoding ModSourceType enum value
- **NeuKnob** inherits `juce::DragAndDropTarget` — accepts `modsrc:` drops
  - `isInterestedInDragSource()` accepts `modsrc:` prefixed descriptions
  - `itemDragEnter/Exit` toggles `isDragOver` flag for cyan highlight ring
  - `itemDropped()` decodes ModSourceType, calls `paramIDToModTarget()` to find target, adds connection at 0.5 depth
- **Modulation depth arc** — pink outer arc on NeuKnob shows total modulation depth from active connections
- **paramIDToModTarget()** — new free function in ModulationMatrix.cpp mapping APVTS param IDs to `ModTargetMapping{ModTargetType, targetIndex}`
- **MidiLearnSlider** — added `getProcessor()` and `getParamID()` protected accessors for subclass use
- Build verified ✅

### Feature 3: Sampler Timestretch (WSOLA)
- **SamplerZone** — `bool timestretchEnabled = false` field
- **SamplerVoice** — WSOLA processor state:
  - `timestretchActive`, `pitchRate`, `timeRate` split from `playbackRate`
  - Pre-allocated grain/output/hann buffers (no alloc in process)
  - `grainSize`, `hopSynthesis`, `hopAnalysis` computed from sample rate and speed
  - `processTimestretch()` — WSOLA overlap-add: read grains at pitchRate, cross-correlate for best overlap, Hann-windowed overlap-add into output ring
  - Loop handling: grains wrap within loop boundaries
  - Fallback: when `timestretchActive` is false, existing varispeed path unchanged
- **SamplerPanel** — `juce::ToggleButton tstrToggle {"TSTR"}` with tooltip; wired to `zone->timestretchEnabled`
- **SamplerEngine** — `timestretchEnabled` serialized in getState/setState
- Build verified ✅

## Completed (Plugin Classification Fix — 2026-05-09)

**Problem:** Cubase classified plugin as Effect instead of Instrument (second time this happened). Projucer `--resave` resets `JucePluginDefines.h` to effect defaults and writes `type=aufx` in `Info-AU.plist`, despite `pluginIsSynth` in `.jucer` characteristics.

**Fix:**
- `Scripts/fix_instrument_classification.sh` — patches both `JucePluginDefines.h` (IsSynth=1, WantsMidiInput=1, VSTCategory=kPlugCategSynth, Vst3Category="Instrument", AUMainType='aumu') and `Info-AU.plist` (type=aumu)
- Added as pre-compile Run Script build phase in all 3 Xcode targets (Shared Code, AU, VST3)
- AI_RULES.md updated: mandatory post-resave step to run the script
- Feedback memory saved: Projucer bug, not user error — verify after every resave

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul — Avenger-Style Redesign Phase 1) (2026-05-09)

- **Knob redesign** — `MultiverseFlatTheme::drawRotarySlider()` rewritten from chunky filled-disc style to sleek thin-ring arc style:
  - Removed: filled disc background (`bgRaised` fillEllipse), indicator line from center, center dot
  - Added: subtle 0.5px outer ring outline in `bgDeep.withAlpha(0.6f)`, clean arc (270° sweep), small 7px tip dot
  - Arc radius: 0.72, track width: 2.0px (was 2.5px)
- **SynthLookAndFeel removed** — `SynthPanel.h` no longer has `SynthLookAndFeel` class; `SynthPanel` uses global `MultiverseFlatTheme` directly
- **EnvelopeDisplay** (`Source/Synth/EnvelopeDisplay.h`) — NEW FILE: ADSR curve visualizer
  - Inherits `juce::Component` + `juce::Timer` (15 Hz polling)
  - `setSliders()` takes 4 slider pointers for ADSR
  - `paint()` draws dark background with border, ADSR curve as cyan filled path with 1.8px stroke, segment labels (A/D/S/R)
  - `timerCallback()` reads slider values and triggers repaint
- **FilterDisplay** (`Source/Synth/FilterDisplay.h`) — NEW FILE: filter response curve visualizer
  - Inherits `juce::Component` + `juce::Timer` (20 Hz polling)
  - `setProcessor(PluginProcessor*)` for APVTS parameter access
  - `setFilterType(int)` for LP/HP/BP/Notch
  - `paint()` draws dark background, grid lines, frequency labels, filter response curve with cyan fill and stroke, filter type label
  - `computeResponse(freq, sr)` implements simplified biquad response calculation for all 4 filter types
  - `timerCallback()` reads filterCutoff, filterResonance, filterType from APVTS and repaints on change
- **Layout restructuring in SynthPanel**:
  - `synthDisplay` moved to top of panel (120px height, was 90px at bottom)
  - `oscDisplay` given explicit bounds (48px, was not visible)
  - ENV section changed from 130px single-row to 160px two-column (EnvelopeDisplay left, knobs right)
  - Filter section changed to two-column layout (FilterDisplay left 180px, knobs right)
  - `envelopeDisplay` and `filterDisplay` visibility toggled based on mode (shown in Classic, hidden in FM)
- **Font deprecation fixes** — 3 instances of `juce::Font(height, juce::Font::plain)` → `juce::Font(juce::FontOptions{}.withHeight(height))` in MultiverseFlatTheme.cpp
- **Avenger 2 research** — 5-phase redesign plan documented at `/Users/jason/.claude/plans/elegant-stargazing-journal.md`

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul — Avenger-Style Redesign Phase 2) (2026-05-09)

- **OscTabBar** (`Source/Synth/SynthPanel.h` — inner class): color-coded tab bar component for oscillator selection
  - 8 accent colors cycling: cyan, pink, purple, green, amber (repeating for 6-8)
  - Active tab: accent color fill + bottom accent line; inactive: bgRaised fill
  - Each tab shows "OSC N" label + type abbreviation (CLA/WT/ADD/PD/ANA/DIG)
  - +/− buttons rendered inline on the right side of the tab bar for adding/removing oscillators
  - Click handling: tab click switches active oscillator; +/− clicks adjust oscCount APVTS param
- **Tab-based oscillator layout** replaces old 8-strip layout:
  - `activeOscIndex` state tracks which oscillator tab is selected
  - Only the active oscillator's controls are visible and laid out in the detail panel
  - Detail panel: oscDisplay (48px) → Type+Waveform row → Level/Detune/WavePos knobs → Shape type combo → ShapeAmt/SelfOsc/PD knobs → WT buttons
  - 370px vertical section height (was variable 220-460px for 1-2 rows of strips)
- **Accent color line** at top of OSC section: draws the active oscillator's tab color as a thin highlight
- **Type change listeners**: each osc typeSelector.onChange calls updateVisibility() + resized() to update tab abbreviations and re-layout
- **Font deprecation fixes**: remaining `juce::Font(height)` and `juce::Font(height, bold)` → `juce::FontOptions{}.withHeight()` in SynthPanel.cpp
- **Removed**: `addOscButton` / `removeOscButton` (functionality moved to OscTabBar +/− buttons)

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul — Avenger-Style Redesign Phase 3) (2026-05-09)

- **Dual-ring modulation display** — `NeuKnob::paint()` rewritten to show per-source colored modulation arcs:
  - Each modulation connection gets its own coloured arc at an increasing radius offset (3px spacing)
  - Source colors: LFO1=cyan, LFO2=pink, LFO3=purple, LFO4=green, LFO5=amber, LFO6-8 cycle; Env/Env2/Env3=green/amber/pink; Velocity/Note/Aftertouch/ModWheel/PitchBend/Random/EnvFollower/MPEPressure/MPESlide/SeqStep mapped to palette
  - Dim background track arc (0.15 alpha) + bright active arc (0.7 alpha) for each source
  - `NeuKnob::getModSourceColour(ModSourceType)` static method provides the palette
  - Old single pink arc replaced with multi-source coloured arcs
- **Readout display bar** — `ReadoutBar` inner class in SynthPanel:
  - 28px persistent bar at the bottom of SynthPanel showing hovered parameter name (left) + value (right)
  - 30 Hz timer polls mouse position, finds NeuKnob/MidiLearnSlider under cursor
  - `formatParamName()` converts camelCase param IDs to human-readable (e.g. "filterCutoff" → "Filter Cutoff")
  - Styled with bgDeep background, accentCyan parameter name, textPrimary value
- **Drag-drop modulation visual feedback**:
  - Global `NeuKnob::modDragActive` + `NeuKnob::modDragSource` static state set during mod-source drags
  - All valid drop-target NeuKnobs show subtle source-colored outline (0.18 alpha) during drag
  - Hovered drop target shows bright source-colored rounded rect (0.5 alpha)
  - `ModDragOverlay` in PluginEditor: full-editor overlay with 30 Hz timer, draws glow line (6px) + core line (2px) + end dot from drag start to cursor, colored by source type
  - `lineStart` captured when mod drag transitions from inactive to active
  - `dragSourceColour` in NeuKnob set from source type colour on `itemDragEnter`

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul — Avenger-Style Redesign Phase 4) (2026-05-09)

- **SignalFlowBar** (`Source/Synth/SignalFlowBar.h`) — NEW FILE: compact horizontal signal-flow strip
  - 7 blocks: OSC → SUB → FLT → SHP → ENV → AMP → OUT
  - Active blocks: accentCyan fill (0.18 alpha) + border (0.6 alpha) + label
  - Inactive blocks: bgRaised fill + borderLight border + textMuted label
  - Arrow lines between blocks: colored by source block (cyan if active, dimmed if not)
  - 10 Hz timer polls APVTS: SUB active when sub/noise oscillator enabled; SHP active when any osc has ShapeType > Off; OSC/FLT/ENV/AMP/OUT always active
  - Click detection on blocks (32px tall, 52px wide blocks)
  - Hidden in FM mode (visibility controlled by SynthPanel::updateVisibility)
  - Placed between synthDisplay and OSC section in SynthPanel layout (32px strip)

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (Phase 5 — Theme Polish) (2026-05-09)

- **Design system constants** — `MultiverseFlatTheme::Metrics` struct: `fontHeader/Label/Value/Title`, `outerMargin/sectionPadding/sectionGap/smallGap/sectionHeaderH`, `dividerAlpha`
- **Font getters** — `headerFont()` (11pt bold), `labelFont()` (9pt plain), `valueFont()` (10pt monospaced), `titleFont()` (13pt bold)
- **drawDivider()** static helper — 1px `borderLight` line at configurable opacity
- **BottomBar macro value readout** — Added `valueLabel` per macro knob showing 0-100%, cell width expanded from 36→42px
- **LFO color fix** — `ModulationMatrixPanel` LFO row labels now use `NeuKnob::getModSourceColour()` instead of hardcoded `accentBlue`; LFO1=cyan, LFO2=pink, LFO3=purple, LFO4=green
- **Font normalization** — All 14+ panels migrated from hardcoded font sizes to theme getters:
  - SynthPanel, SynthDisplay, FilterDisplay, EnvelopeDisplay, SignalFlowBar
  - EffectsPanel, GranularPanel, LayersPanel, MacroPanel
  - PerformancePanel, RoutingPanel, LibrarianPanel, QuickFXStrip
  - PluginEditor presetNameLabel → `titleFont()`
- **Spacing normalization** — `.reduced(16)` → `.reduced(Metrics::outerMargin)`, gap values → `Metrics::sectionGap/smallGap`
- **Divider lines** — Added `drawDivider()` calls between sections in SynthPanel, GranularPanel, EffectsPanel, MacroPanel, LayersPanel
- **Build fix** — `JUCE_DECLARE_NON_COPYABLE_WITH_LEAKY_DETECTOR` → `JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR` (typo fix for JUCE 7+); `valueFont()` uses `Font::getDefaultMonospacedFontName()` instead of nonexistent `withMonospaced()`

**Build verified:** VST3 + AU both build and install successfully ✅

## Completed (UI Overhaul — All-Oscillator View + Proportions) (2026-05-09)

- **Tab visibility fix** — `MultiverseFlatTheme::drawTabButton()` inactive tabs now use `bgHover` fill (clearly lighter) on `bgDeep` tab bar background; added `borderLight` bottom line for visual separation; `drawTabAreaBehindFrontButton()` uses `bgDeep` instead of `bgRaised`
- **Search bar ♫ symbol fix** — `LibrarianPanel.h`: `autoPreviewButton` changed from Unicode `♫` (U+266B) to plain text `"Auto"`
- **Metrics overhaul** — `MultiverseFlatTheme::Metrics` font sizes increased: header 11→12, label 9→10, value 10→11, title 13→14; spacing: sectionGap 8→10, sectionPadding 8→10, smallGap 4→5, outerMargin 8→10
- **All-oscillator view** — `OscTabBar` removed; all active oscillators shown simultaneously in a horizontal row in Classic mode; each oscillator has its own mini `OscDisplay` waveform indicator, type selector, level/detune/wavePos knobs, shape controls, wavetable buttons; `+`/`-` buttons replace tab bar for oscillator count control; `SynthDisplay` (oscilloscope+FFT) and `SignalFlowBar` removed from SynthPanel to free 152px vertical space; `LFODisplay` removed
- **Per-oscillator waveform display** — `std::array<OscDisplay, 8> oscDisplays` replaces single `OscDisplay`; each osc strip shows its waveform type; waveform selector onChange updates the display per-oscillator
- **Progressive disclosure** — when strip width < 120px (many oscillators), advanced shaping controls are hidden; only type, level, detune shown
- **SynthPanel layout** — horizontal osc row at top (280px section), then SUB/NOISE (110px), UNISON (110px), FILTER (170px, with FilterDisplay), ENV (150px, with EnvelopeDisplay), CHORD/STRUM (90px), readout bar (28px)
- **Proportion fixes** — SynthPanel knobs 70→80px; QuickFXStrip knobs 28→36px, labels 12→14px, combos 20→24px, gaps 6→8px; BottomBar knobs 32→38px, cell width 42→48px; left sidebar 280→260px
- **FM mode** unchanged — FM operator layout still works with separate branch in `resized()` and `updateVisibility()`
- **Build verified:** VST3 + AU both build and install successfully ✅

## Completed (Serum 2-Style UI Redesign — Phase 1: ModBar) (2026-05-09)

- **ModBar replaces BottomBar + QuickFXStrip** — single 160px bottom modulation bar with 7 sub-tabs: ENV1, ENV2, ENV3, LFO, MACRO, QFX, KEY
- **EnvSubPanel** — 4 ADSR NeuKnobs + EnvelopeDisplay per envelope; ENV1 uses `attack/decay/sustain/release`, ENV2 uses `modEnv2Attack/Decay/Sustain/Release`, ENV3 uses `modEnv3Attack/Decay/Sustain/Release`
- **LFOSubPanel** — 8-button bank selector (LFO1-8) with accent colors; Rate knob, Shape combo, Sync toggle, Sync Division combo, DRAW button (opens LFOShapeEditor CallOutBox); waveform preview; attachments rebuild on bank switch
- **MacroSubPanel** — 8 macro knobs with name labels + value readouts; timer-driven macro push (same pattern as old BottomBar)
- **QuickFXSubPanel** — Filter Mod (enable + cutoff/res/env), Amp Mod (enable + vol/pan), Delay (mix/time/fdb), Reverb (wet/room/damp), Main Filter (enable + type + cutoff/res); horizontal layout
- **KeyboardSubPanel** — Pitch wheel + mod wheel + MidiKeyboardComponent
- **PluginEditor layout** — 36px header, tabs fill full width (no right strip), ModBar 160px at bottom (full width)
- **QuickFXStrip removed from PluginEditor** — its content merged into ModBar QFX sub-tab
- **BottomBar.h/.cpp** — renamed class from `BottomBar` to `ModBar`; file names unchanged; PitchWheel and ModWheel classes preserved
- **Build verified:** VST3 + AU both build and install successfully ✅

## Next Session

**Serum 2-Style UI Redesign — Phase 2: Header Redesign** (36px header, clickable preset name, visible scale/quality/FX mode controls)

**Remaining phases:** Phase 2 (Header), Phase 3 (Two-tier tabs), Phase 4 (Modulation restructure), Phase 5 (Preset overlay), Phase 6 (Theme polish)

**Remaining Phase 7 (deferred):** 7.2 (standalone via Projucer GUI — user action only).
