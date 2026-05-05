# Graph Report - .  (2026-05-06)

## Corpus Check
- Large corpus: 4003 files · ~5,024,555 words. Semantic extraction will be expensive (many Claude tokens). Consider running on a subfolder, or use --no-semantic to run AST-only.

## Summary
- 987 nodes · 1278 edges · 89 communities (81 shown, 8 thin omitted)
- Extraction: 99% EXTRACTED · 1% INFERRED · 0% AMBIGUOUS · INFERRED: 19 edges (avg confidence: 0.83)
- Token cost: 18,200 input · 3,100 output

## Community Hubs (Navigation)
- [[_COMMUNITY_SynthEngine Core|SynthEngine Core]]
- [[_COMMUNITY_Project Docs & Architecture|Project Docs & Architecture]]
- [[_COMMUNITY_DrumSequencer Engine|DrumSequencer Engine]]
- [[_COMMUNITY_Plugin Host Bridge|Plugin Host Bridge]]
- [[_COMMUNITY_Voice & Oscillator Processing|Voice & Oscillator Processing]]
- [[_COMMUNITY_Sequencer & MIDI|Sequencer & MIDI]]
- [[_COMMUNITY_Modulation Matrix|Modulation Matrix]]
- [[_COMMUNITY_Pro Sequencer|Pro Sequencer]]
- [[_COMMUNITY_Pattern Engine|Pattern Engine]]
- [[_COMMUNITY_Preset Browser UI|Preset Browser UI]]
- [[_COMMUNITY_Drum Sequencer UI|Drum Sequencer UI]]
- [[_COMMUNITY_Wavetable Oscillator|Wavetable Oscillator]]
- [[_COMMUNITY_Arpeggiator|Arpeggiator]]
- [[_COMMUNITY_LFO Shape Editor|LFO Shape Editor]]
- [[_COMMUNITY_Sampler Engine|Sampler Engine]]
- [[_COMMUNITY_Preset Manager|Preset Manager]]
- [[_COMMUNITY_Filter DSP|Filter DSP]]
- [[_COMMUNITY_Sequencer Panel UI|Sequencer Panel UI]]
- [[_COMMUNITY_Factory Presets|Factory Presets]]
- [[_COMMUNITY_Cyberpunk Theme LookAndFeel|Cyberpunk Theme LookAndFeel]]
- [[_COMMUNITY_Reverb Effect|Reverb Effect]]
- [[_COMMUNITY_LFO Draw Panel UI|LFO Draw Panel UI]]
- [[_COMMUNITY_Sampler Panel UI|Sampler Panel UI]]
- [[_COMMUNITY_Pro Sequencer Panel UI|Pro Sequencer Panel UI]]
- [[_COMMUNITY_UI Design Docs (FigmaCyberpunk)|UI Design Docs (Figma/Cyberpunk)]]
- [[_COMMUNITY_Effects Panel UI|Effects Panel UI]]
- [[_COMMUNITY_Macro Manager|Macro Manager]]
- [[_COMMUNITY_Preset Data Model|Preset Data Model]]
- [[_COMMUNITY_Plugin Editor (Main UI)|Plugin Editor (Main UI)]]
- [[_COMMUNITY_Layer Effect Chain|Layer Effect Chain]]
- [[_COMMUNITY_Layer Engine|Layer Engine]]
- [[_COMMUNITY_Synth Panel UI|Synth Panel UI]]
- [[_COMMUNITY_Granular DSP (voicesgrains)|Granular DSP (voices/grains)]]
- [[_COMMUNITY_Layer Manager|Layer Manager]]
- [[_COMMUNITY_Modulation Matrix Panel UI|Modulation Matrix Panel UI]]
- [[_COMMUNITY_Envelope DSP|Envelope DSP]]
- [[_COMMUNITY_EQ Effect|EQ Effect]]
- [[_COMMUNITY_Layers Panel UI|Layers Panel UI]]
- [[_COMMUNITY_Compressor Effect|Compressor Effect]]
- [[_COMMUNITY_MIDI Learn & Macro Binding|MIDI Learn & Macro Binding]]
- [[_COMMUNITY_Granular Voice DSP|Granular Voice DSP]]
- [[_COMMUNITY_FM Operator|FM Operator]]
- [[_COMMUNITY_Classic Oscillator|Classic Oscillator]]
- [[_COMMUNITY_Delay Effect|Delay Effect]]
- [[_COMMUNITY_Chorus Effect|Chorus Effect]]
- [[_COMMUNITY_Distortion Effect|Distortion Effect]]
- [[_COMMUNITY_Granular Panel UI|Granular Panel UI]]
- [[_COMMUNITY_Sampler Voice|Sampler Voice]]
- [[_COMMUNITY_Arpeggiator Panel UI|Arpeggiator Panel UI]]
- [[_COMMUNITY_NeuKnob Component|NeuKnob Component]]
- [[_COMMUNITY_Macro Panel UI|Macro Panel UI]]
- [[_COMMUNITY_DSP Priority Rules|DSP Priority Rules]]

