# Multiverse Synth: Complete Development Guide

A step-by-step guide to building a professional polyphonic synthesizer using JUCE 8.0.12 and Pamplejuce.

---

## Phase 1: Project Setup

### Step 1.1: Initialize the Project

```bash
cd /Users/jason/MultiphaseAudio/MultiverseUltimate/MultiverseUltimate

# Open the CMakeLists.txt and modify:
```

**Edit CMakeLists.txt:**
```cmake
# Change these values:
set(PROJECT_NAME "Multiverse")
set(PRODUCT_NAME "Multiverse Synth")
set(COMPANY_NAME "MultiphaseAudio")
set(BUNDLE_ID "com.multiphaseaudio.multiverse")
set(PLUGIN_MANUFACTURER_CODE "Mltv")
set(PLUGIN_CODE "M001")
set(FORMATS Standalone AU VST3 AUv3)
```

### Step 1.2: Enable Modules in CMakeLists.txt

```cmake
# Add these to your target_link_libraries section:
target_link_libraries(SharedCode
    INTERFACE
    Assets
    melatonin_inspector
    juce_audio_utils
    juce_audio_processors
    juce_dsp
    juce_gui_basics
    juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    # Add FFTw for spectral analysis (optional)
)
```

### Step 1.3: Build Shell Project

```bash
cmake -B Builds -DCMAKE_BUILD_TYPE=Release
cmake --build Builds --config Release
```

---

## Phase 2: Core Architecture

### Step 2.1: Define Voice Architecture

**Create file: `source/Synth/Oscillator.h`**
```cpp
#pragma once
#include <juce_dsp/juce_dsp.h>

enum class WaveformType
{
    Sine,
    Saw,
    Square,
    Triangle,
    Noise,
    Sample
};

class Oscillator
{
public:
    Oscillator();
    void setFrequency(float freq);
    void setWaveform(WaveformType type);
    void setPulseWidth(float width);
    float process();
    void reset();
    
private:
    float frequency = 440.0f;
    float phase = 0.0f;
    float pulseWidth = 0.5f;
    WaveformType waveform = WaveformType::Sine;
    float sampleRate = 44100.0f;
};
```

**Create file: `source/Synth/Oscillator.cpp`**
```cpp
#include "Oscillator.h"
#include <cmath>

Oscillator::Oscillator() : phase(0.0f), sampleRate(44100.0f) {}

void Oscillator::setFrequency(float freq)
{
    frequency = freq;
}

void Oscillator::setWaveform(WaveformType type)
{
    waveform = type;
}

void Oscillator::setPulseWidth(float width)
{
    pulseWidth = juce::jlimit(0.01f, 0.99f, width);
}

float Oscillator::process()
{
    float output = 0.0f;
    const float phaseIncrement = frequency / sampleRate;
    
    switch (waveform)
    {
        case WaveformType::Sine:
            output = std::sin(phase * 2.0f * M_PI);
            break;
            
        case WaveformType::Saw:
            output = 2.0f * (phase - 0.5f);
            break;
            
        case WaveformType::Square:
            output = phase < pulseWidth ? 1.0f : -1.0f;
            break;
            
        case WaveformType::Triangle:
            output = 4.0f * std::abs(phase - 0.5f) - 1.0f;
            break;
            
        case WaveformType::Noise:
            output = (float)rand() / RAND_MAX * 2.0f - 1.0f;
            break;
    }
    
    phase += phaseIncrement;
    if (phase >= 1.0f) phase -= 1.0f;
    
    return output;
}

void Oscillator::reset()
{
    phase = 0.0f;
}
```

### Step 2.2: Define Voice Class

**Create file: `source/Synth/Voice.h`**
```cpp
#pragma once
#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

class Voice
{
public:
    Voice();
    
    void noteOn(int midiNote, float velocity);
    void noteOff();
    bool isActive() const;
    
    void setEnvelopeParams(float a, float d, float s, float r);
    void setFilterParams(float cutoff, float resonance);
    
    float process();
    
    int getMidiNote() const { return midiNote; }
    float getVelocity() const { return velocity; }
    
private:
    Oscillator oscillator;
    Envelope envelope;
    Filter filter;
    
    int midiNote = -1;
    float velocity = 0.0f;
    bool active = false;
    float output = 0.0f;
};
```

