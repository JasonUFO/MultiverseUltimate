#include "SynthEngine.h"
#include <cmath>

SynthEngine::SynthEngine()
    : envAttack(0.01f), envDecay(0.1f), envSustain(0.7f), envRelease(0.3f)
{
    formatManager.registerBasicFormats();
}

void SynthEngine::prepare(double sr, int)
{
    if (sr <= 0.0 || sr > 1000000.0)
        sr = 44100.0;

    sampleRate = static_cast<float>(sr);

    for (auto& vi : voices)
        vi.voice.setSampleRate(static_cast<float>(sr));

    for (auto& vi : fmVoices)
        vi.voice.setSampleRate(sr);
}

void SynthEngine::noteOn(int note, float velocity)
{
    if (synthMode == SynthMode::FM)
    {
        auto* vi = findFMVoiceForNote(note);
        if (vi != nullptr)
        {
            vi->voice.noteOff();
            vi->inUse = false;
        }

        vi = findFreeFMVoice();
        if (vi != nullptr)
        {
            vi->voice.noteOn(note, velocity);
            vi->voice.setPitchBend(static_cast<float>(pitchBend));
            vi->inUse = true;
            vi->lastUseTime = voiceCounter++;
        }
        return;
    }

    // Kill any existing voices for this note (handles retrigger + unison groups)
    for (auto& vi : voices)
        if (vi.inUse && vi.voice.getMidiNote() == note)
        { vi.voice.noteOff(); vi.inUse = false; }

    const int n = juce::jlimit(1, MAX_VOICES, unisonVoiceCount);
    for (int u = 0; u < n; ++u)
    {
        auto* vi = findFreeVoice();
        if (vi == nullptr) break;

        float detuneOffset = 0.0f;
        float panL = 1.0f, panR = 1.0f;
        if (n > 1)
        {
            const float t = static_cast<float>(u) / static_cast<float>(n - 1); // 0..1
            detuneOffset = (t - 0.5f) * 2.0f * unisonDetuneSemitones;
            const float pan = (t - 0.5f) * 2.0f * unisonWidthAmount; // -1..+1
            panL = pan <= 0.0f ? 1.0f : (1.0f - pan);
            panR = pan >= 0.0f ? 1.0f : (1.0f + pan);
        }

        vi->voice.noteOn(note, velocity);
        vi->voice.setPitchBend(static_cast<float>(pitchBend));
        for (int i = 0; i < 3; ++i)
        {
            vi->voice.setOscillatorType(i, oscSettings[i].type);
            vi->voice.setOscillatorLevel(i, oscSettings[i].level);
            vi->voice.setOscillatorDetune(i, oscSettings[i].detuneSemitones + detuneOffset);
            vi->voice.setOscillatorWaveform(i, oscSettings[i].classicWaveform);
            vi->voice.setOscillatorWavePosition(i, oscSettings[i].wavePosition);
        }
        vi->unisonDetuneOffset = detuneOffset;
        vi->panLeft  = panL;
        vi->panRight = panR;
        vi->inUse = true;
        vi->lastUseTime = voiceCounter++;
    }
}

void SynthEngine::noteOff(int note)
{
    if (synthMode == SynthMode::FM)
    {
        auto* vi = findFMVoiceForNote(note);
        if (vi != nullptr)
            vi->voice.noteOff();
        return;
    }

    // Release all unison voices for this note
    for (auto& vi : voices)
        if (vi.inUse && vi.voice.getMidiNote() == note)
            vi.voice.noteOff();
}

void SynthEngine::allNotesOff()
{
    for (auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive())
            vi.voice.noteOff();
    }

    for (auto& vi : fmVoices)
    {
        if (vi.inUse && vi.voice.isActive())
            vi.voice.noteOff();
    }
}

void SynthEngine::setMasterVolume(float volume)
{
    masterVolume = juce::jlimit(0.0f, 1.0f, volume);
}

void SynthEngine::setPitchBend(float cents)
{
    pitchBend = cents;
    if (synthMode == SynthMode::FM)
    {
        for (auto& vi : fmVoices)
            if (vi.inUse && vi.voice.isActive())
                vi.voice.setPitchBend(cents);
    }
    else
    {
        for (auto& vi : voices)
            if (vi.inUse && vi.voice.isActive())
                vi.voice.setPitchBend(cents);
    }
}

void SynthEngine::setEnvelopeParams(float a, float d, float s, float r)
{
    envAttack = a;
    envDecay = d;
    envSustain = s;
    envRelease = r;
    for (auto& vi : voices)
    {
        vi.voice.setEnvelopeParams(a, d, s, r);
    }
}

void SynthEngine::setFilterParams(float cutoff, float resonance)
{
    for (auto& vi : voices)
    {
        vi.voice.setFilterParams(cutoff, resonance);
    }
}

void SynthEngine::setOversamplingMode(Filter::OversamplingMode mode)
{
    for (auto& vi : voices)
        vi.voice.setOversamplingMode(mode);
}

