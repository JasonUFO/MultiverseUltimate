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

## Phase2: Sampler Enhancements (Priority: High)
**Dependencies**: None (separate engine)
2.1 Drag-and-drop support
   - Add `DragAndDropTarget` to SamplerPanel for audio/MIDI files
   - Implement drag-to-DAW for sampler audio/MIDI
2.2 Time-stretch & pitch-shift
   - Use JUCE `LagrangeInterpolator` for time-stretch (free, no external libs)
   - Decouple pitch-shift from time-stretch with independent controls
2.3 Multi-sampling upgrade
   - Extend `SamplerZone` with velocity/key layer support
   - Update SamplerPanel UI to display/edit velocity/key layers
2.4 MIDI pattern drag/sync with Sequencer

---

## Phase3: Sequencer & Arpeggiator Upgrades (Priority: High)
**Dependencies**: None (separate engine)
3.1 MIDI pattern drag-and-drop
   - Add `DragAndDropTarget` to SequencerPanel for MIDI files
   - Implement MIDI pattern drag-to-DAW via `juce::MidiFile` export
3.2 Smart chord tracking
   - Add chord detection for held notes (major/minor/extensions)
   - Display detected chords in SequencerPanel
3.3 Polyrhythm/polymeter
   - Add per-pattern time signature, per-step custom clock divisions
3.4 Per-step probability/conditional triggers
   - Add probability parameter to `Step` struct
   - Modify `Sequencer::process` to skip steps based on probability
3.5 Audio-rate step modulation
   - Add step-level modulation targets to ModulationMatrix
3.6 Drag MIDI to audio: Render MIDI pattern to dragable audio file

---

## Phase4: Audio Output & Drum Sequencer (Priority: Medium)
**Dependencies**: JUCE bus layout support
4.1 Unlimited audio outputs
   - Add multiple output buses via `AudioProcessor::addBus()` in PluginProcessor
   - Add output routing UI to SynthPanel/LayersPanel
4.2 Individual outs per oscillator/voice
   - Add routing controls to assign oscillators/voices to output buses
4.3 Drum Sequencer per-track FX
   - Extend `LayerEffectChain` to support per-drum-track effect chains
   - Update DrumSequencerPanel UI with per-track FX toggles/mix

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
