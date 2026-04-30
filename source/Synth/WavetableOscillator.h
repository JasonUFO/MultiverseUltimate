#pragma once
#include <JuceHeader.h>

class WavetableOscillator
{
public:
    WavetableOscillator();
    ~WavetableOscillator() = default;

    void prepare(float sampleRate);
    void reset();

    void setFrequency(float freqHz);
    void setWavePosition(float position); // 0.0 - 1.0, selects wavetable index

    void loadWavetable(const juce::AudioBuffer<float>& audio, int tableSize = 2048);
    void generateStandardTable(int tableIndex, int waveform); // 0=sine 1=saw 2=square 3=tri

    float process();

    void setTableCount(int count) { tableCount = count; }

private:
    float sampleRate = 44100.0f;
    float frequency = 440.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;

    int tableCount = 4;
    int currentTable = 0;
    int tableSize = 2048;

    juce::AudioBuffer<float> wavetables; // interleaved: tableCount tables × tableSize samples

    void generateTables();
    float readTable(int tableIdx, float phase);
};
