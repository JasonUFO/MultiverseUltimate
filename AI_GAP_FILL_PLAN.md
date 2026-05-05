# MultiverseUltimate Gap-Fill Implementation Plan
## Preamble: Clarifying Questions — RESOLVED
1. ✅ **Per-voice**, cap 8 oscillators. Global `oscCount` controls count for all voices simultaneously.
2. ✅ **Extend the enum** — Additive/PhaseDist/Analog/Digital are `OscillatorType` values, not separate engines.
3. ❓ Multi-output: osc-level, voice-level, or layer-level? (Phase 4 — defer)
4. ❓ 1000+ presets: programmatic or curated? (Phase 6 — defer)
5. ❓ Standalone mode: separate executable or audio effect? (Phase 7 — defer)

---

## Phase 0: Existing Functionality Verification ✅ COMPLETE (2026-05-04)
- ✅ ModulationMatrix LFO advancement and target routing confirmed functional
- ✅ SamplerEngine/DrumSequencer audio output wiring confirmed
- ✅ Per-step velocity confirmed in Sequencer/DrumSequencer

---

## Phase1: Core Synthesis & Oscillator Upgrades ✅ COMPLETE (2026-05-04)
**Architecture chosen:** per-voice, 8 pre-allocated slots, global `oscCount` param, extend `OscillatorType` enum, wave shaping as Drive/Fold/Clip modes.
1.1 ✅ Dynamic 1–8 oscillators per voice — `oscCount` APVTS Choice; `+ OSC`/`- OSC` buttons; 2-row layout in SynthPanel; `SynthEngine::setOscCount()` propagates to all voices
1.2 ✅ New types: Additive (8-harmonic Fourier), PhaseDist (sinusoidal phase bending), Analog (LCG micro-drift), Digital (16-level bit reduction)
1.3 ✅ Wave shaping (Drive/Fold/Clip) + self-oscillation feedback per osc; inline `applyOscShaping()` helper in Voice.cpp; `OscShapeType` enum
1.4 ✅ Mod targets `OscShapeAmount` + `OscPhaseDistAmount` added; `MAX_MOD_TARGETS` 24→26; wired in processBlock (osc 0)

---

## Phase2: Sampler Enhancements ✅ COMPLETE (2026-05-04)
**Architecture:** per-zone `tuning` + `speed` fields in `SamplerZone`; folded into `MvSamplerVoice::noteOn` playbackRate; UI controls added to SamplerPanel.
2.1 ✅ Drag-and-drop for audio files — was already implemented (`FileDragAndDropTarget` in SamplerPanel); verified functional
2.2 ✅ Independent tune/speed controls — `tuning` (±24 st, 0.1 res) + `speed` (0.25–4×) per zone; `playbackRate = exp2((note-root+tuning)/12) * (fileSR/sr) * speed`
2.3 ✅ Zone range editing UI — Lo Key / Hi Key combos + Lo Vel / Hi Vel sliders wired to `SamplerZone::loNote/hiNote/loVel/hiVel`; Auto Map button distributes n zones evenly across MIDI 0–127
2.4 MIDI pattern drag/sync with Sequencer — deferred to Phase 3

---

## Phase3: Sequencer & Arpeggiator Upgrades ✅ COMPLETE (2026-05-04)
3.1 ✅ MIDI drag-and-drop import — `FileDragAndDropTarget` on SequencerPanel; .mid/.midi quantized to 16-step grid; cyan glow on drag-enter
3.2 ✅ Smart chord tracking — 10-chord table (maj/min/maj7/dom7/m7/dim/aug/m7b5/sus4/sus2); `chordLabel` in export row, updates at 50ms
3.3 ✅ Polyrhythm/step length — `SeqPattern::stepLengthMultiplier`; STEP ComboBox (32nd/16th/8th/Qtr/8T); per-pattern, state-persisted
3.4 ✅ Per-step probability — `Step::probability`; xorshift RNG check in `process()`; right-click → Probability submenu; coloured dot indicator
3.5 ✅ Sequencer Step mod source — `ModSourceType::SequencerStep`; normalized 0→1 in processBlock; visible in ModMatrix dropdown as "Seq Step"
3.6 Drag MIDI to audio — DEFERRED (requires offline engine rendering)

---

## Phase4: Audio Output & Drum Sequencer ✅ COMPLETE (2026-05-04)
**Architecture:** 17 fixed output buses (Logic Pro compatible). Layer routing at layer granularity. Drum routing at per-track granularity.
4.1 ✅ 17 stereo output buses: Bus 0 (main), Buses 1–8 (Layer 1–8), Buses 9–16 (Drum 1–8); all disabled by default except main
4.2 ✅ Per-layer routing: `LayerEngine::outputBusIndex`; "BUS" ComboBox in LayersPanel; `PluginProcessor` routes via `getBusBuffer()` after effects chain; `LayerManager::processBlock` skips individual-bus layers
4.3 ✅ Per-drum-track FX: `LayerEffectChain trackFX[8]` + `trackBufs[8]` in DrumSequencer; `process()` refactored to accumulate per-track; FX applied per track; "FX" button in TrackRow → CallOutBox; "BUS" combo (Main/Out 1–8) → `setTrackOutputBus()`; state persisted

---