**Create file: `source/Synth/Voice.cpp`**
```cpp
#include "Voice.h"

Voice::Voice() {}

void Voice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    active = true;
    
    float freq = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    oscillator.setFrequency(freq);
    envelope.triggerAttack();
}

void Voice::noteOff()
{
    envelope.triggerRelease();
}

bool Voice::isActive() const
{
    return active && !envelope.isFinished();
}

void Voice::setEnvelopeParams(float a, float d, float s, float r)
{
    envelope.setParameters(a, d, s, r);
}

void Voice::setFilterParams(float cutoff, float resonance)
{
    filter.setCutoff(cutoff);
    filter.setResonance(resonance);
}

float Voice::process()
{
    if (!active) return 0.0f;
    
    output = oscillator.process();
    output = envelope.process(output);
    output = filter.process(output);
    
    if (envelope.isFinished())
    {
        active = false;
    }
    
    return output;
}
```

### Step 2.3: Create Sound Engine

**Create file: `source/Synth/SynthEngine.h`**
```cpp
#pragma once
#include "Voice.h"
#include <array>

constexpr int MAX_VOICES = 16;

class SynthEngine
{
public:
    SynthEngine();
    
    void prepare(double sampleRate, int samplesPerBlock);
    
    void noteOn(int midiNote, float velocity);
    void noteOff(int midiNote);
    void allNotesOff();
    
    void setMasterVolume(float volume);
    void setPitchBend(float cents);
    
    float process();
    
    int getActiveVoiceCount() const;
    
private:
    int findFreeVoice() const;
    int findVoiceForNote(int midiNote) const;
    
    std::array<Voice, MAX_VOICES> voices;
    float masterVolume = 0.5f;
    float sampleRate = 44100.0f;
    double pitchBend = 0.0;
};
```

**Create file: `source/Synth/SynthEngine.cpp`**
```cpp
#include "SynthEngine.h"
#include <cmath>

SynthEngine::SynthEngine() {}

void SynthEngine::prepare(double sr, int)
{
    sampleRate = sr;
    for (auto& voice : voices)
    {
        (void)voice;  // Prepare voice if needed
    }
}

void SynthEngine::noteOn(int note, float velocity)
{
    // First, check if this note is already playing
    auto* voice = findVoiceForNote(note);
    if (voice != nullptr)
    {
        voice->noteOff();
    }
    
    // Find a free voice or steal the oldest
    voice = findFreeVoice();
    if (voice != nullptr)
    {
        voice->noteOn(note, velocity);
    }
}

void SynthEngine::noteOff(int note)
{
    auto* voice = findVoiceForNote(note);
    if (voice != nullptr)
    {
        voice->noteOff();
    }
}

void SynthEngine::allNotesOff()
{
    for (auto& voice : voices)
    {
        if (voice.isActive())
        {
            voice.noteOff();
        }
    }
}

void SynthEngine::setMasterVolume(float volume)
{
    masterVolume = juce::jlimit(0.0f, 1.0f, volume);
}

void SynthEngine::setPitchBend(float cents)
{
    pitchBend = cents;
}

float SynthEngine::process()
{
    float output = 0.0f;
    int activeCount = 0;
    
    for (auto& voice : voices)
    {
        if (voice.isActive())
        {
            output += voice.process();
            activeCount++;
        }
    }
    
    if (activeCount > 0)
    {
        output /= activeCount;
    }
    
    return output * masterVolume;
}

int SynthEngine::findFreeVoice() const
{
    for (auto& voice : voices)
    {
        if (!voice.isActive())
        {
            return const_cast<Voice&>(voice).getMidiNote();  // Placeholder
        }
    }
    return -1;
}

int SynthEngine::findVoiceForNote(int note) const
{
    for (auto& voice : voices)
    {
        if (voice.isActive() && voice.getMidiNote() == note)
        {
            return const_cast<Voice&>(voice).getMidiNote();  // Placeholder
        }
    }
    return -1;
}

int SynthEngine::getActiveVoiceCount() const
{
    int count = 0;
    for (auto& voice : voices)
    {
        if (voice.isActive()) count++;
    }
    return count;
}
```

---

## Phase 3: Effects Processing

### Step 3.1: Create Effect Base Class

**Create file: `source/Effects/Effect.h`**
```cpp
#pragma once

class Effect
{
public:
    virtual ~Effect() = default;
    
    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
    virtual float process(float input) = 0;
    virtual void reset() = 0;
    
    virtual void setBypass(bool bypass) { bypassed = bypass; }
    bool isBypassed() const { return bypassed; }
    
protected:
    double sampleRate = 44100.0f;
    bool bypassed = false;
};
```