void SynthEngine::setOscillatorType(int index, OscillatorType type)
{
    if (index < 0 || index > 2) return;
    oscSettings[index].type = type;
    for (auto& vi : voices)
        vi.voice.setOscillatorType(index, type);
}

void SynthEngine::setOscillatorLevel(int index, float level)
{
    if (index < 0 || index > 2) return;
    oscSettings[index].level = juce::jlimit(0.0f, 1.0f, level);
    for (auto& vi : voices)
        vi.voice.setOscillatorLevel(index, oscSettings[index].level);
}

void SynthEngine::setOscillatorDetune(int index, float detuneSemitones)
{
    if (index < 0 || index > 2) return;
    oscSettings[index].detuneSemitones = detuneSemitones;
    for (auto& vi : voices)
        vi.voice.setOscillatorDetune(index, detuneSemitones + vi.unisonDetuneOffset);
}

void SynthEngine::setOscillatorWaveform(int index, WaveformType wf)
{
    if (index < 0 || index > 2) return;
    oscSettings[index].classicWaveform = wf;
    for (auto& vi : voices)
        vi.voice.setOscillatorWaveform(index, wf);
}

void SynthEngine::setOscillatorWavePosition(int index, float pos)
{
    if (index < 0 || index > 2) return;
    oscSettings[index].wavePosition = juce::jlimit(0.0f, 1.0f, pos);
    for (auto& vi : voices)
        vi.voice.setOscillatorWavePosition(index, pos);
}

float SynthEngine::process()
{
    float output = 0.0f;
    int activeCount = 0;

    if (synthMode == SynthMode::FM)
    {
        for (auto& vi : fmVoices)
        {
            if (vi.inUse && vi.voice.isActive())
            {
                output += vi.voice.process();
                ++activeCount;
            }
            else if (vi.inUse && !vi.voice.isActive())
            {
                vi.inUse = false;
            }
        }
    }
    else
    {
        for (auto& vi : voices)
        {
            if (vi.inUse && vi.voice.isActive())
            {
                output += vi.voice.process();
                ++activeCount;
            }
            else if (vi.inUse && !vi.voice.isActive())
            {
                vi.inUse = false;
            }
        }
    }

    if (activeCount > 0)
        output /= static_cast<float>(activeCount);

    return output * masterVolume;
}

int SynthEngine::processBuffer(juce::AudioBuffer<float>& buffer, int numSamples)
{
    int activeCount = 0;
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    if (synthMode == SynthMode::FM)
    {
        for (auto& vi : fmVoices)
        {
            if (!vi.inUse) continue;

            if (vi.voice.isActive())
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    float s = vi.voice.process();
                    left[i] += s;
                    right[i] += s;
                }
                ++activeCount;
            }
            else
            {
                vi.inUse = false;
            }
        }
    }
    else
    {
        for (auto& vi : voices)
        {
            if (!vi.inUse) continue;

            if (vi.voice.isActive())
            {
                const float pL = vi.panLeft;
                const float pR = vi.panRight;
                for (int i = 0; i < numSamples; ++i)
                {
                    float s = vi.voice.process();
                    left[i]  += s * pL;
                    right[i] += s * pR;
                }
                ++activeCount;
            }
            else
            {
                vi.inUse = false;
            }
        }
    }

    if (activeCount > 0)
    {
        float scale = 1.0f / static_cast<float>(activeCount);
        for (int i = 0; i < numSamples; ++i)
        {
            left[i] *= scale;
            right[i] *= scale;
        }
    }

    float vol = masterVolume;
    for (int i = 0; i < numSamples; ++i)
    {
        left[i] *= vol;
        right[i] *= vol;
    }

    return activeCount;
}

int SynthEngine::getActiveVoiceCount() const
{
    if (synthMode == SynthMode::FM)
    {
        int count = 0;
        for (const auto& vi : fmVoices)
        {
            if (vi.inUse && vi.voice.isActive())
                ++count;
        }
        return count;
    }

    int count = 0;
    for (const auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive())
            ++count;
    }
    return count;
}

void SynthEngine::setSynthMode(SynthMode mode)
{
    if (mode == synthMode)
        return;

    allNotesOff();
    synthMode = mode;
}

void SynthEngine::setFMAlgorithm(int index)
{
    if (index < 0 || index >= FM_ALGORITHM_COUNT)
        return;

    fmAlgorithmIndex = index;
    for (auto& vi : fmVoices)
        vi.voice.setAlgorithm(index);
}

void SynthEngine::setFMOperatorParams(int opIndex,
                                      float ratio,
                                      float level,
                                      float feedback,
                                      float attack,
                                      float decay,
                                      float sustain,
                                      float release)
{
    if (opIndex < 0 || opIndex >= 4) return;
    auto& p = fmOpParams[opIndex];
    p.ratio = ratio;
    p.level = level;
    p.feedback = feedback;
    p.attack = attack;
    p.decay = decay;
    p.sustain = sustain;
    p.release = release;

    for (auto& vi : fmVoices)
    {
        vi.voice.setOperatorParams(opIndex, ratio, level, feedback, attack, decay, sustain, release);
    }
}

