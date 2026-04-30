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
