#include "WavetableOscillator.h"
#include <cmath>

WavetableOscillator::WavetableOscillator()
{
    generateTables();
}

void WavetableOscillator::prepare(float sr)
{
    sampleRate = sr;
    phaseIncrement = frequency / sampleRate;
}

void WavetableOscillator::reset()
{
    phase = 0.0f;
}

void WavetableOscillator::setFrequency(float freqHz)
{
    frequency = juce::jmax(0.0f, freqHz);
    phaseIncrement = frequency / sampleRate;
}

void WavetableOscillator::setWavePosition(float position)
{
    float idx = position * (tableCount - 1);
    currentTable = juce::jlimit(0, tableCount - 1, static_cast<int>(std::round(idx)));
}

void WavetableOscillator::loadWavetable(const juce::AudioBuffer<float>& audio, int newTableSize)
{
    if (audio.getNumSamples() == 0) return;
    tableSize = newTableSize;
    tableCount = 1;
    wavetables.setSize(1, tableSize + 1); // +1 for interpolation wrap

    const float* src = audio.getReadPointer(0);
    float* dst = wavetables.getWritePointer(0);
    int srcLen = audio.getNumSamples();

    for (int i = 0; i < tableSize; ++i)
    {
        float readPos = (static_cast<float>(i) / static_cast<float>(tableSize)) * static_cast<float>(srcLen - 1);
        int i0 = static_cast<int>(readPos);
        int i1 = juce::jmin(i0 + 1, srcLen - 1);
        float frac = readPos - static_cast<float>(i0);
        dst[i] = src[i0] + frac * (src[i1] - src[i0]);
    }
    dst[tableSize] = dst[0]; // wrap for interpolation
}

void WavetableOscillator::generateStandardTable(int tableIndex, int waveform)
{
    if (tableIndex < 0 || tableIndex >= tableCount) return;
    if (wavetables.getNumSamples() < tableSize + 1)
        wavetables.setSize(tableCount, tableSize + 1);

    float* dst = wavetables.getWritePointer(tableIndex);
    const float twoPi = 2.0f * M_PI;

    for (int i = 0; i < tableSize; ++i)
    {
        float ph = static_cast<float>(i) / static_cast<float>(tableSize);
        float sample = 0.0f;

        switch (waveform)
        {
            case 0: // sine
                sample = std::sin(ph * twoPi);
                break;
            case 1: // saw
                sample = 2.0f * ph - 1.0f;
                break;
            case 2: // square
                sample = ph < 0.5f ? 1.0f : -1.0f;
                break;
            case 3: // triangle
                sample = 2.0f * std::abs(2.0f * ph - 1.0f) - 1.0f;
                break;
            default:
                sample = std::sin(ph * twoPi);
                break;
        }
        dst[i] = sample;
    }
    dst[tableSize] = dst[0];
}

void WavetableOscillator::generateTables()
{
    tableCount = 4;
    tableSize = 2048;
    wavetables.setSize(tableCount, tableSize + 1);

    generateStandardTable(0, 0); // sine
    generateStandardTable(1, 1); // saw
    generateStandardTable(2, 2); // square
    generateStandardTable(3, 3); // triangle
}

float WavetableOscillator::readTable(int tableIdx, float ph)
{
    const float* table = wavetables.getReadPointer(tableIdx);
    float readPos = ph * static_cast<float>(tableSize);
    int i0 = static_cast<int>(readPos);
    float frac = readPos - static_cast<float>(i0);
    // tableSize+1 has wrap sample at index tableSize
    float s0 = table[i0];
    float s1 = table[i0 + 1];
    return s0 + frac * (s1 - s0);
}

float WavetableOscillator::process()
{
    float output = readTable(currentTable, phase);
    phase += phaseIncrement;
    if (phase >= 1.0f) phase -= 1.0f;
    return output;
}