## God Nodes (most connected - your core abstractions)
1. `noteOn()` - 17 edges
2. `noteOff()` - 17 edges
3. `noteOnMPE()` - 14 edges
4. `buildDefaults()` - 12 edges
5. `rebuildFilter()` - 11 edges
6. `nSkw()` - 11 edges
7. `MultiverseUltimate Plugin` - 11 edges
8. `LFOShapeEditor()` - 8 edges
9. `updateCoeffs()` - 8 edges
10. `updateSamplesPerStep()` - 7 edges

## Surprising Connections (you probably didn't know these)
- `Future Feature Ideas (Ring Mod, XY Pad, Round-Robin)` --semantically_similar_to--> `XY Pad (Macro1 X-axis / Macro2 Y-axis)`  [INFERRED] [semantically similar]
  FEATURE_SUGGESTIONS.md → AI_HANDOFF.md
- `Dark Forge Color Palette (bgVoid/neonCyan/neonPink/neonPurple)` --semantically_similar_to--> `Figma Color Palette Spec`  [INFERRED] [semantically similar]
  AI_CYBERPUNK_PLAN.md → FIGMA_DESIGN_BRIEF.md
- `Multiverse Synth Development Guide (Copy)` --semantically_similar_to--> `Multiverse Synth Development Guide`  [INFERRED] [semantically similar]
  Multiverse_Synth_Development_Guide copy.md → Multiverse_Synth_Development_Guide.md
- `MidiLearnSlider()` --calls--> `NeuKnob()`  [INFERRED]
  NeuKnob.h → NeuKnob.cpp
- `DSP Real-Time Safety Rule` --rationale_for--> `ModulationMatrix`  [INFERRED]
  AI_RULES.md → AI_HANDOFF.md

## Hyperedges (group relationships)
- **AI Session Context Trilogy (Rules + Handoff + State)** — ai_rules_no_scan_policy, ai_handoff_multiverse_ultimate, ai_state_document [EXTRACTED 1.00]
- **Dark Forge UI Design System (Theme + Palette + Components)** — ai_handoff_cyberpunk_theme, ai_cyberpunk_plan_palette, ai_handoff_neuknob, ai_cyberpunk_plan_neumorphic_card [EXTRACTED 0.95]
- **Audio Engine Signal Pipeline (Synth + Granular + Sampler + DrumSeq → Effects → Output)** — ai_handoff_synth_engine, ai_handoff_granular_engine, ai_handoff_sampler_engine, ai_handoff_drum_sequencer, ai_handoff_effects_chain [EXTRACTED 1.00]

## Communities (89 total, 8 thin omitted)

### Community 0 - "SynthEngine Core"
Cohesion: 0.05
Nodes (23): allNotesOff(), findFMVoiceForNote(), findFreeFMVoice(), findFreeVoice(), getActiveVoiceCount(), noteOff(), noteOffMPE(), noteOn() (+15 more)

