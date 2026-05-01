#include "GranularEngine.h"

GranularEngine::GranularEngine()
{
    // Pre-allocate source buffer: 2s at 48kHz stereo (worst case before prepare())
    sourceBuffer.setSize (2, 48000 * 2, false, true, false);
    buildDefaultSourceBuffer();
}

void GranularEngine::prepare (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    blockSize  = samplesPerBlock;

    // Resize source buffer to 4s at actual sample rate
    {
        juce::ScopedLock sl (sourceLock);
        if (sourceFilePath.isEmpty())
        {
            sourceBuffer.setSize (2, static_cast<int>(sr * 4.0), false, true, false);
            buildDefaultSourceBuffer();
        }
    }

    for (auto& v : voices)
        v.prepare (sr);
}

void GranularEngine::buildDefaultSourceBuffer()
{
    // Default: 2s sine sweep 80Hz→1200Hz — rich enough to demonstrate all grain effects
    const int len = sourceBuffer.getNumSamples();
    const float sr = (sampleRate > 0.0) ? static_cast<float>(sampleRate) : 44100.0f;
    float* L = sourceBuffer.getWritePointer(0);
    float* R = sourceBuffer.getWritePointer(1);

    for (int i = 0; i < len; ++i)
    {
        float t    = static_cast<float>(i) / sr;
        float frac = static_cast<float>(i) / static_cast<float>(len - 1);
        float freq = 80.0f * std::pow (15.0f, frac);  // 80 → 1200 Hz
        float phase = juce::MathConstants<float>::twoPi * freq * t;
        float samp  = 0.5f * std::sin (phase);
        L[i] = samp;
        R[i] = samp;
    }
}

void GranularEngine::noteOn (int midiNote, float velocity)
{
    // Find an inactive voice; steal oldest active if all busy
    int target = 0;
    for (int i = 0; i < MAX_GRANULAR_VOICES; ++i)
    {
        if (!voices[i].isActive()) { target = i; break; }
        target = i; // fallback: last one
    }
    voices[target].noteOn (midiNote, velocity);
}

void GranularEngine::noteOff (int midiNote)
{
    for (auto& v : voices)
        if (v.isActive() && v.getMidiNote() == midiNote)
            v.noteOff();
}

void GranularEngine::allNotesOff()
{
    for (auto& v : voices)
        if (v.isActive())
            v.noteOff();
}

void GranularEngine::processBuffer (juce::AudioBuffer<float>& buffer, int numSamples)
{
    const float pos    = position.load();
    const float gs     = grainSize.load();
    const float sp     = spray.load();
    const float den    = density.load();
    const float psc    = pitchScatter.load();
    const auto  es     = static_cast<GrainEnvShape>(envShape.load());
    const bool  rev    = reverse.load();
    const float spread = stereoSpread.load();
    const float att    = attack.load();
    const float dec    = decay.load();
    const float sus    = sustain.load();
    const float rel    = release.load();
    const float vol    = masterVolume.load();

    // Snap source pointers under lock; skip if source unavailable
    const float* srcL = nullptr;
    const float* srcR = nullptr;
    int          srcLen = 0;
    {
        juce::ScopedTryLock stl (sourceLock);
        if (stl.isLocked())
        {
            srcLen = sourceBuffer.getNumSamples();
            srcL   = sourceBuffer.getReadPointer(0);
            srcR   = sourceBuffer.getNumChannels() > 1
                   ? sourceBuffer.getReadPointer(1)
                   : srcL;
        }
    }
    if (srcLen == 0 || srcL == nullptr) return;

    // Render each voice into a temp buffer, then mix into main buffer
    juce::AudioBuffer<float> voiceBuf (2, numSamples);
    voiceBuf.clear();

    float* vL = voiceBuf.getWritePointer(0);
    float* vR = voiceBuf.getWritePointer(1);

    for (auto& v : voices)
    {
        if (!v.isActive()) continue;
        v.processBlock (vL, vR, numSamples,
                        srcL, srcR, srcLen,
                        pos, gs, sp, den, psc, es, rev, spread,
                        att, dec, sus, rel);
    }

    // Mix into main buffer
    if (buffer.getNumChannels() >= 2)
    {
        buffer.addFrom (0, 0, voiceBuf, 0, 0, numSamples, vol);
        buffer.addFrom (1, 0, voiceBuf, 1, 0, numSamples, vol);
    }
    else if (buffer.getNumChannels() == 1)
    {
        for (int i = 0; i < numSamples; ++i)
            buffer.getWritePointer(0)[i] += (vL[i] + vR[i]) * 0.5f * vol;
    }
}

bool GranularEngine::loadSourceFile (const juce::File& file)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr)
        return false;

    const int numChannels = juce::jmin (2, static_cast<int>(reader->numChannels));
    const int numSamples  = static_cast<int>(reader->lengthInSamples);
    if (numSamples <= 0)
        return false;

    juce::AudioBuffer<float> loaded (numChannels, numSamples);
    reader->read (&loaded, 0, numSamples, 0, true, true);

    // Resample to engine's sample rate if needed
    juce::AudioBuffer<float> resampled;
    if (reader->sampleRate != sampleRate && sampleRate > 0.0)
    {
        const double ratio = sampleRate / reader->sampleRate;
        const int newLen   = static_cast<int>(numSamples * ratio);
        resampled.setSize (numChannels, newLen, false, true, false);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            juce::LagrangeInterpolator interp;
            interp.reset();
            interp.process (1.0 / ratio,
                            loaded.getReadPointer(ch),
                            resampled.getWritePointer(ch),
                            newLen);
        }
    }
    else
    {
        resampled = loaded;
    }

    // Make stereo if mono
    if (resampled.getNumChannels() == 1)
    {
        juce::AudioBuffer<float> stereo (2, resampled.getNumSamples());
        stereo.copyFrom (0, 0, resampled, 0, 0, resampled.getNumSamples());
        stereo.copyFrom (1, 0, resampled, 0, 0, resampled.getNumSamples());
        resampled = stereo;
    }

    {
        juce::ScopedLock sl (sourceLock);
        sourceBuffer  = resampled;
        sourceFilePath = file.getFullPathName();
    }

    return true;
}

juce::ValueTree GranularEngine::getState() const
{
    juce::ValueTree tree ("GranularEngine");
    tree.setProperty ("sourceFile", sourceFilePath, nullptr);
    return tree;
}

void GranularEngine::setState (const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;
    juce::String path = tree.getProperty ("sourceFile", juce::String()).toString();
    if (path.isNotEmpty())
        loadSourceFile (juce::File (path));
}
