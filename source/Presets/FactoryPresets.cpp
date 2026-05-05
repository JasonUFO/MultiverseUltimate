#include "FactoryPresets.h"
#include <cmath>

// ─── Normalization helpers ─────────────────────────────────────────────────
// Normalized value = what JUCE stores in APVTS state XML and passes to setValueNotifyingHost()

// Linear range
static float nLin(float v, float mn, float mx) {
    return juce::jlimit(0.0f, 1.0f, (v - mn) / (mx - mn));
}
// Skewed range:  norm = pow((v-min)/(max-min), skew)
static float nSkw(float v, float mn, float mx, float sk) {
    return std::pow(juce::jlimit(0.0f, 1.0f, (v - mn) / (mx - mn)), sk);
}
// Choice param:  norm = index / (numChoices - 1)
static float nCh(int i, int n) {
    return n > 1 ? juce::jlimit(0.0f, 1.0f, (float)i / (float)(n - 1)) : 0.0f;
}
// Bool param
static float nB(bool v) { return v ? 1.0f : 0.0f; }

// Shorthand helpers for common ranges
static float A(float s)  { return nSkw(s, 0.001f, 5.0f,     0.4f); } // attack (0.001-5s)
static float D(float s)  { return nSkw(s, 0.001f, 5.0f,     0.4f); } // decay  (0.001-5s)
static float R(float s)  { return nSkw(s, 0.001f, 10.0f,    0.4f); } // release (0.001-10s)
static float FC(float hz){ return nSkw(hz, 20.0f, 20000.0f, 0.3f); } // filter cutoff (20-20kHz)
static float FR(float v) { return nSkw(v, 0.1f,  10.0f,     0.5f); } // filter resonance (0.1-10)
static float LR(float hz){ return nSkw(hz, 0.01f, 20.0f,    0.3f); } // LFO rate (0.01-20Hz)
static float PT(float s) { return nSkw(s, 0.0f,  2.0f,      0.4f); } // portamento (0-2s)
static float DT(float st){ return nSkw(st + 12.0f, 0.0f, 24.0f, 0.5f); } // detune (-12..+12 semitones)

// ─── Per-oscillator data (for SynthParams XML) ────────────────────────────
struct OscData {
    int   type     = 0;    // 0=Classic 1=Wavetable 2=Additive 3=PhaseDist 4=Analog 5=Digital
    float level    = 0.0f; // 0-1
    float detune   = 0.0f; // semitones (actual, not normalized)
    int   wave     = 1;    // 0=Sine 1=Saw 2=Square 3=Tri 4=Noise
    float wavePos  = 0.0f;
    int   shape    = 0;    // 0=Off 1=Drive 2=Fold 3=Clip
    float shapeAmt = 0.0f;
    float selfOsc  = 0.0f;
    float phaseDist= 0.5f;
};

// ─── Preset data container ─────────────────────────────────────────────────
struct PresetData {
    juce::String              name, category;
    int                       synthMode = 0; // 0=Classic 1=FM
    int                       oscCount  = 1;
    std::array<OscData, 8>    oscs;
    std::map<juce::String, float> params; // normalized APVTS overrides

    void set(const juce::String& id, float v) { params[id] = v; }

    // Convenience setters
    void setAdsr(float a, float d, float s, float r) {
        set("attack", A(a)); set("decay", D(d)); set("sustain", s); set("release", R(r));
    }
    void setFilter(float hz, float res, int type = 0) {
        set("filterCutoff", FC(hz)); set("filterResonance", FR(res));
        set("filterType", nCh(type, 4));
    }
    void setReverb(float wet, float room = 0.6f) {
        set("reverbWet", wet); set("reverbRoom", room);
    }
    void setChorus(float mix, float rate = 0.5f, float depth = 0.5f) {
        set("chorusMix", mix); set("chorusRate", nLin(rate, 0.1f, 5.0f));
        set("chorusDepth", depth);
    }
    void setDelay(float mix, float time = 0.375f, float fb = 0.35f) {
        set("delayMix", mix);
        set("delayTime", nSkw(time, 0.0f, 2.0f, 0.4f));
        set("delayFeedback", nLin(fb, 0.0f, 0.95f));
    }
    void setUnison(int voices, float detune = 20.0f, float width = 1.0f) {
        set("unisonVoices", nCh(voices - 1, 8));
        set("unisonDetune", nLin(detune, 0.0f, 100.0f));
        set("unisonWidth", width);
    }
    void setOscApvts(int oscIdx, const OscData& o) {
        juce::String s = "osc" + juce::String(oscIdx + 1);
        set(s + "Type",      nCh(o.type, 6));
        set(s + "Level",     o.level);
        set(s + "Detune",    DT(o.detune));
        set(s + "Waveform",  nCh(o.wave, 5));
        set(s + "WavePos",   o.wavePos);
        set(s + "ShapeType", nCh(o.shape, 4));
        set(s + "ShapeAmt",  o.shapeAmt);
        set(s + "SelfOsc",   o.selfOsc);
        set(s + "PhaseDist", o.phaseDist);
    }
    // Set all 8 osc APVTS params + SynthParams osc data from the oscs array
    void applyOscs() {
        set("oscCount", nCh(oscCount - 1, 8));
        for (int i = 0; i < 8; ++i)
            setOscApvts(i, oscs[i]);
    }
    void setMono(bool legato = false, float porta = 0.0f) {
        set("voiceMode", nCh(legato ? 2 : 1, 3));
        set("portamento", PT(porta));
    }
};