SynthEngine::VoiceInfo* SynthEngine::findFreeVoice()
{
    VoiceInfo* oldest = nullptr;
    int oldestTime = INT_MAX;

    for (auto& vi : voices)
    {
        if (!vi.inUse)
            return &vi;

        if (vi.lastUseTime < oldestTime)
        {
            oldestTime = vi.lastUseTime;
            oldest = &vi;
        }
    }

    return oldest;
}

SynthEngine::VoiceInfo* SynthEngine::findVoiceForNote(int note)
{
    for (auto& vi : voices)
    {
        if (vi.inUse && vi.voice.isActive() && vi.voice.getMidiNote() == note)
            return &vi;
    }
    return nullptr;
}

SynthEngine::FMVoiceInfo* SynthEngine::findFreeFMVoice()
{
    FMVoiceInfo* oldest = nullptr;
    int oldestTime = INT_MAX;

    for (auto& vi : fmVoices)
    {
        if (!vi.inUse)
            return &vi;

        if (vi.lastUseTime < oldestTime)
        {
            oldestTime = vi.lastUseTime;
            oldest = &vi;
        }
    }

    return oldest;
}

SynthEngine::FMVoiceInfo* SynthEngine::findFMVoiceForNote(int note)
{
    for (auto& vi : fmVoices)
    {
        if (vi.inUse && vi.voice.isActive() && vi.voice.getMidiNote() == note)
            return &vi;
    }
    return nullptr;
}

// Getters for state persistence
void SynthEngine::getEnvelopeParams(float& a, float& d, float& s, float& r) const
{
    a = envAttack;
    d = envDecay;
    s = envSustain;
    r = envRelease;
}

WaveformType SynthEngine::getWaveform() const
{
    return oscSettings[0].classicWaveform;
}

OscillatorType SynthEngine::getOscillatorType(int index) const
{
    return (index >= 0 && index < 3) ? oscSettings[index].type : OscillatorType::Classic;
}

float SynthEngine::getOscillatorLevel(int index) const
{
    return (index >= 0 && index < 3) ? oscSettings[index].level : 1.0f;
}

float SynthEngine::getOscillatorDetune(int index) const
{
    return (index >= 0 && index < 3) ? oscSettings[index].detuneSemitones : 0.0f;
}

WaveformType SynthEngine::getOscillatorWaveform(int index) const
{
    return (index >= 0 && index < 3) ? oscSettings[index].classicWaveform : WaveformType::Saw;
}

float SynthEngine::getOscillatorWavePosition(int index) const
{
    return (index >= 0 && index < 3) ? oscSettings[index].wavePosition : 0.0f;
}

void SynthEngine::getFMOperatorParams(int opIndex,
                                      float& ratio,
                                      float& level,
                                      float& feedback,
                                      float& attack,
                                      float& decay,
                                      float& sustain,
                                      float& release) const
{
    if (opIndex >= 0 && opIndex < 4)
    {
        const auto& p = fmOpParams[opIndex];
        ratio = p.ratio;
        level = p.level;
        feedback = p.feedback;
        attack = p.attack;
        decay = p.decay;
        sustain = p.sustain;
        release = p.release;
    }
    else
    {
        ratio = 1.0f; level = 1.0f; feedback = 0.0f;
        attack = 0.01f; decay = 0.1f; sustain = 0.7f; release = 0.3f;
    }
}

void SynthEngine::setUnisonVoices(int n)
{
    unisonVoiceCount = juce::jlimit(1, MAX_VOICES, n);
}

void SynthEngine::setUnisonDetune(float semitones)
{
    unisonDetuneSemitones = juce::jmax(0.0f, semitones);
}

void SynthEngine::setUnisonWidth(float w)
{
    unisonWidthAmount = juce::jlimit(0.0f, 1.0f, w);
}

bool SynthEngine::loadWavetableFile(int oscIndex, const juce::File& file)
{
    if (oscIndex < 0 || oscIndex > 2) return false;
    if (!file.existsAsFile()) return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr) return false;

    const int numSamples = (int)juce::jmin(reader->lengthInSamples, (juce::int64)1048576);
    juce::AudioBuffer<float> audio(1, numSamples);
    audio.clear();
    reader->read(&audio, 0, numSamples, 0, true, false);

    for (auto& vi : voices)
        vi.voice.loadWavetableData(oscIndex, audio);

    wavetableFilePaths[oscIndex] = file.getFullPathName();
    return true;
}

juce::String SynthEngine::getWavetableFilePath(int oscIndex) const
{
    if (oscIndex < 0 || oscIndex > 2) return {};
    return wavetableFilePaths[oscIndex];
}
