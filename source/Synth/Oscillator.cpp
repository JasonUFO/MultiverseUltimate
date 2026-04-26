#include "Oscillator.h"
#include <cmath>
#include <random>

Oscillator::Oscillator() : phase(0.0f), sampleRate(44100.0f) {}

void Oscillator::setFrequency(float freq)
{
    frequency = freq;
}

void Oscillator::setWaveform(WaveformType type)
{
    if (type != waveform)
    {
        prevWaveform = waveform;
        waveform = type;
        crossfadeAlpha = 0.0f;
    }
}

void Oscillator::setPulseWidth(float width)
{
    pulseWidth = juce::jlimit(0.01f, 0.99f, width);
}

void Oscillator::setSampleRate(float sr)
{
    sampleRate = sr;
}

float Oscillator::generateSample(WaveformType t, float ph, float pw)
{
    switch (t)
    {
        case WaveformType::Sine:     return std::sin(ph * 2.0f * M_PI);
        case WaveformType::Saw:      return 2.0f * (ph - 0.5f);
        case WaveformType::Square:   return ph < pw ? 1.0f : -1.0f;
        case WaveformType::Triangle: return 4.0f * std::abs(ph - 0.5f) - 1.0f;
        case WaveformType::Noise:
        {
            static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            static thread_local std::mt19937 gen;
            return dist(gen);
        }
        case WaveformType::Sample:   return ph < 0.5f ? 1.0f : -1.0f;
    }
    return 0.0f;
}

float Oscillator::process()
{
    float output;
    const float phaseIncrement = frequency / sampleRate;

    if (crossfadeAlpha < 1.0f)
    {
        const float a = generateSample(prevWaveform, phase, pulseWidth);
        const float b = generateSample(waveform,     phase, pulseWidth);
        output = a + crossfadeAlpha * (b - a);
        crossfadeAlpha = juce::jmin(1.0f, crossfadeAlpha + crossfadeRate);
    }
    else
    {
        output = generateSample(waveform, phase, pulseWidth);
    }

    phase += phaseIncrement;
    if (phase >= 1.0f) phase -= 1.0f;

    return output;
}

void Oscillator::reset()
{
    phase = 0.0f;
}