### Community 1 - "Project Docs & Architecture"
Cohesion: 0.05
Nodes (50): Phase 9: Competitive Feature Expansion, AI Gap Fill Implementation Plan, APVTS (AudioProcessorValueTreeState) Parameter System, Chord/Strum Mode, DrumSequencer, Effect Chain (Chorus→Distortion→EQ→Compressor→Delay→Reverb), FactoryPresets (100 presets), GranularEngine (+42 more)

### Community 2 - "DrumSequencer Engine"
Cohesion: 0.06
Nodes (14): anySoloActive(), DrumSequencer(), DrumVoice(), forceStop(), handleStep(), prepare(), process(), setBPM() (+6 more)

### Community 3 - "Plugin Host Bridge"
Cohesion: 0.07
Nodes (15): applyChainEffect(), applyMidiMapping(), getParameterIDFromIndex(), getParameterIndexFromID(), getStateInformation(), handleMidiForLearn(), loadMidiMappingsFromState(), loadPresetAtIndex() (+7 more)

### Community 4 - "Voice & Oscillator Processing"
Cohesion: 0.07
Nodes (9): applyOscShaping(), noteOn(), process(), setFrequencyDirect(), setOscillatorDetune(), setPitchBend(), setSampleRate(), updateOscillatorFrequencies() (+1 more)

### Community 5 - "Sequencer & MIDI"
Cohesion: 0.1
Nodes (14): clearArpNotes(), fastRand(), getNextStepIndex(), loadPattern(), prepare(), process(), scheduleNoteOff(), Sequencer() (+6 more)

### Community 6 - "Modulation Matrix"
Cohesion: 0.08
Nodes (6): getActiveConnectionsForTarget(), getConnections(), getCustomTable(), ModulationMatrix(), prepareForBlock(), swapModSumsBuffers()

### Community 7 - "Pro Sequencer"
Cohesion: 0.11
Nodes (12): fastRand01(), getNextStep(), prepare(), process(), retriggerNote(), scheduleNextStep(), sendNoteOff(), setBPM() (+4 more)

### Community 8 - "Pattern Engine"
Cohesion: 0.14
Nodes (14): fastRand01(), generateNote(), noteOff(), PatternEngine(), prepare(), process(), scheduleNextStep(), selectNoteIndex() (+6 more)

### Community 9 - "Preset Browser UI"
Cohesion: 0.17
Nodes (16): buttonClicked(), comboBoxChanged(), deleteSelectedPreset(), drawNeumorphicRect(), exportPreset(), importPreset(), listBoxItemDoubleClicked(), loadSelectedPreset() (+8 more)

### Community 10 - "Drum Sequencer UI"
Cohesion: 0.13
Nodes (10): advanceChain(), DrumSequencerPanel(), refreshStepDisplay(), resized(), showTrackFXPopup(), showVelocityMenu(), timerCallback(), TrackRow::TrackRow() (+2 more)

### Community 11 - "Wavetable Oscillator"
Cohesion: 0.11
Nodes (7): generateStandardTable(), generateTables(), normalizeFrame(), process(), processFFT(), readTable(), WavetableOscillator()

### Community 12 - "Arpeggiator"
Cohesion: 0.16
Nodes (16): clearHeldNotes(), fastRand01(), noteOn(), prepare(), process(), scheduleNextStep(), selectNoteIndex(), sendAllNoteOffs() (+8 more)

### Community 13 - "LFO Shape Editor"
Cohesion: 0.2
Nodes (14): fillSaw(), fillSine(), fillSquare(), fillTri(), LFOShapeEditor(), mouseDown(), mouseDrag(), mouseUp() (+6 more)

### Community 14 - "Sampler Engine"
Cohesion: 0.16
Nodes (13): addZone(), allNotesOff(), clearZones(), findFreeVoice(), findVoiceForNote(), noteOff(), noteOn(), prepareZonesForPlayback() (+5 more)

