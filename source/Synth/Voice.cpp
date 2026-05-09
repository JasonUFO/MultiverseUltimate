#include "Voice.h"
#include <cmath>

Voice::Voice()
{
    envelope.setParameters(attackTime, decayTime, sustainLevel, releaseTime);
    noiseOsc.noiseOsc.setWaveform(WaveformType::Noise);
    noiseOsc.noiseOsc.setSampleRate(storedSampleRate);
    subOsc.osc.setSampleRate(storedSampleRate);
}

void Voice::noteOn(int note, float vel)
{
    midiNote = note;
    velocity = vel;
    active = true;

    baseFrequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    subOsc.osc.setFrequency(baseFrequency * 0.5f);
    updateOscillatorFrequencies();
    envelope.triggerAttack();
}

void Voice::setPitchBend(float semitones)
{
    pitchBendSemitones = semitones;
    updateOscillatorFrequencies();
}

void Voice::updateOscillatorFrequencies()
{
    float bentFreq = baseFrequency * std::pow(2.0f, pitchBendSemitones / 12.0f);
    for (int i = 0; i < 8; ++i)
    {
        auto& osc = oscStates[i];
        float freq = bentFreq * std::pow(2.0f, osc.detuneSemitones / 12.0f);
        osc.classicOsc.setFrequency(freq);
        osc.wavetableOsc.setFrequency(freq);
    }
    subOsc.osc.setFrequency(bentFreq * 0.5f);
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

void Voice::setOversamplingMode(Filter::OversamplingMode mode)
{
    filter.setOversamplingMode(mode);
}

void Voice::setSampleRate(float sr)
{
    storedSampleRate = sr;
    for (int i = 0; i < 8; ++i)
    {
        oscStates[i].classicOsc.setSampleRate(sr);
        oscStates[i].wavetableOsc.prepare(sr);
    }
    subOsc.osc.setSampleRate(sr);
    noiseOsc.noiseOsc.setSampleRate(sr);
    // Recompute noise color coefficient: one-pole LP coeff = 1 - exp(-2pi*fc/sr)
    const float fc = juce::jlimit(20.0f, 20000.0f, noiseOsc.colorCutoffHz);
    noiseOsc.colorCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi * fc / sr);
    filter.setSampleRate(sr);
    envelope.setSampleRate(sr);
}

void Voice::setActiveOscs(int n)
{
    activeOscs = juce::jlimit(1, 4, n);
}

void Voice::setOscillatorType(int index, OscillatorType t)
{
    if (index < 0 || index > 7) return;
    oscStates[index].type = t;
}

void Voice::setOscillatorLevel(int index, float level)
{
    if (index < 0 || index > 7) return;
    oscStates[index].level = juce::jlimit(0.0f, 1.0f, level);
}

void Voice::setOscillatorDetune(int index, float detuneSemitones)
{
    if (index < 0 || index > 7) return;
    oscStates[index].detuneSemitones = detuneSemitones;
    updateOscillatorFrequencies();
}

void Voice::setOscillatorWaveform(int index, WaveformType wf)
{
    if (index < 0 || index > 7) return;
    oscStates[index].classicOsc.setWaveform(wf);
}

void Voice::setOscillatorWavePosition(int index, float pos)
{
    if (index < 0 || index > 7) return;
    oscStates[index].wavetableOsc.setWavePosition(pos);
}

void Voice::setOscillatorShapeType(int index, OscShapeType st)
{
    if (index < 0 || index > 7) return;
    oscStates[index].shapeType = st;
}

void Voice::setOscillatorShapeAmount(int index, float amt)
{
    if (index < 0 || index > 7) return;
    oscStates[index].shapeAmount = juce::jlimit(0.0f, 1.0f, amt);
}

void Voice::setOscillatorSelfOsc(int index, float feedback)
{
    if (index < 0 || index > 7) return;
    oscStates[index].selfOscFeedback = juce::jlimit(0.0f, 1.0f, feedback);
}

void Voice::setOscillatorPhaseDistAmount(int index, float amt)
{
    if (index < 0 || index > 7) return;
    oscStates[index].phaseDistAmount = juce::jlimit(0.0f, 1.0f, amt);
}

void Voice::setSubOscEnabled(bool e)  { subOsc.enabled = e; }
void Voice::setSubOscLevel(float l)   { subOsc.level = juce::jlimit(0.0f, 1.0f, l); }
void Voice::setSubOscWaveform(WaveformType wf) { subOsc.osc.setWaveform(wf); }

