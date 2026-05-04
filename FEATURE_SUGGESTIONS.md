# Feature Suggestions — MultiverseUltimate
**Last updated: 2026-05-04**

## ✅ Shipped

### Synth
- ✅ Unison / Detune (stacked voices)
- ✅ Voice modes: Poly / Mono / Legato
- ✅ Portamento / glide (per-sample, pitch-linear)
- ✅ FM synthesis (4 operators, 8 algorithms)
- ✅ Wavetable oscillators (3 per voice, LOAD WT + EDIT WT)
- ✅ MPE support (per-note bend ±48 st, pressure, slide CC74)
- ✅ Filter oversampling (Off / 2x / 4x / Auto)
- ✅ Sub oscillator (dedicated, 1 oct below, Sine/Square)
- ✅ Noise oscillator (dedicated, LP color filter 200–20kHz)
- ✅ Filter topology selector: LP / HP / BP / Notch
- ✅ Unison spread modes: Stacked / Chord / Random

### Effects
- ✅ Chorus, Distortion/Saturation, 3-Band EQ, Compressor, Delay, Reverb
- ✅ Effect chain reordering (drag-to-reorder)
- ✅ Reverb: pre-delay, LF damping, stereo width, freeze
- ✅ Per-layer independent effect chains

### Drum Sequencer
- ✅ Swing per track
- ✅ Step probability (ProSequencer)
- ✅ Mute groups (via solo/mute per track)

### Sampler
- ✅ Zone-based playback
- ✅ Chromatic mode
- ✅ Per-zone pitch tuning (±24 semitones, 0.1 resolution)
- ✅ Per-zone playback speed (0.25×–4×, independent of pitch mapping)
- ✅ Per-zone Lo/Hi key range editing (combos in SamplerPanel)
- ✅ Per-zone Lo/Hi velocity range editing (sliders in SamplerPanel)
- ✅ Auto Map button (distributes n zones evenly across MIDI 0–127)

### Modulation
- ✅ LFO → any parameter (ModulationMatrix)
- ✅ Velocity, Note Number, Random / S&H, Envelope Follower sources
- ✅ MPE Pressure + MPE Slide mod sources
- ✅ Macro controls (8 knobs, DAW-automatable, right-click assign)
- ✅ MIDI Learn (CC / pitch-bend / channel pressure → any param)

### Layers
- ✅ 8-layer engine (Synth / Granular / Sampler per layer)
- ✅ Level / Pan / Mute / Solo per layer
- ✅ Layer key ranges (lo/hi note filter)
- ✅ Layer velocity ranges (lo/hi velocity filter)
- ✅ Layer MIDI channel filter (per-layer multitimbral)
- ✅ Per-layer independent effect chain (Chorus/Dist/EQ/Comp/Delay/Reverb)

### UX
- ✅ Preset browser with categories (Factory / User banks)
- ✅ Undo / Redo (Cmd+Z / Cmd+Shift+Z)
- ✅ Tooltips on all controls
- ✅ Neumorphic Dark Forge UI (CyberpunkTheme)
- ✅ Real-time oscilloscope + FFT spectrum (SynthDisplay)
- ✅ WavetableEditor (draw, formula gen, import)

---

## 🔜 Remaining / Next

### Oscillator (Phase 1 — shipped 2026-05-04)
- ✅ Dynamic 1–8 oscillators per voice (global oscCount param, + OSC / - OSC buttons)
- ✅ Additive synthesis (8-harmonic Fourier series per osc)
- ✅ Phase Distortion synthesis (sinusoidal phase bending)
- ✅ Analog modeling osc (LCG micro-drift)
- ✅ Digital osc (16-level bit reduction)
- ✅ Per-osc wave shaping: Drive / Fold / Clip
- ✅ Per-osc self-oscillation feedback

### Future Ideas
- Ring modulator
- XY pad modulation source
- Sampler: round-robin, key switching
- Sidechain input
- Multi-out routing
- Chord mode: user-selectable chord type (minor, dom7, etc.)