### Step 3.2: Create Common Effects

**Create file: `source/Effects/Delay.h`**
```cpp
#pragma once
#include "Effect.h"
#include <juce_dsp/juce_dsp.h>

class Delay : public Effect
{
public:
    Delay();
    
    void prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void reset() override;
    
    void setTime(float seconds);
    void setFeedback(float feedback);
    void setMix(float mix);
    
private:
    juce::dsp::DelayLine<float> delayLine;
    float time = 0.5f;
    float feedback = 0.3f;
    float mix = 0.5f;
};
```

**Create file: `source/Effects/Delay.cpp`**
```cpp
#include "Delay.h"

Delay::Delay()
{
    delayLine.setMaximumDelayInSamples(44100);  // 1 second max
}

void Delay::prepare(double sr, int)
{
    sampleRate = sr;
    delayLine.reset();
}

float Delay::process(float input)
{
    if (bypassed) return input;
    
    auto delayed = delayLine.processSample(input);
    delayLine.setFeedback(feedback);
    
    return mix * delayed + (1.0f - mix) * input;
}

void Delay::reset()
{
    delayLine.reset();
}

void Delay::setTime(float seconds)
{
    time = juce::jlimit(0.0f, 1.0f, seconds);
    delayLine.setDelay(sampleRate * time);
}

void Delay::setFeedback(float fb)
{
    feedback = juce::jlimit(0.0f, 0.95f, fb);
}

void Delay::setMix(float m)
{
    mix = juce::jlimit(0.0f, 1.0f, m);
}
```

### Step 3.3: Create Reverb Effect

**Create file: `source/Effects/Reverb.h`**
```cpp
#pragma once
#include "Effect.h"
#include <juce_dsp/juce_dsp.h>

class Reverb : public Effect
{
public:
    Reverb();
    
    void prepare(double sampleRate, int samplesPerBlock) override;
    float process(float input) override;
    void reset() override;
    
    void setRoomSize(float size);
    void setDamping(float damping);
    void setWetLevel(float wet);
    void setDryLevel(float dry);
    
private:
    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters params;
};
```

**Create file: `source/Effects/Reverb.cpp`**
```cpp
#include "Reverb.h"

Reverb::Reverb()
{
    params.roomSize = 0.5f;
    params.damping = 0.5f;
    params.wetLevel = 0.3f;
    params.dryLevel = 0.6f;
    params.width = 1.0f;
    params.freezeMode = false;
}

void Reverb::prepare(double sr, int)
{
    (void)sr;
    reverb.setParameters(params);
}

float Reverb::process(float input)
{
    if (bypassed) return input;
    return reverb.processSample(input);
}

void Reverb::reset()
{
    reverb.reset();
}

void Reverb::setRoomSize(float size)
{
    params.roomSize = juce::jlimit(0.0f, 1.0f, size);
    reverb.setParameters(params);
}

void Reverb::setDamping(float damp)
{
    params.damping = juce::jlimit(0.0f, 1.0f, damp);
    reverb.setParameters(params);
}

void Reverb::setWetLevel(float wet)
{
    params.wetLevel = juce::jlimit(0.0f, 1.0f, wet);
    reverb.setParameters(params);
}

void Reverb::setDryLevel(float dry)
{
    params.dryLevel = juce::jlimit(0.0f, 1.0f, dry);
    reverb.setParameters(params);
}
```

---

## Phase 4: Preset System

### Step 4.1: Create Preset Class

**Create file: `source/Presets/Preset.h`**
```cpp
#pragma once
#include <juce_core/juce_core.h>

class Preset
{
public:
    Preset();
    explicit Preset(const juce::String& name);
    
    juce::String getName() const;
    void setName(const juce::String& name);
    
    juce::String getCategory() const;
    void setCategory(const juce::String& category);
    
    juce::String getAuthor() const;
    void setAuthor(const juce::String& author);
    
    void setParameter(int index, float value);
    float getParameter(int index) const;
    
    void serialize(juce::XmlElement& xml) const;
    bool deserialize(const juce::XmlElement& xml);
    
private:
    juce::String name = "Init";
    juce::String category = "Basic";
    juce::String author = "MultiphaseAudio";
    std::vector<float> parameters;
};
```

