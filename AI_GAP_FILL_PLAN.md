# MultiverseUltimate Gap-Fill Implementation Plan
## Preamble: Clarifying Questions (resolve before implementation)
1. Should unlimited oscillators be **per-voice** (dynamic count per SynthEngine voice) or **per-layer** (global oscillator count for all voices in a layer)?
2. Should new synthesis engines (Additive/Phase Distortion/Analog/Digital) be implemented as **per-oscillator types** (extending current `Oscillator` class) or **standalone engines** (like GranularEngine)?
3. For multi-output support: Do you need separate output buses for individual oscillators, voices, or entire layers?
4. For 1000+ presets: Should these be generated programmatically (via randomization) or manually curated? Do you have existing preset content to bundle?
5. For standalone mode: Do you want a separate executable (JUCE standalone target) or a VST3/AU that can load as an audio effect?

---

## Phase 0: Existing Functionality Verification (Priority: Critical)
- Verify ModulationMatrix LFO advancement and target routing (resolve discrepancy between `AI_STATE.md` and `MULTIVERSE_ULTIMATE_SPEC.md`)
- Verify SamplerEngine/DrumSequencer audio output wiring
- Confirm per-step velocity works in Sequencer/DrumSequencer

---

## Phase1: Core Synthesis & Oscillator Upgrades (Priority: High)
**Dependencies**: None (core engine work)
1.1 Refactor SynthEngine for dynamic oscillator count
   - Replace fixed `OscSettings[3]` array with `std::vector<OscState>`
   - Add "Add Oscillator" button to SynthPanel per brief
   - Update state persistence for variable oscillator counts
1.2 Add missing oscillator types (extend `OscillatorType` enum):
   - Additive: Sum of configurable sine partials with level control
   - Phase Distortion: Linear ramp bent via user-defined breakpoints
   - Analog Modeling: Emulated analog drift, jitter, wave shape variations
   - Digital: Fixed digital waveforms with FM/PD hybrid mode
1.3 Per-oscillator features
   - Add wave shaping (drive/fold/clip) per oscillator
   - Add self-oscillation feedback parameter per oscillator
1.4 Update ModulationMatrix to support new oscillator targets

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
