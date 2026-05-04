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

## Next Session

**All tracks complete — ready for gap-fill work per `AI_GAP_FILL_PLAN.md`.**

**Competitive brief reminder:** Goal is to match/surpass Serum 2, Nexus 5, Avenger 2, Diva, Zebra 3.

**Pending clarification (before Phase 1):**
1. Should unlimited oscillators be per-voice or per-layer?
2. Should new synth engines be per-oscillator types or standalone engines?
3. Multi-output: individual osc, voices, or entire layers?
4. 1000+ presets: programmatic generation or curated?
5. Standalone mode: separate executable or audio effect mode?