// ─── Build full defaults map ────────────────────────────────────────────────
static std::map<juce::String, float> buildDefaults()
{
    std::map<juce::String, float> m;

    m["masterVolume"]    = 0.7f;
    m["melodicSequencer"]= nCh(2, 3);

    // Main envelope
    m["attack"]  = A(0.01f);
    m["decay"]   = D(0.1f);
    m["sustain"] = 0.7f;
    m["release"] = R(0.5f);

    // Filter
    m["filterCutoff"]    = FC(20000.0f);
    m["filterResonance"] = FR(0.707f);
    m["filterType"]      = 0.0f;  // LP

    // Delay (off by default)
    m["delayTime"]     = nSkw(0.5f, 0.0f, 2.0f, 0.4f);
    m["delayFeedback"] = nLin(0.3f, 0.0f, 0.95f);
    m["delayMix"]      = 0.0f;

    // Reverb (off by default)
    m["reverbRoom"]     = 0.5f;
    m["reverbDamp"]     = 0.5f;
    m["reverbWet"]      = 0.0f;
    m["reverbPreDelay"] = 0.0f;
    m["reverbLFDamp"]   = 0.0f;
    m["reverbWidth"]    = 1.0f;
    m["reverbFreeze"]   = 0.0f;

    m["auxSendDelay"]  = 0.0f;
    m["auxSendReverb"] = 0.0f;

    // Chorus (off by default)
    m["chorusRate"]  = nLin(0.5f, 0.1f, 5.0f);
    m["chorusDepth"] = 0.5f;
    m["chorusMix"]   = 0.0f;

    // Distortion (off by default)
    m["distDrive"] = nSkw(10.0f, 1.0f, 100.0f, 0.3f);
    m["distTone"]  = 0.7f;
    m["distMix"]   = 0.0f;

    // EQ (flat)
    m["eqLowGain"]  = nLin(0.0f, -12.0f, 12.0f);
    m["eqMidGain"]  = nLin(0.0f, -12.0f, 12.0f);
    m["eqHighGain"] = nLin(0.0f, -12.0f, 12.0f);

    // Compressor
    m["compThreshold"] = nLin(-20.0f, -60.0f, 0.0f);
    m["compRatio"]     = nSkw(4.0f, 1.0f, 20.0f, 0.4f);
    m["compAttack"]    = nSkw(10.0f, 0.1f, 100.0f, 0.5f);
    m["compRelease"]   = nSkw(100.0f, 10.0f, 1000.0f, 0.4f);
    m["compMakeup"]    = 0.0f;

    // Sampler
    m["samplerVolume"] = nLin(1.0f, 0.0f, 2.0f);
    m["samplerPan"]    = nLin(0.0f, -1.0f, 1.0f);

    // LFOs
    for (int i = 1; i <= 8; ++i) {
        juce::String s = juce::String(i);
        m["lfo" + s + "Rate"]    = LR(1.0f);
        m["lfo" + s + "Shape"]   = 0.0f;           // Sine
        m["lfo" + s + "Sync"]    = 0.0f;
        m["lfo" + s + "SyncDiv"] = nCh(3, 8);      // 1/4
    }

    // Mod envelopes
    for (int e = 2; e <= 3; ++e) {
        juce::String s = juce::String(e);
        m["modEnv" + s + "Attack"]  = nSkw(0.01f, 0.001f, 4.0f, 0.3f);
        m["modEnv" + s + "Decay"]   = nSkw(0.1f,  0.001f, 4.0f, 0.3f);
        m["modEnv" + s + "Sustain"] = 0.7f;
        m["modEnv" + s + "Release"] = nSkw(0.2f,  0.001f, 8.0f, 0.3f);
    }

    m["fmAlgorithm"]      = 0.0f;
    m["maxVoices"]        = 1.0f;          // 16 voices
    m["metronomeEnabled"] = 0.0f;
    m["metronomeVolume"]  = 0.5f;

    // FM operators
    for (int op = 1; op <= 4; ++op) {
        juce::String s = "fmOp" + juce::String(op);
        m[s + "Ratio"]    = nSkw(1.0f, 0.5f, 32.0f, 0.5f);
        m[s + "Level"]    = 1.0f;
        m[s + "Feedback"] = 0.0f;
        m[s + "Attack"]   = A(0.01f);
        m[s + "Decay"]    = D(0.1f);
        m[s + "Sustain"]  = 0.7f;
        m[s + "Release"]  = nSkw(0.3f, 0.001f, 10.0f, 0.4f);
    }

    // Oscillators (3 active by default)
    const float DT0 = DT(0.0f); // 0 semitones detune → normalized ≈ 0.7071
    for (int osc = 1; osc <= 8; ++osc) {
        juce::String s = "osc" + juce::String(osc);
        m[s + "Type"]      = 0.0f;                          // Classic
        m[s + "Level"]     = (osc <= 3) ? 1.0f : 0.0f;
        m[s + "Detune"]    = DT0;
        m[s + "Waveform"]  = nCh(1, 5);                     // Saw
        m[s + "WavePos"]   = 0.0f;
        m[s + "ShapeType"] = 0.0f;                          // Off
        m[s + "ShapeAmt"]  = 0.0f;
        m[s + "SelfOsc"]   = 0.0f;
        m[s + "PhaseDist"] = 0.5f;
    }
    m["oscCount"] = nCh(2, 8); // index 2 = 3 oscs

    // Unison (off by default)
    m["unisonVoices"]     = 0.0f;
    m["unisonDetune"]     = nLin(20.0f, 0.0f, 100.0f);
    m["unisonWidth"]      = 1.0f;
    m["unisonSpreadMode"] = 0.0f;

    m["filterType"] = 0.0f; // LP

    // Sub / Noise osc
    m["subOscEnable"]   = 0.0f;
    m["subOscLevel"]    = 0.5f;
    m["subOscWave"]     = 0.0f;
    m["noiseOscEnable"] = 0.0f;
    m["noiseOscLevel"]  = 0.3f;
    m["noiseOscColor"]  = nSkw(5000.0f, 200.0f, 20000.0f, 0.3f);

    // Voice mode
    m["voiceMode"]   = 0.0f;  // Poly
    m["portamento"]  = 0.0f;
    m["portaAlways"] = 0.0f;
    m["mpeEnabled"]  = 0.0f;

    // Macros
    for (int i = 1; i <= 8; ++i)
        m["macro" + juce::String(i)] = 0.0f;

    // Granular
    m["granularPosition"]     = 0.0f;
    m["granularGrainSize"]    = nSkw(0.08f, 0.01f, 0.5f, 0.4f);
    m["granularSpray"]        = 0.1f;
    m["granularDensity"]      = nSkw(12.0f, 1.0f, 64.0f, 0.4f);
    m["granularPitchScatter"] = 0.0f;
    m["granularEnvShape"]     = nCh(1, 4);  // Hann
    m["granularReverse"]      = 0.0f;
    m["granularStereoSpread"] = 0.5f;
    m["granularAttack"]       = A(0.01f);
    m["granularDecay"]        = D(0.1f);
    m["granularSustain"]      = 0.7f;
    m["granularRelease"]      = R(0.5f);

    // Chord / Strum
    m["chordModeEnabled"] = 0.0f;
    m["chordShape"]       = nCh(1, 12);  // Major
    m["chordStrumDelay"]  = 0.0f;

    return m;
}