### Community 15 - "Preset Manager"
Cohesion: 0.16
Nodes (10): addPreset(), createFactoryPresetsIfNeeded(), deletePreset(), getBankDirectory(), importPreset(), loadPreset(), PresetManager(), saveState() (+2 more)

### Community 16 - "Filter DSP"
Cohesion: 0.21
Nodes (15): Filter(), prepareFilter(), prepareNotchFilter(), process(), process2x(), process4x(), processNotch(), processNotch2x() (+7 more)

### Community 17 - "Sequencer Panel UI"
Cohesion: 0.17
Nodes (12): detectChord(), detectChordFromClasses(), exportMidi(), filesDropped(), importMidiFile(), noteName(), paint(), refreshStepDisplay() (+4 more)

### Community 18 - "Factory Presets"
Cohesion: 0.28
Nodes (16): A(), buildAllPresets(), buildDefaults(), buildPresetXml(), D(), DT(), FC(), FR() (+8 more)

### Community 19 - "Cyberpunk Theme LookAndFeel"
Cohesion: 0.15
Nodes (8): CyberpunkTheme(), drawButtonBackground(), drawComboBox(), drawLinearSlider(), drawNeumorphicEllipse(), drawNeumorphicRect(), drawRotarySlider(), drawToggleButton()

### Community 20 - "Reverb Effect"
Cohesion: 0.14
Nodes (6): applyPreDelay(), prepare(), processBlock(), ReverbEffect(), setLFDamping(), updateHPCoeff()

### Community 21 - "LFO Draw Panel UI"
Cohesion: 0.18
Nodes (10): applyDrawPoints(), buttonClicked(), drawWaveform(), importWavetable(), mouseDown(), mouseDrag(), mouseMove(), mouseUp() (+2 more)

### Community 22 - "Sampler Panel UI"
Cohesion: 0.22
Nodes (7): autoMap(), filesDropped(), loadFile(), SamplerPanel(), updateControlsForSelectedZone(), updateZoneList(), ZoneListModel::listBoxItemClicked()

### Community 23 - "Pro Sequencer Panel UI"
Cohesion: 0.26
Nodes (11): applyPlayMode(), exportMidi(), noteName(), paint(), ProSequencerPanel(), refreshStepEditor(), refreshStepGrid(), selectLane() (+3 more)

### Community 24 - "UI Design Docs (Figma/Cyberpunk)"
Cohesion: 0.19
Nodes (15): Neumorphic Cyberpunk UI Plan, Neumorphic Card Component Spec, Dark Forge Color Palette (bgVoid/neonCyan/neonPink/neonPurple), CyberpunkTheme LookAndFeel, NeuKnob Component, SynthDisplay (Oscilloscope + FFT), Figma Component Naming Convention, Figma Build Guide (+7 more)

### Community 26 - "Effects Panel UI"
Cohesion: 0.21
Nodes (9): effectName(), EffectsPanel(), mouseDown(), mouseDrag(), paint(), setupLabel(), setupSlider(), slotAt() (+1 more)

### Community 29 - "Plugin Editor (Main UI)"
Cohesion: 0.26
Nodes (8): buttonClicked(), PluginEditor(), randomizeParams(), resized(), setupMidiLearnButton(), setupTabs(), showRandomizeMenu(), updateMidiLearnUI()

### Community 31 - "Layer Engine"
Cohesion: 0.2
Nodes (4): allNotesOff(), applyPan(), processBlock(), setEngineType()

### Community 32 - "Synth Panel UI"
Cohesion: 0.24
Nodes (7): drawSection(), paint(), resized(), setupLabel(), setupSlider(), SynthPanel(), updateVisibility()

### Community 33 - "Granular DSP (voices/grains)"
Cohesion: 0.25
Nodes (7): allNotesOff(), buildDefaultSourceBuffer(), GranularEngine(), loadSourceFile(), noteOff(), prepare(), setState()