**Create file: `source/Presets/Preset.cpp`**
```cpp
#include "Preset.h"

Preset::Preset() {}

Preset::Preset(const juce::String& name) : name(name) {}

juce::String Preset::getName() const { return name; }
void Preset::setName(const juce::String& n) { name = n; }

juce::String Preset::getCategory() const { return category; }
void Preset::setCategory(const juce::String& c) { category = c; }

juce::String Preset::getAuthor() const { return author; }
void Preset::setAuthor(const juce::String& a) { author = a; }

void Preset::setParameter(int index, float value)
{
    if (index >= (int)parameters.size())
        parameters.resize(index + 1);
    parameters[index] = value;
}

float Preset::getParameter(int index) const
{
    if (index < (int)parameters.size())
        return parameters[index];
    return 0.0f;
}

void Preset::serialize(juce::XmlElement& xml) const
{
    xml.setAttribute("name", name);
    xml.setAttribute("category", category);
    xml.setAttribute("author", author);
    
    for (int i = 0; i < (int)parameters.size(); i++)
    {
        xml.setAttribute("param" + juce::String(i), parameters[i]);
    }
}

bool Preset::deserialize(const juce::XmlElement& xml)
{
    name = xml.getStringAttribute("name", "Init");
    category = xml.getStringAttribute("category", "Basic");
    author = xml.getStringAttribute("author", "MultiphaseAudio");
    
    parameters.clear();
    for (int i = 0; xml.hasAttribute("param" + juce::String(i)); i++)
    {
        parameters.push_back(xml.getFloatAttribute("param" + juce::String(i), 0.0f));
    }
    
    return true;
}
```

### Step 4.2: Create Preset Manager

**Create file: `source/Presets/PresetManager.h`**
```cpp
#pragma once
#include "Preset.h"
#include <vector>

class PresetManager
{
public:
    PresetManager();
    
    void loadPreset(const juce::String& path);
    void savePreset(const juce::String& path);
    
    void addPreset(const Preset& preset);
    const Preset& getCurrentPreset() const;
    Preset& getCurrentPreset();
    
    void nextPreset();
    void previousPreset();
    
    std::vector<Preset> getPresets() const { return presets; }
    
private:
    std::vector<Preset> presets;
    int currentPresetIndex = 0;
    juce::File presetsDirectory;
};
```

---

## Phase 5: Plugin Processor Integration

### Step 5.1: Update PluginProcessor.h

```cpp
#pragma once

#if JUCE_MODULE_AVAILABLE_juce_audio_processors
#include <juce_audio_processors/juce_audio_processors.h>
#endif

#include "../Synth/SynthEngine.h"
#include "../Effects/Delay.h"
#include "../Effects/Reverb.h"
#include "../Presets/PresetManager.h"

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
    void processBlock(juce::AudioBuffer<float>& buffer, 
                     juce::MidiBuffer& midiMessages) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    const juce::String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    
    double getTailLengthSeconds() const override;
    
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    void getStateInformation(juce::MemoryBlock& data) override;
    void setStateInformation(const void* data, int size) override;
    
private:
    SynthEngine synthEngine;
    Delay delay;
    Reverb reverb;
    PresetManager presetManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
```

### Step 5.2: Update PluginProcessor.cpp