// ─── XML builder ─────────────────────────────────────────────────────────────
static juce::String buildPresetXml(const PresetData& pd,
                                   const std::map<juce::String, float>& defaults)
{
    // Merge defaults with preset overrides
    auto merged = defaults;
    for (auto& kv : pd.params)
        merged[kv.first] = kv.second;

    juce::String xml;
    xml.preallocateBytes(8192);
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xml << "<Multiverse>\n";

    // APVTSState
    xml << "  <APVTSState id=\"APVTSState\">\n";
    for (auto& kv : merged)
        xml << "    <PARAM id=\"" << kv.first << "\" value=\""
            << juce::String(kv.second, 10) << "\"/>\n";
    xml << "  </APVTSState>\n";

    // SynthParams
    xml << "  <SynthParams synthMode=\"" << pd.synthMode
        << "\" oscCount=\"" << pd.oscCount << "\">\n";
    for (int i = 0; i < 8; ++i) {
        const auto& o = pd.oscs[i];
        xml << "    <Osc index=\"" << i
            << "\" type=\""      << o.type
            << "\" level=\""     << juce::String(o.level, 6)
            << "\" detune=\""    << juce::String(o.detune, 4)
            << "\" waveform=\""  << o.wave
            << "\" wavePos=\""   << juce::String(o.wavePos, 4)
            << "\" wavetableFile=\"\""
            << " shapeType=\""   << o.shape
            << "\" shapeAmt=\""  << juce::String(o.shapeAmt, 4)
            << "\" selfOsc=\""   << juce::String(o.selfOsc, 4)
            << "\" phaseDist=\"" << juce::String(o.phaseDist, 4)
            << "\"/>\n";
    }
    // FM operators (default values)
    for (int op = 0; op < 4; ++op)
        xml << "    <FmOp index=\"" << op
            << "\" ratio=\"1.0\" level=\"1.0\" feedback=\"0.0\""
            << " attack=\"0.01\" decay=\"0.5\" sustain=\"0.7\" release=\"0.3\"/>\n";
    xml << "  </SynthParams>\n";

    xml << "  <ReverbExtra dry=\"0.0\"/>\n";
    xml << "  <EffectChain order=\"5517840\"/>\n";  // 0x543210 = Chorus→Dist→EQ→Comp→Delay→Reverb
    xml << "</Multiverse>\n";

    return xml;
}

// ─── Write helper ────────────────────────────────────────────────────────────
static void writePreset(const juce::File& factoryDir, const juce::String& category,
                        const juce::String& name, const juce::String& xml)
{
    auto f = factoryDir.getChildFile(category).getChildFile(name + ".mvpreset");
    f.replaceWithText(xml, false, false, "\n");
}