### Community 34 - "Layer Manager"
Cohesion: 0.22
Nodes (4): isAnyLayerSoloed(), LayerManager(), noteOn(), processBlock()

### Community 35 - "Modulation Matrix Panel UI"
Cohesion: 0.27
Nodes (4): ModulationMatrixPanel(), rebuild(), resized(), timerCallback()

### Community 36 - "Envelope DSP"
Cohesion: 0.24
Nodes (4): Envelope(), recomputeIncrements(), setParameters(), setSampleRate()

### Community 38 - "EQ Effect"
Cohesion: 0.33
Nodes (9): makeHighShelf(), makeLowShelf(), makePeak(), prepare(), reset(), setHighGain(), setLowGain(), setMidGain() (+1 more)

### Community 39 - "Layers Panel UI"
Cohesion: 0.29
Nodes (6): createRow(), drawSection(), LayersPanel(), paint(), showFXPopup(), updateUI()

### Community 40 - "Compressor Effect"
Cohesion: 0.29
Nodes (5): prepare(), reset(), setAttack(), setRelease(), updateCoeffs()

### Community 41 - "MIDI Learn & Macro Binding"
Cohesion: 0.31
Nodes (5): checkHasMacro(), checkHasMapping(), mouseDown(), showContextMenu(), timerCallback()

### Community 42 - "Granular Voice DSP"
Cohesion: 0.33
Nodes (5): applyEnvelope(), findFreeGrain(), processBlock(), spawnGrain(), updateADSRCoeffs()

### Community 46 - "Delay Effect"
Cohesion: 0.25
Nodes (3): DelayEffect(), prepare(), reset()

### Community 47 - "Chorus Effect"
Cohesion: 0.25
Nodes (3): ChorusEffect(), prepare(), reset()

### Community 48 - "Distortion Effect"
Cohesion: 0.36
Nodes (4): prepare(), reset(), setTone(), updateLPCoeff()

### Community 49 - "Granular Panel UI"
Cohesion: 0.43
Nodes (4): GranularPanel(), loadSourceFile(), setupLabel(), setupSlider()

### Community 50 - "Sampler Voice"
Cohesion: 0.38
Nodes (4): forceStop(), getSampleAt(), prepare(), process()

### Community 51 - "Arpeggiator Panel UI"
Cohesion: 0.43
Nodes (4): ArpeggiatorPanel(), refreshStepEditor(), refreshStepGrid(), selectStep()

## Knowledge Gaps
- **16 isolated node(s):** `Minimal Changes Rule`, `Feature Priority Order (Audio → Modulation → UI → Presets)`, `SamplerEngine`, `SynthDisplay (Oscilloscope + FFT)`, `MPE Support` (+11 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **8 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `getStep()` connect `Pattern Engine` to `Sequencer & MIDI`?**
  _High betweenness centrality (0.001) - this node is a cross-community bridge._
- **What connects `Minimal Changes Rule`, `Feature Priority Order (Audio → Modulation → UI → Presets)`, `SamplerEngine` to the rest of the system?**
  _16 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `SynthEngine Core` be split into smaller, more focused modules?**
  _Cohesion score 0.05 - nodes in this community are weakly interconnected._
- **Should `Project Docs & Architecture` be split into smaller, more focused modules?**
  _Cohesion score 0.05 - nodes in this community are weakly interconnected._
- **Should `DrumSequencer Engine` be split into smaller, more focused modules?**
  _Cohesion score 0.06 - nodes in this community are weakly interconnected._
- **Should `Plugin Host Bridge` be split into smaller, more focused modules?**
  _Cohesion score 0.07 - nodes in this community are weakly interconnected._
- **Should `Voice & Oscillator Processing` be split into smaller, more focused modules?**
  _Cohesion score 0.07 - nodes in this community are weakly interconnected._