```cpp
#include "PluginProcessor.h"

PluginProcessor::PluginProcessor()
{
    addParameter(new juce::AudioParameterFloat("volume", "Volume", 
        juce::NormalisableRange(0.0f, 1.0f), 0.7f));
    addParameter(new juce::AudioParameterFloat("cutoff", "Filter Cutoff", 
        juce::NormalisableRange(20.0f, 20000.0f), 20000.0f));
    addParameter(new juce::AudioParameterFloat("resonance", "Filter Resonance", 
        juce::NormalisableRange(0.0f, 1.0f), 0.0f));
}

PluginProcessor::~PluginProcessor() {}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synthEngine.prepare(sampleRate, samplesPerBlock);
    delay.prepare(sampleRate, samplesPerBlock);
    reverb.prepare(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources() {}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                   juce::MidiBuffer& midiMessages)
{
    // Handle MIDI
    for (const auto& metadata : midiMessages)
    {
        const auto* message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            synthEngine.noteOn(message.getNoteNumber(), 
                message.getVelocity() / 127.0f);
        }
        else if (message.isNoteOff())
        {
            synthEngine.noteOff(message.getNoteNumber());
        }
    }
    
    // Process audio
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        auto* data = buffer.getWritePointer(channel);
        
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sample = synthEngine.process();
            sample = delay.process(sample);
            sample = reverb.process(sample);
            data[i] = sample;
        }
    }
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

bool PluginProcessor::hasEditor() const { return true; }

const juce::String PluginProcessor::getName() const 
{ 
    return JucePlugin_Name; 
}

bool PluginProcessor::acceptsMidi() const { return true; }
bool PluginProcessor::producesMidi() const { return false; }

double PluginProcessor::getTailLengthSeconds() const { return 2.0f; }

int PluginProcessor::getNumPrograms() { return 128; }
int PluginProcessor::getCurrentProgram() { return 0; }

void PluginProcessor::setCurrentProgram(int index) { (void)index; }

const juce::String PluginProcessor::getProgramName(int index) 
{ 
    return "Program " + juce::String(index); 
}

void PluginProcessor::changeProgramName(int index, const juce::String& newName)
{
    (void)index;
    (void)newName;
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& data)
{
    // Save state
    juce::XmlElement xml("MULTIVERSE_STATE");
    xml.setAttribute("version", 1);
    xml.setAttribute("volume", *getParameter("volume"));
    copyXmlToBinary(xml, data);
}

void PluginProcessor::setStateInformation(const void* data, int size)
{
    // Load state
    auto xml = parseXml(juce::MemoryBlock(data, (size_t)size));
    if (xml && xml->hasAttribute("version"))
    {
        // Restore parameters
    }
}
```

---

## Phase 6: Building and Testing

### Step 6.1: Compile

```bash
cd /Users/jason/MultiphaseAudio/MultiverseUltimate/MultiverseUltimate
cmake -B Builds -DCMAKE_BUILD_TYPE=Release
cmake --build Builds --config Release
```

### Step 6.2: Test

Open your DAW and scan for VST3/AU plugins. You should see "Multiverse Synth" listed.

---

## Phase 7: UI Development (Serum-inspired)

**Create file: `source/UI/MainPanel.h`**
```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class MainPanel : public juce::Component
{
public:
    MainPanel();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    juce::Label titleLabel;
    juce::GroupComponent oscillatorGroup;
    juce::GroupComponent filterGroup;
    juce::GroupComponent effectsGroup;
};
```

---

## Quick Reference: Core Files to Create

```
source/
├── MultiverseSynth.h           # Main include
├── PluginProcessor.h          # Audio engine
├── PluginProcessor.cpp
├── PluginEditor.h             # UI
├── PluginEditor.cpp
├── Synth/
│   ├── Oscillator.h
│   ├── Oscillator.cpp
│   ├── Voice.h
│   ├── Voice.cpp
│   ├── SynthEngine.h
│   ├── SynthEngine.cpp
│   ├── Envelope.h
│   ├── Envelope.cpp
│   ├── Filter.h
│   └── Filter.cpp
├── Effects/
│   ├── Effect.h
│   ├── Delay.h
│   ├── Delay.cpp
│   ├── Reverb.h
│   ├── Reverb.cpp
│   ├── Distortion.h
│   ├── Distortion.cpp
│   ├── Chorus.h
│   ├── Chorus.cpp
│   ├── Phaser.h
│   └── Phaser.cpp
└── Presets/
    ├── Preset.h
    ├── Preset.cpp
    ├── PresetManager.h
    └── PresetManager.cpp
```

---

## Build Commands Reference

```bash
# Release build
cmake -B Builds -DCMAKE_BUILD_TYPE=Release
cmake --build Builds --config Release

# Debug build
cmake -B Builds -DCMAKE_BUILD_TYPE=Debug
cmake --build Builds --config Debug

# With Ninja (faster)
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release

# Universal binary (macOS)
cmake -B Builds -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

---

## Extension Points for Advanced Features

### Wavetable Synthesis
- Load .wav/.fft files as wavetables
- Implement wavetable interpolation
- Add morph/import features

### Granular Synthesis
- Grain scheduler with randomisation
- Grain size/overlap controls
- Pitch/time stretch

### FM Synthesis
- Operator hierarchy (carrier/modulator)
- Algorithm presets
- Feedback routing

### Sampler
- Audio file loading with drag-drop
- Key mapping/velocity layers
- Loop points and crossfades

### Sequencer/Arpeggiator
- Step sequencer grid
- Pattern storage
- MIDI export

### Drum Sequencer
- Per-track synthesis
- Pattern editor
- Sample loading

### Modulation Matrix
- Visual routing
- Drag-drop assignment
- Signal flow display