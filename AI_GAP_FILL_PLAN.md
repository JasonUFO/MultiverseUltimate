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

## Phase5: Modulation Upgrades (Priority: Medium)
**Dependencies**: Phase1 (new oscillator targets)
5.1 Unlimited LFOs/envelopes
   - Replace fixed 4 LFOs with dynamic `std::vector<LFOState>`
   - Add UI to add/remove LFOs in ModulationMatrixPanel
5.2 Drawable custom LFO shapes
   - Add LFO shape editor (reuse WavetableEditor logic) to ModulationMatrixPanel
5.3 DAW-synced LFO shapes
   - Sync LFO phase/rate to DAW transport (extend existing DAW sync)
5.4 Unlimited envelopes: Add dynamic envelope support per modulation target

---

## Phase6: UI & Presets (Priority: Medium)
6.1 Resizable UI & DPI scaling
   - Add UI scale slider to header, use `Desktop::setGlobalScaleFactor()`
   - Add DPI-aware asset loading
6.2 Built-in keyboard
   - Add `juce::MidiKeyboardComponent` to SynthPanel/LayersPanel
   - Add scale/chord selection with highlighted compatible keys
6.3 1000+ presets
   - Generate presets programmatically via randomization (leverage Phase7 quick randomization)
   - Organize into brief-specified categories
6.4 Enhance drag-drop assign to all UI parameters

---

## Phase7: Effects & Additional Features (Priority: Low-Medium)
7.1 Return/aux sends
   - Add send/return buses to effect chain, pre/post reverb send
   - Update EffectsPanel UI with send level controls
7.2 Standalone mode
   - Enable JUCE standalone target in Multiverse.jucer
7.3 Quick randomization
   - Add "Randomize" button to header, support per-section/all params
7.4 CPU voice limiting
   - Add voice limit parameter to SynthEngine/GranularEngine/SamplerEngine
7.5 Zero-latency + oversampling
   - Extend filter oversampling to global zero-latency mode
7.6 Audio effect mode
   - Add stereo audio input bus to PluginProcessor for external processing
7.7 Built-in tuner/metronome
   - Add tuner UI (reuse SynthDisplay FFT)
   - Add metronome click track to Sequencer

---

## Phase8: Testing & Validation (Ongoing)
- Test all features in Logic Pro/Ableton Live
- Run `auval -v aufx MvUl MpAu` for AU validation
- Verify state persistence for all new parameters
- Profile CPU usage for voice limiting/oversampling

---

## Implementation Rules (Per `AI_RULES.md`)
- Reuse existing JUCE 8 APIs, no new external libraries
- Follow existing code conventions: angle-bracket JUCE includes, C++17, JUCE 8 deprecation workarounds
- Minimal changes: Extend existing SynthEngine/LayerManager/ModulationMatrix infrastructure
- No full repo scans: Use context from `AI_HANDOFF.md`/`AI_STATE.md` for file locations
- All new parameters added to APVTS for DAW automation
- State persistence via existing XML/ValueTree system