void WavetableOscillator::setSample(int frame, int index, float value)
{
    if (frame < 0 || frame >= tableCount || index < 0 || index >= tableSize) return;
    wavetables.getWritePointer(frame)[index] = value;
    wavetables.getWritePointer(frame)[tableSize] = wavetables.getReadPointer(frame)[0]; // update wrap
}

float WavetableOscillator::getSample(int frame, int index) const
{
    if (frame < 0 || frame >= tableCount || index < 0 || index >= tableSize) return 0.0f;
    return wavetables.getReadPointer(frame)[index];
}

void WavetableOscillator::clearFrame(int frame)
{
    if (frame < 0 || frame >= tableCount) return;
    float* dst = wavetables.getWritePointer(frame);
    std::fill_n(dst, tableSize, 0.0f);
    dst[tableSize] = 0.0f;
}

void WavetableOscillator::normalizeFrame(int frame)
{
    if (frame < 0 || frame >= tableCount) return;
    const float* src = wavetables.getReadPointer(frame);
    float maxVal = 0.0f;
    for (int i = 0; i < tableSize; ++i) maxVal = juce::jmax(maxVal, std::abs(src[i]));
    if (maxVal > 0.0f)
    {
        float scale = 1.0f / maxVal;
        float* dst = wavetables.getWritePointer(frame);
        for (int i = 0; i < tableSize; ++i) dst[i] *= scale;
        dst[tableSize] = dst[0];
    }
}

void WavetableOscillator::loadMultiCycleWavetable(const juce::AudioBuffer<float>& audio, int numFrames)
{
    if (audio.getNumSamples() == 0) return;
    tableCount = juce::jmax(1, juce::jmin(8, numFrames));
    tableSize = 2048;
    wavetables.setSize(tableCount, tableSize + 1);

    const float* src = audio.getReadPointer(0);
    int srcLen = audio.getNumSamples();
    int samplesPerFrame = srcLen / tableCount;

    for (int frame = 0; frame < tableCount; ++frame)
    {
        float* dst = wavetables.getWritePointer(frame);
        int frameStart = frame * samplesPerFrame;

        for (int i = 0; i < tableSize; ++i)
        {
            float readPos = static_cast<float>(frameStart) + (static_cast<float>(i) / static_cast<float>(tableSize)) * static_cast<float>(samplesPerFrame - 1);
            int i0 = static_cast<int>(readPos);
            int i1 = juce::jmin(i0 + 1, srcLen - 1);
            float frac = readPos - static_cast<float>(i0);
            dst[i] = src[i0] + frac * (src[i1] - src[i0]);
        }
        dst[tableSize] = dst[0];
    }
}

void WavetableOscillator::generateFormula(int frame, std::function<float(float)> generator)
{
    if (frame < 0 || frame >= tableCount) return;
    float* dst = wavetables.getWritePointer(frame);
    for (int i = 0; i < tableSize; ++i)
    {
        float ph = static_cast<float>(i) / static_cast<float>(tableSize);
        dst[i] = generator(ph);
    }
    dst[tableSize] = dst[0];
}

void WavetableOscillator::processFFT(int frame, bool forward)
{
    // Simplified FFT placeholder — just normalize the frame
    normalizeFrame(frame);
}

void WavetableOscillator::fadeFrame(int frame, float startGain, float endGain)
{
    if (frame < 0 || frame >= tableCount) return;
    float* dst = wavetables.getWritePointer(frame);
    for (int i = 0; i < tableSize; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(tableSize - 1);
        dst[i] *= startGain + t * (endGain - startGain);
    }
    dst[tableSize] = dst[0];
}

void WavetableOscillator::reverseFrame(int frame)
{
    if (frame < 0 || frame >= tableCount) return;
    float* dst = wavetables.getWritePointer(frame);
    for (int i = 0; i < tableSize / 2; ++i)
    {
        float tmp = dst[i];
        dst[i] = dst[tableSize - 1 - i];
        dst[tableSize - 1 - i] = tmp;
    }
    dst[tableSize] = dst[0];
}