## Phase5: Modulation Upgrades ✅ COMPLETE (2026-05-04)
5.1 ✅ 8 LFOs (LFO5-LFO8 added; arrays replace individual members; advanceLFOs loops 8)
5.2 ✅ LFO shapes: Sine/Triangle/Saw/Square/S&H per LFO; shape-aware advanceLFOs(); lfoXShape APVTS params
5.3 ✅ DAW sync: lfoXSync Bool + lfoXSyncDiv Choice (1/32-4/1); synced rate = bpm/60/divisor in processBlock
5.4 ✅ Mod envelopes: juce::ADSR modEnv2/modEnv3 in PluginProcessor; triggered from noteOn/Off; Envelope2/3 mod sources

---

## Phase6: UI & Presets ✅ COMPLETE (2026-05-04)
6.1 ✅ Resizable UI: setResizable + setResizeLimits(800-1920); Scale combo 75/100/125/150% in header
6.2 ✅ Built-in keyboard: keyboardState in PluginProcessor; MidiKeyboardComponent 64px footer in PluginEditor; range C2-C8
6.3 1000+ presets — DEFERRED (requires preset content generation)
6.4 Drag-drop assign — DEFERRED

---

## Phase7: Effects & Additional Features (Priority: Low-Medium)
7.1 ✅ Return/aux sends: `auxSendDelay`/`auxSendReverb` APVTS params; parallel `auxDelay`/`auxReverb` instances; dry mix captured pre-chain; SENDS card in EffectsPanel (2026-05-04)
7.2 Standalone mode
   - Enable JUCE standalone target in Multiverse.jucer
7.3 ✅ Quick randomization: RAND button in header; PopupMenu with 4 options; randomizeParams(prefixes) skips structural/sync params (2026-05-04)
7.4 ✅ CPU voice limiting: setVoiceLimit(int) in SynthEngine; maxVoices APVTS Choice (1-16); findFreeVoice/findFreeFMVoice limited to voiceLimit slots (2026-05-04)
7.5 Zero-latency + oversampling — DEFERRED
7.6 Audio effect mode — DEFERRED (complex bus reconfiguration)
7.7 ✅ Metronome: metronomeEnabled Bool + metronomeVolume Float APVTS; PPQ beat detection using prevDawPpqPos; 25ms decaying sine click (1200Hz downbeat, 900Hz off-beat) added to buffer (2026-05-04)
   ✅ Tuner UI: FFT peak detection + parabolic interpolation in SynthDisplay; note name + cents bar in scope area (2026-05-04)
7.2 Standalone mode — DEFERRED: must be enabled in Projucer GUI (File Formats → Standalone Plugin checkbox); plugin code is already standalone-compatible

---

## Phase8: Testing & Validation (Ongoing)
- Test all features in Logic Pro/Ableton Live
- Run `auval -v aufx MvUl MpAu` for AU validation
- Verify state persistence for all new parameters
- Profile CPU usage for voice limiting/oversampling

---

## Phase9: Competitive Feature Expansion (Post-Gap, 2026-05-05+)

### 9.1 Drawable LFO Shapes ✅ COMPLETE (2026-05-05)
- `LFOShape::Custom` (index 5) added to enum
- `LFOShapeEditor` component: pencil+line draw, SIN/SAW/SQR/TRI fill, NORM
- `lfoCustomTable[8][256]` in ModulationMatrix; linear interpolation in `advanceLFOs()`
- `DRAW` button in each LFO row (enabled only when Custom selected)
- State persistence: comma-separated float string per LFO in preset XML

### 9.2 Chord/Strum Mode (Next)
**Goal:** Single MIDI note triggers a full chord voicing, with optional strum delay between notes.
**Architecture (proposed):**
- `ChordMode` enum {Off, Major, Minor, Dom7, Maj7, Min7, Sus4, Sus2, Custom} — APVTS Choice param
- `strumDelayMs` APVTS Float param (0–80ms)
- `chordIntervals[8]` lookup table per mode (semitone offsets from root)
- In `SynthEngine::noteOn()`: if chordMode != Off, fire additional voices at root+interval with staggered timing using `strumDelayMs` converted to samples
- UI: CHORD section in SynthPanel header row — mode ComboBox + strum delay slider
- No new MIDI generation; chord notes fired directly as additional voice noteOns

### 9.3 Performance View (Future)
**Goal:** Full-screen overlay showing 8 large macro knobs + optional XY pad.
**Architecture (proposed):**
- New `PerformancePanel` component; "PERF" toggle button in header shows/hides as full overlay
- 8 NeuKnobs wired to macro1–macro8 APVTS params (same as MacroPanel)
- Optional XY pad: X→macro1, Y→macro2 (or user-assignable)
- No new DSP; purely UI

### 9.4 Programmatic Preset Generation (Future)
**Goal:** Ship 100+ named factory presets across Bass/Lead/Pad/Drums/FX categories.
**Architecture (proposed):**
- Standalone C++ tool or in-plugin "Generate Presets" dev button
- Randomize within musical parameter ranges per category (e.g. Bass: low cutoff, high sustain)
- Save as `.mvpreset` files to `~/Library/Audio/Presets/MultiphaseAudio/MultiverseUltimate/`

---

## Implementation Rules (Per `AI_RULES.md`)
- Reuse existing JUCE 8 APIs, no new external libraries
- Follow existing code conventions: angle-bracket JUCE includes, C++17, JUCE 8 deprecation workarounds
- Minimal changes: Extend existing SynthEngine/LayerManager/ModulationMatrix infrastructure
- No full repo scans: Use context from `AI_HANDOFF.md`/`AI_STATE.md` for file locations
- All new parameters added to APVTS for DAW automation
- State persistence via existing XML/ValueTree system