void Voice::setNoiseOscEnabled(bool e) { noiseOsc.enabled = e; }
void Voice::setNoiseOscLevel(float l)  { noiseOsc.level = juce::jlimit(0.0f, 1.0f, l); }
void Voice::setNoiseOscColor(float hz)
{
    noiseOsc.colorCutoffHz = juce::jlimit(20.0f, 20000.0f, hz);
    if (storedSampleRate > 0.0f)
        noiseOsc.colorCoeff = 1.0f - std::exp(-juce::MathConstants<float>::twoPi
                                               * noiseOsc.colorCutoffHz / storedSampleRate);
}

void Voice::setFilterType(Filter::FilterType t) { filter.setFilterType(t); }

void Voice::loadWavetableData(int oscIndex, const juce::AudioBuffer<float>& audio)
{
    if (oscIndex < 0 || oscIndex > 7) return;
    oscStates[oscIndex].wavetableOsc.loadWavetable(audio);
}

void Voice::setFrequencyDirect(float hz)
{
    baseFrequency = juce::jmax(1.0f, hz);
    updateOscillatorFrequencies();
}

void Voice::setNoteLegato(int note)
{
    midiNote = note;
    baseFrequency = 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
}

static inline float applyOscShaping(float s, OscShapeType shapeType, float shapeAmount, float& selfOscPrev, float selfOscFeedback)
{
    if (shapeAmount > 0.001f)
    {
        switch (shapeType)
        {
            case OscShapeType::Drive:
            {
                const float drive = 1.0f + shapeAmount * 9.0f;
                const float norm  = std::tanh(drive);
                s = std::tanh(s * drive) / norm;
                break;
            }
            case OscShapeType::Fold:
            {
                const float threshold = 1.0f - shapeAmount * 0.8f;
                while (std::abs(s) > threshold)
                    s = s > 0.0f ? 2.0f * threshold - s : -2.0f * threshold - s;
                break;
            }
            case OscShapeType::Clip:
            {
                const float limit = juce::jmax(0.05f, 1.0f - shapeAmount * 0.5f);
                s = juce::jlimit(-limit, limit, s) / limit;
                break;
            }
            default: break;
        }
    }
    if (selfOscFeedback > 0.001f)
    {
        s = juce::jlimit(-1.0f, 1.0f, s + selfOscPrev * selfOscFeedback);
    }
    selfOscPrev = s;
    return s;
}

float Voice::process()
{
    if (!active) return 0.0f;

    const float twoPi = juce::MathConstants<float>::twoPi;
    float oscMix = 0.0f;
    for (int i = 0; i < activeOscs; ++i)
    {
        auto& osc = oscStates[i];
        if (osc.level <= 0.0f) { osc.classicOsc.process(); continue; }

        float s = 0.0f;
        switch (osc.type)
        {
            case OscillatorType::Classic:
                s = osc.classicOsc.process();
                break;
            case OscillatorType::Wavetable:
                osc.classicOsc.process(); // keep phase in sync
                s = osc.wavetableOsc.process();
                break;
            case OscillatorType::Additive:
            {
                osc.classicOsc.process();
                const float ph = osc.classicOsc.getPhase() * twoPi;
                float sum = 0.0f, norm = 0.0f;
                for (int k = 1; k <= 8; ++k)
                {
                    sum  += std::sin(ph * k) / k;
                    norm += 1.0f / k;
                }
                s = sum / norm;
                break;
            }
            case OscillatorType::PhaseDist:
            {
                osc.classicOsc.process();
                const float ph = osc.classicOsc.getPhase();
                const float distorted = ph + osc.phaseDistAmount * std::sin(ph * twoPi);
                s = std::sin(distorted * twoPi);
                break;
            }
            case OscillatorType::Analog:
            {
                s = osc.classicOsc.process();
                osc.analogLCG = osc.analogLCG * 1664525u + 1013904223u;
                const float drift = (int32_t(osc.analogLCG) / float(0x7fffffff)) * 0.015f;
                s = juce::jlimit(-1.0f, 1.0f, s + drift);
                break;
            }
            case OscillatorType::Digital:
            {
                s = osc.classicOsc.process();
                const float levels = 16.0f;
                s = std::round(s * levels) / levels;
                break;
            }
        }

        s = applyOscShaping(s, osc.shapeType, osc.shapeAmount, osc.selfOscPrev, osc.selfOscFeedback);
        oscMix += s * osc.level;
    }

    if (subOsc.enabled)
        oscMix += subOsc.osc.process() * subOsc.level;

    if (noiseOsc.enabled)
    {
        float rawNoise = noiseOsc.noiseOsc.process();
        noiseOsc.colorFilterState = noiseOsc.colorCoeff * rawNoise
                                  + (1.0f - noiseOsc.colorCoeff) * noiseOsc.colorFilterState;
        oscMix += noiseOsc.colorFilterState * noiseOsc.level;
    }

    oscMix = envelope.process(oscMix);
    oscMix = filter.process(oscMix);

    return oscMix;
}
