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
    waveform = type;
}

void Oscillator::setPulseWidth(float width)
{
    pulseWidth = juce::jlimit(0.01f, 0.99f, width);
}

void Oscillator::setSampleRate(float sr)
{
    sampleRate = sr;
}

float Oscillator::process()
{
    float output = 0.0f;
    float phaseIncrement = frequency / sampleRate;
    
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
        {
            static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            static thread_local std::mt19937 gen;
            output = dist(gen);
            break;
        }
            
        case WaveformType::Sample:
            output = phase < 0.5f ? 1.0f : -1.0f;
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