// ─── Preset factory ──────────────────────────────────────────────────────────
static std::vector<PresetData> buildAllPresets(const std::map<juce::String, float>& defs)
{
    (void)defs; // referenced through captures below
    std::vector<PresetData> presets;

    auto make = [&](const char* name, const char* cat, int mode = 0) -> PresetData& {
        PresetData pd;
        pd.name     = name;
        pd.category = cat;
        pd.synthMode= mode;
        // Default osc: 1 saw osc at full level
        pd.oscCount = 1;
        pd.oscs[0]  = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        // Oscs 1-7 silent
        for (int i = 1; i < 8; ++i) pd.oscs[i] = {0, 0.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        presets.push_back(std::move(pd));
        return presets.back();
    };

    // ────────────────────────────────── INIT ──────────────────────────────────

    {
        auto& p = make("Init", "Init");
        p.oscCount = 1;
        p.oscs[0]  = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Classic saw
        p.setAdsr(0.01f, 0.1f, 0.7f, 0.5f);
        p.setFilter(20000.0f, 0.707f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Init FM", "Init", 1);
        p.oscCount = 1;
        p.setAdsr(0.01f, 0.5f, 0.7f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Init Additive", "Init");
        p.oscCount = 1;
        p.oscs[0] = {2, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Additive
        p.setAdsr(0.01f, 0.1f, 0.7f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Init PhaseDist", "Init");
        p.oscCount = 1;
        p.oscs[0] = {3, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // PhaseDist
        p.setAdsr(0.01f, 0.1f, 0.7f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Init Wavetable", "Init");
        p.oscCount = 1;
        p.oscs[0] = {1, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Wavetable
        p.setAdsr(0.01f, 0.1f, 0.7f, 0.5f);
        p.applyOscs();
    }

    // ────────────────────────────────── BASS ──────────────────────────────────

    {
        auto& p = make("Acid Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0]  = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.65f, 0.0f, 0.5f}; // Saw + Drive
        p.setAdsr(0.001f, 0.15f, 0.0f, 0.12f);
        p.setFilter(600.0f, 7.0f, 0);
        p.set("distMix", 0.3f);
        p.applyOscs();
    }
    {
        auto& p = make("Sub Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.015f, 0.3f, 0.8f, 0.6f);
        p.setFilter(4000.0f, 0.707f, 0);
        p.set("subOscEnable", 1.0f);
        p.set("subOscLevel", 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("Reese Bass", "Bass");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.75f, -0.08f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.75f,  0.08f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.01f, 0.2f, 0.85f, 0.5f);
        p.setFilter(3000.0f, 1.5f, 0);
        p.setChorus(0.2f, 0.3f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("FM Bass", "Bass", 1);
        p.oscCount = 1;
        p.setAdsr(0.001f, 0.4f, 0.0f, 0.2f);
        p.applyOscs();
    }
    {
        auto& p = make("Dirty Grit", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.85f, 0.0f, 0.5f}; // Drive
        p.setAdsr(0.001f, 0.1f, 0.6f, 0.2f);
        p.setFilter(5000.0f, 2.5f, 0);
        p.set("distMix", 0.4f);
        p.set("distDrive", nSkw(40.0f, 1.0f, 100.0f, 0.3f));
        p.applyOscs();
    }
    {
        auto& p = make("Pluck Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.35f, 0.0f, 0.15f);
        p.setFilter(8000.0f, 3.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Rubber Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Square
        p.setAdsr(0.15f, 0.3f, 0.85f, 0.4f);
        p.setFilter(2500.0f, 1.2f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Deep Dark", "Bass");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.8f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.oscs[1] = {0, 0.4f, 0.0f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Square
        p.setAdsr(0.01f, 0.5f, 0.7f, 0.8f);
        p.setFilter(400.0f, 1.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Wobble Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 2, 0.4f, 0.0f, 0.5f}; // Fold
        p.setAdsr(0.001f, 0.2f, 0.7f, 0.3f);
        p.setFilter(2000.0f, 4.0f, 0);
        p.set("lfo1Rate", LR(3.0f));
        p.applyOscs();
    }
    {
        auto& p = make("Growl Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 2, 0.7f, 0.0f, 0.5f}; // Fold
        p.setAdsr(0.001f, 0.15f, 0.6f, 0.25f);
        p.setFilter(3500.0f, 3.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Stab Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 3, 0.5f, 0.0f, 0.5f}; // Clip
        p.setAdsr(0.001f, 0.08f, 0.0f, 0.08f);
        p.setFilter(6000.0f, 2.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("DnB Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 2, 0.0f, 1, 0.4f, 0.0f, 0.5f}; // Square + Drive
        p.setAdsr(0.001f, 0.25f, 0.3f, 0.15f);
        p.setFilter(3000.0f, 6.0f, 0);
        p.set("distMix", 0.25f);
        p.applyOscs();
    }
    {
        auto& p = make("Analog Warmth", "Bass");
        p.oscCount = 2;
        p.oscs[0] = {4, 0.8f, -0.05f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Analog
        p.oscs[1] = {4, 0.5f,  0.05f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Analog Square
        p.setAdsr(0.01f, 0.3f, 0.75f, 0.5f);
        p.setFilter(5000.0f, 1.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Tech House", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 2, 0.0f, 1, 0.3f, 0.0f, 0.5f}; // Square + Drive
        p.setAdsr(0.001f, 0.2f, 0.5f, 0.2f);
        p.setFilter(5000.0f, 2.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Dub Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.01f, 0.4f, 0.8f, 1.0f);
        p.setFilter(1500.0f, 0.8f, 0);
        p.setDelay(0.2f, 0.375f, 0.4f);
        p.setReverb(0.15f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Supersaw Bass", "Bass");
        p.oscCount = 3;
        p.oscs[0] = {0, 1.0f, -0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f,  0.0f,  1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 1.0f,  0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.01f, 0.15f, 0.7f, 0.4f);
        p.setFilter(8000.0f, 1.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Lead Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.8f, 0.3f);
        p.setFilter(6000.0f, 1.5f, 0);
        p.setMono(true, 0.08f);
        p.applyOscs();
    }
    {
        auto& p = make("Sub Thump", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.001f, 0.5f, 0.0f, 0.3f);
        p.setFilter(300.0f, 0.7f, 0);
        p.set("subOscEnable", 1.0f);
        p.set("subOscLevel", 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Modular Bass", "Bass");
        p.oscCount = 2;
        p.oscs[0] = {2, 0.8f, 0.0f, 0, 0.0f, 1, 0.3f, 0.0f, 0.5f}; // Additive
        p.oscs[1] = {0, 0.5f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.3f, 0.5f, 0.3f);
        p.setFilter(5000.0f, 3.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Portal Bass", "Bass");
        p.oscCount = 1;
        p.oscs[0] = {3, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.7f}; // PhaseDist
        p.setAdsr(0.005f, 0.2f, 0.6f, 0.35f);
        p.setFilter(4000.0f, 2.5f, 0);
        p.applyOscs();
    }

    // ─────────────────────────────── LEAD ────────────────────────────────────

    {
        auto& p = make("Classic Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.85f, 0.4f);
        p.setFilter(8000.0f, 2.5f, 0);
        p.setMono(false);
        p.applyOscs();
    }
    {
        auto& p = make("Supersaw Lead", "Lead");
        p.oscCount = 4;
        p.oscs[0] = {0, 1.0f, -0.25f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f, -0.08f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.08f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[3] = {0, 1.0f,  0.25f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.01f, 0.1f, 0.9f, 0.5f);
        p.setFilter(12000.0f, 1.5f, 0);
        p.setChorus(0.25f, 0.4f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Square Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Square
        p.setAdsr(0.001f, 0.08f, 0.9f, 0.3f);
        p.setFilter(10000.0f, 1.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Pluck Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.4f, 0.0f, 0.25f);
        p.setFilter(9000.0f, 3.0f, 0);
        p.setReverb(0.2f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Mono Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.2f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.85f, 0.3f);
        p.setFilter(10000.0f, 2.0f, 0);
        p.setMono(false, 0.04f);
        p.applyOscs();
    }
    {
        auto& p = make("FM Lead", "Lead", 1);
        p.oscCount = 1;
        p.setAdsr(0.001f, 0.3f, 0.6f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("Whistle Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.05f, 0.1f, 0.9f, 0.5f);
        p.setFilter(20000.0f, 0.707f, 0);
        p.setChorus(0.1f, 0.3f, 0.3f);
        p.applyOscs();
    }
    {
        auto& p = make("Arp Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.2f, 0.0f, 0.15f);
        p.setFilter(9000.0f, 2.5f, 0);
        p.set("chordModeEnabled", 1.0f);
        p.set("chordShape", nCh(1, 12));  // Major
        p.applyOscs();
    }
    {
        auto& p = make("Dual Saw Lead", "Lead");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.9f, -0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f,  0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.005f, 0.1f, 0.85f, 0.4f);
        p.setFilter(11000.0f, 1.8f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Bite Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.6f, 0.0f, 0.5f}; // Drive
        p.setAdsr(0.001f, 0.1f, 0.9f, 0.3f);
        p.setFilter(14000.0f, 2.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Alien Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {3, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.8f}; // PhaseDist
        p.setAdsr(0.001f, 0.2f, 0.7f, 0.4f);
        p.setFilter(12000.0f, 3.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Analog Lead", "Lead");
        p.oscCount = 2;
        p.oscs[0] = {4, 0.9f, -0.05f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Analog
        p.oscs[1] = {4, 0.7f,  0.07f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Analog Square
        p.setAdsr(0.001f, 0.1f, 0.85f, 0.4f);
        p.setFilter(10000.0f, 1.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Bright Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.08f, 0.9f, 0.3f);
        p.setFilter(18000.0f, 1.2f, 0);
        p.set("eqHighGain", nLin(5.0f, -12.0f, 12.0f));
        p.applyOscs();
    }
    {
        auto& p = make("Pad Lead", "Lead");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.9f, -0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.4f, 0.2f, 0.9f, 0.8f);
        p.setFilter(8000.0f, 1.0f, 0);
        p.setReverb(0.3f, 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("Stab Lead", "Lead");
        p.oscCount = 2;
        p.oscs[0] = {0, 1.0f, -0.05f, 1, 0.0f, 3, 0.4f, 0.0f, 0.5f}; // Clip
        p.oscs[1] = {0, 1.0f,  0.05f, 1, 0.0f, 3, 0.4f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.06f, 0.0f, 0.08f);
        p.setFilter(10000.0f, 2.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Chord Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.8f, 0.5f);
        p.setFilter(9000.0f, 1.5f, 0);
        p.set("chordModeEnabled", 1.0f);
        p.set("chordShape", nCh(1, 12));  // Major
        p.set("chordStrumDelay", nLin(15.0f, 0.0f, 200.0f));
        p.applyOscs();
    }
    {
        auto& p = make("Legato Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.15f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.85f, 0.4f);
        p.setFilter(11000.0f, 2.0f, 0);
        p.setMono(true, 0.1f);
        p.applyOscs();
    }
    {
        auto& p = make("Screamer", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 1, 0.7f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.9f, 0.35f);
        p.setFilter(14000.0f, 2.5f, 0);
        p.set("distMix", 0.5f);
        p.set("distDrive", nSkw(60.0f, 1.0f, 100.0f, 0.3f));
        p.applyOscs();
    }
    {
        auto& p = make("Crystal Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {5, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Digital
        p.setAdsr(0.001f, 0.3f, 0.6f, 0.5f);
        p.setFilter(16000.0f, 2.0f, 0);
        p.setReverb(0.2f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Unison Lead", "Lead");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.01f, 0.1f, 0.9f, 0.5f);
        p.setFilter(12000.0f, 1.8f, 0);
        p.setUnison(6, 30.0f, 1.0f);
        p.setChorus(0.2f, 0.4f, 0.5f);
        p.applyOscs();
    }

    // ─────────────────────────────── PAD ─────────────────────────────────────

    {
        auto& p = make("String Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.9f, -0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f,  1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.6f, 0.4f, 0.8f, 1.2f);
        p.setFilter(8000.0f, 0.8f, 0);
        p.setChorus(0.45f, 0.4f, 0.6f);
        p.setReverb(0.35f, 0.65f);
        p.applyOscs();
    }
    {
        auto& p = make("Angel Pad", "Pad");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(1.2f, 0.5f, 0.9f, 2.0f);
        p.setFilter(6000.0f, 0.7f, 0);
        p.setReverb(0.6f, 0.8f);
        p.setChorus(0.3f, 0.3f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("Saw Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {0, 1.0f, -0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.8f,  0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 1.0f,  0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.8f, 0.3f, 0.85f, 1.5f);
        p.setFilter(7000.0f, 0.9f, 0);
        p.setReverb(0.4f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Warm Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.8f, -0.08f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Tri
        p.oscs[1] = {0, 0.8f,  0.08f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.5f, 0.4f, 0.85f, 1.5f);
        p.setFilter(4500.0f, 0.8f, 0);
        p.setChorus(0.3f, 0.25f, 0.4f);
        p.setReverb(0.3f, 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("Dark Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 1.0f, -0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.8f,  0.15f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.7f, 0.5f, 0.75f, 2.0f);
        p.setFilter(2000.0f, 1.0f, 0);
        p.setReverb(0.5f, 0.75f);
        p.applyOscs();
    }
    {
        auto& p = make("Lush Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.9f, -0.18f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f,  1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.18f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.0f, 0.3f, 0.9f, 2.0f);
        p.setFilter(9000.0f, 0.8f, 0);
        p.setUnison(4, 25.0f, 1.0f);
        p.setChorus(0.4f, 0.35f, 0.6f);
        p.setReverb(0.45f, 0.75f);
        p.applyOscs();
    }
    {
        auto& p = make("Glass Pad", "Pad");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 1, 0.2f, 0.0f, 0.5f}; // Sine + Drive
        p.setAdsr(0.8f, 0.5f, 0.8f, 2.0f);
        p.setFilter(10000.0f, 1.2f, 0);
        p.setReverb(0.5f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Choir Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {2, 0.9f, -0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Additive
        p.oscs[1] = {2, 0.9f,  0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.8f, 0.4f, 0.85f, 1.8f);
        p.setFilter(7000.0f, 0.7f, 0);
        p.setChorus(0.5f, 0.3f, 0.6f);
        p.setReverb(0.4f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Motion Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 1.0f, -0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.1f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Tri
        p.setAdsr(0.5f, 0.3f, 0.85f, 1.5f);
        p.setFilter(3000.0f, 2.5f, 0);
        p.set("lfo1Rate", LR(0.4f));
        p.setReverb(0.4f, 0.65f);
        p.applyOscs();
    }
    {
        auto& p = make("Trance Pad", "Pad");
        p.oscCount = 4;
        p.oscs[0] = {0, 1.0f, -0.3f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f, -0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[3] = {0, 1.0f,  0.3f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.4f, 0.3f, 0.9f, 1.5f);
        p.setFilter(10000.0f, 1.2f, 0);
        p.setReverb(0.5f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Cinematic Pad", "Pad", 1);
        p.oscCount = 1;
        p.setAdsr(1.5f, 0.5f, 0.85f, 3.0f);
        p.setReverb(0.6f, 0.85f);
        p.applyOscs();
    }
    {
        auto& p = make("Ambient Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.7f, -0.07f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.oscs[1] = {0, 0.7f,  0.07f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Tri
        p.setAdsr(2.0f, 1.0f, 0.8f, 5.0f);
        p.setFilter(5000.0f, 0.7f, 0);
        p.setReverb(0.7f, 0.9f);
        p.setDelay(0.15f, 0.5f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("Shimmer Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.8f, -0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.8f,  0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.8f, 0.4f, 0.88f, 2.5f);
        p.setFilter(11000.0f, 0.8f, 0);
        p.setUnison(4, 20.0f, 1.0f);
        p.setReverb(0.6f, 0.85f);
        p.applyOscs();
    }
    {
        auto& p = make("Ice Pad", "Pad");
        p.oscCount = 1;
        p.oscs[0] = {5, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Digital
        p.setAdsr(0.9f, 0.5f, 0.8f, 2.0f);
        p.setFilter(14000.0f, 1.5f, 0);
        p.setReverb(0.55f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Ether Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {4, 0.8f, -0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Analog
        p.oscs[1] = {4, 0.8f,  0.1f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.0f, 0.4f, 0.85f, 2.0f);
        p.setFilter(6000.0f, 0.8f, 0);
        p.setDelay(0.2f, 0.4f, 0.45f);
        p.setReverb(0.4f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Breathe Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.85f, -0.06f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.85f,  0.06f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.2f, 0.6f, 0.75f, 2.0f);
        p.setFilter(5500.0f, 0.8f, 0);
        p.setChorus(0.35f, 0.3f, 0.5f);
        p.setReverb(0.4f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Vintage Keys", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {4, 0.9f, -0.08f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {4, 1.0f,  0.0f,  1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {4, 0.9f,  0.08f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.01f, 0.4f, 0.6f, 0.8f);
        p.setFilter(7000.0f, 0.9f, 0);
        p.setChorus(0.25f, 0.3f, 0.35f);
        p.applyOscs();
    }
    {
        auto& p = make("Tape Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {4, 0.9f, -0.05f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {4, 0.9f,  0.05f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.3f, 0.3f, 0.85f, 1.5f);
        p.setFilter(6000.0f, 0.8f, 0);
        p.setChorus(0.4f, 0.3f, 0.55f);
        p.setReverb(0.25f, 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("Evolving Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.8f, -0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f,  3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.8f,  0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.0f, 0.5f, 0.85f, 2.5f);
        p.setFilter(3500.0f, 2.0f, 0);
        p.set("lfo1Rate", LR(0.2f));
        p.setReverb(0.45f, 0.75f);
        p.applyOscs();
    }
    {
        auto& p = make("Space Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.8f, -0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.8f,  0.1f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.5f, 0.8f, 0.8f, 4.0f);
        p.setFilter(7000.0f, 0.7f, 0);
        p.setReverb(0.75f, 0.95f);
        p.setDelay(0.25f, 0.375f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Solar Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {3, 0.9f, -0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.75f}; // PhaseDist
        p.oscs[1] = {3, 0.9f,  0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.75f};
        p.setAdsr(0.6f, 0.4f, 0.85f, 1.8f);
        p.setFilter(8000.0f, 1.2f, 0);
        p.setReverb(0.45f, 0.75f);
        p.applyOscs();
    }
    {
        auto& p = make("Pulse Pad", "Pad");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.9f, -0.12f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Square
        p.oscs[1] = {0, 0.9f,  0.12f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.7f, 0.4f, 0.82f, 2.0f);
        p.setFilter(6000.0f, 1.0f, 0);
        p.setChorus(0.3f, 0.4f, 0.5f);
        p.setReverb(0.35f, 0.65f);
        p.applyOscs();
    }
    {
        auto& p = make("Horizon Pad", "Pad");
        p.oscCount = 4;
        p.oscs[0] = {0, 0.7f, -0.35f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f, -0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.9f,  0.12f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[3] = {0, 0.7f,  0.35f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.0f, 0.5f, 0.9f, 3.0f);
        p.setFilter(10000.0f, 0.7f, 0);
        p.setReverb(0.5f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Drone Pad", "Pad");
        p.oscCount = 3;
        p.oscs[0] = {4, 0.8f, -0.05f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {4, 0.8f,  0.0f,  3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {4, 0.8f,  0.05f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(3.0f, 1.0f, 0.9f, 5.0f);
        p.setFilter(4000.0f, 0.9f, 0);
        p.setReverb(0.6f, 0.9f);
        p.applyOscs();
    }
    {
        auto& p = make("Bell Pad", "Pad", 1);
        p.oscCount = 1;
        p.setAdsr(0.001f, 1.5f, 0.0f, 1.0f);
        p.setReverb(0.5f, 0.75f);
        p.applyOscs();
    }

    // ────────────────────────────── DRUMS ─────────────────────────────────────

    {
        auto& p = make("Kick Body", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.001f, 0.5f, 0.0f, 0.2f);
        p.setFilter(300.0f, 0.7f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Kick Click", "Drums");
        p.oscCount = 2;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.oscs[1] = {0, 0.6f, 0.0f, 2, 0.0f, 1, 0.6f, 0.0f, 0.5f}; // Square + Drive
        p.setAdsr(0.001f, 0.25f, 0.0f, 0.15f);
        p.setFilter(800.0f, 1.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Snare Core", "Drums");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.6f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine (body)
        p.oscs[1] = {0, 0.7f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise (snare)
        p.setAdsr(0.001f, 0.18f, 0.0f, 0.12f);
        p.setFilter(5000.0f, 1.0f, 1);  // HP
        p.applyOscs();
    }
    {
        auto& p = make("Hi-Hat Open", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.5f, 0.0f, 0.4f);
        p.setFilter(9000.0f, 1.5f, 1);  // HP
        p.applyOscs();
    }
    {
        auto& p = make("Hi-Hat Closed", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.06f, 0.0f, 0.05f);
        p.setFilter(10000.0f, 1.5f, 1);  // HP
        p.applyOscs();
    }
    {
        auto& p = make("Tom Low", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.001f, 0.4f, 0.0f, 0.3f);
        p.setFilter(500.0f, 1.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Tom High", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.001f, 0.25f, 0.0f, 0.2f);
        p.setFilter(1200.0f, 1.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Rim Shot", "Drums");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.5f, 0.0f, 2, 0.0f, 1, 0.5f, 0.0f, 0.5f}; // Square + Drive
        p.oscs[1] = {0, 0.6f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.1f, 0.0f, 0.08f);
        p.setFilter(4000.0f, 2.0f, 1);  // HP
        p.applyOscs();
    }
    {
        auto& p = make("Clap", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.22f, 0.0f, 0.18f);
        p.setFilter(3000.0f, 2.5f, 1);  // HP
        p.setReverb(0.2f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("Cymbal Crash", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 2.0f, 0.0f, 1.5f);
        p.setFilter(8000.0f, 1.2f, 1);  // HP
        p.setReverb(0.25f, 0.5f);
        p.applyOscs();
    }
    {
        auto& p = make("Perc Snap", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 2, 0.0f, 1, 0.7f, 0.0f, 0.5f}; // Square + Drive
        p.setAdsr(0.001f, 0.08f, 0.0f, 0.06f);
        p.setFilter(6000.0f, 2.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Shaker", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 0.7f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.07f, 0.0f, 0.06f);
        p.setFilter(8000.0f, 1.0f, 1);  // HP
        p.applyOscs();
    }
    {
        auto& p = make("Conga", "Drums");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.9f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.oscs[1] = {0, 0.3f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 0.2f, 0.0f, 0.15f);
        p.setFilter(800.0f, 1.5f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Synth Perc", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {5, 1.0f, 0.0f, 0, 0.0f, 1, 0.5f, 0.0f, 0.5f}; // Digital + Drive
        p.setAdsr(0.001f, 0.15f, 0.0f, 0.1f);
        p.setFilter(7000.0f, 3.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Sub Kick", "Drums");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Sine
        p.setAdsr(0.001f, 0.8f, 0.0f, 0.4f);
        p.setFilter(120.0f, 0.7f, 0);
        p.set("subOscEnable", 1.0f);
        p.set("subOscLevel", 0.9f);
        p.applyOscs();
    }

    // ─────────────────────────────── FX ──────────────────────────────────────

    {
        auto& p = make("Riser", "FX");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.8f, -0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.8f,  0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(4.0f, 1.0f, 0.9f, 1.0f);
        p.setFilter(1000.0f, 3.0f, 0);
        p.setReverb(0.4f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Downlifter", "FX");
        p.oscCount = 2;
        p.oscs[0] = {0, 1.0f, -0.1f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.1f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.5f, 0.9f, 3.0f);
        p.setFilter(15000.0f, 1.0f, 0);
        p.setReverb(0.5f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Noise Sweep", "FX");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.001f, 2.0f, 0.0f, 1.0f);
        p.setFilter(500.0f, 3.5f, 0);
        p.set("lfo1Rate", LR(0.3f));
        p.setReverb(0.4f, 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("White Space", "FX");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.setAdsr(0.5f, 0.5f, 0.8f, 2.0f);
        p.setFilter(8000.0f, 0.7f, 0);
        p.setReverb(0.7f, 0.9f);
        p.applyOscs();
    }
    {
        auto& p = make("Feedback FX", "FX");
        p.oscCount = 1;
        p.oscs[0] = {0, 0.6f, 0.0f, 0, 0.0f, 0, 0.0f, 0.9f, 0.5f}; // Sine + Self-Osc
        p.setAdsr(0.01f, 0.2f, 0.8f, 0.5f);
        p.setFilter(5000.0f, 8.0f, 0);
        p.setDelay(0.3f, 0.35f, 0.6f);
        p.applyOscs();
    }
    {
        auto& p = make("Drone FX", "FX");
        p.oscCount = 4;
        p.oscs[0] = {4, 0.7f, -0.3f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {4, 0.7f, -0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {4, 0.7f,  0.1f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[3] = {4, 0.7f,  0.3f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(2.0f, 1.0f, 0.9f, 4.0f);
        p.setFilter(6000.0f, 1.0f, 0);
        p.setReverb(0.6f, 0.85f);
        p.applyOscs();
    }
    {
        auto& p = make("Glitch FX", "FX");
        p.oscCount = 1;
        p.oscs[0] = {5, 1.0f, 0.0f, 2, 0.0f, 3, 0.6f, 0.0f, 0.5f}; // Digital + Clip
        p.setAdsr(0.001f, 0.05f, 0.0f, 0.04f);
        p.setFilter(12000.0f, 4.0f, 0);
        p.applyOscs();
    }
    {
        auto& p = make("Sci-Fi Sweep", "FX");
        p.oscCount = 2;
        p.oscs[0] = {3, 0.9f, -0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.85f}; // PhaseDist
        p.oscs[1] = {3, 0.7f,  0.1f, 0, 0.0f, 0, 0.0f, 0.0f, 0.6f};
        p.setAdsr(0.5f, 1.0f, 0.7f, 1.5f);
        p.setFilter(8000.0f, 3.0f, 0);
        p.setReverb(0.45f, 0.75f);
        p.setDelay(0.2f, 0.3f, 0.4f);
        p.applyOscs();
    }
    {
        auto& p = make("Alien Signal", "FX");
        p.oscCount = 3;
        p.oscs[0] = {3, 0.8f, -0.5f, 0, 0.0f, 2, 0.4f, 0.0f, 0.7f};
        p.oscs[1] = {3, 1.0f,  0.0f, 0, 0.0f, 0, 0.0f, 0.0f, 0.9f};
        p.oscs[2] = {3, 0.8f,  0.5f, 0, 0.0f, 2, 0.4f, 0.0f, 0.7f};
        p.setAdsr(0.01f, 0.5f, 0.6f, 1.0f);
        p.setFilter(10000.0f, 2.5f, 0);
        p.setReverb(0.5f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Wind FX", "FX");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.8f, -0.2f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f}; // Noise
        p.oscs[1] = {0, 0.6f,  0.2f, 4, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(1.0f, 1.5f, 0.7f, 2.0f);
        p.setFilter(3000.0f, 1.5f, 0);
        p.set("lfo1Rate", LR(0.2f));
        p.setReverb(0.5f, 0.7f);
        p.applyOscs();
    }
    {
        auto& p = make("Stutter FX", "FX");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(0.001f, 0.1f, 0.8f, 0.2f);
        p.setFilter(9000.0f, 2.0f, 0);
        p.set("lfo1Rate", LR(8.0f));
        p.set("lfo1Shape", nCh(3, 6));  // Square
        p.applyOscs();
    }
    {
        auto& p = make("Reverse FX", "FX");
        p.oscCount = 2;
        p.oscs[0] = {0, 0.9f, -0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 0.9f,  0.15f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(3.0f, 0.5f, 0.9f, 0.5f);
        p.setFilter(5000.0f, 1.5f, 0);
        p.setReverb(0.6f, 0.85f);
        p.applyOscs();
    }
    {
        auto& p = make("Chord Riser", "FX");
        p.oscCount = 1;
        p.oscs[0] = {0, 1.0f, 0.0f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(2.0f, 0.5f, 0.9f, 1.5f);
        p.setFilter(8000.0f, 2.0f, 0);
        p.set("chordModeEnabled", 1.0f);
        p.set("chordShape", nCh(1, 12));  // Major
        p.set("chordStrumDelay", nLin(30.0f, 0.0f, 200.0f));
        p.setReverb(0.5f, 0.8f);
        p.applyOscs();
    }
    {
        auto& p = make("Horror Scape", "FX");
        p.oscCount = 3;
        p.oscs[0] = {4, 0.7f, -0.7f, 1, 0.0f, 2, 0.5f, 0.0f, 0.5f}; // Analog + Fold
        p.oscs[1] = {4, 0.8f,  0.0f, 2, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {4, 0.7f,  0.7f, 1, 0.0f, 2, 0.5f, 0.0f, 0.5f};
        p.setAdsr(0.5f, 2.0f, 0.8f, 3.0f);
        p.setFilter(2000.0f, 4.0f, 0);
        p.setReverb(0.7f, 0.95f);
        p.applyOscs();
    }
    {
        auto& p = make("Pad Rise FX", "FX");
        p.oscCount = 3;
        p.oscs[0] = {0, 0.8f, -0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[1] = {0, 1.0f,  0.0f, 3, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.oscs[2] = {0, 0.8f,  0.2f, 1, 0.0f, 0, 0.0f, 0.0f, 0.5f};
        p.setAdsr(3.0f, 1.0f, 0.85f, 2.0f);
        p.setFilter(2000.0f, 2.5f, 0);
        p.set("lfo1Rate", LR(0.15f));
        p.setReverb(0.55f, 0.85f);
        p.applyOscs();
    }

    return presets;
}

// ─── Public API ──────────────────────────────────────────────────────────────
void FactoryPresets::writeToDirectory(const juce::File& factoryDir)
{
    auto defaults = buildDefaults();
    auto presets  = buildAllPresets(defaults);

    for (const auto& pd : presets)
    {
        // Ensure category dir exists
        factoryDir.getChildFile(pd.category).createDirectory();

        auto xml = buildPresetXml(pd, defaults);
        writePreset(factoryDir, pd.category, pd.name, xml);
    }
}
