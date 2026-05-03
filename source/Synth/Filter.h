#pragma once
#include <JuceHeader.h>

class Filter
{
public:
    enum class OversamplingMode { Off, X2, X4, Auto };
    enum class FilterType { LP, HP, BP, Notch };

    Filter();

    void setCutoff(float cutoff);
    void setResonance(float resonance);
    void setSampleRate(float sr);
    void setOversamplingMode(OversamplingMode mode);
    void setFilterType(FilterType t);

    float process(float input);
    void reset();

    float getCutoff() const { return cutoff; }
    float getResonance() const { return resonance; }
    FilterType getFilterType() const { return filterType; }

private:
    using SVFFilter = juce::dsp::StateVariableTPTFilter<float>;
    using IIRFilter = juce::dsp::IIR::Filter<float>;

    SVFFilter filter;    // 1x
    SVFFilter filter2x;  // 2x sample rate
    SVFFilter filter4x;  // 4x sample rate

    IIRFilter notchFilter;
    IIRFilter notchFilter2x;
    IIRFilter notchFilter4x;

    float cutoff = 20000.0f;
    float resonance = 0.707f;
    float sampleRate = 44100.0f;
    OversamplingMode osMode = OversamplingMode::Off;
    FilterType filterType = FilterType::LP;

    void prepareFilter(SVFFilter& f, double sr);
    void prepareNotchFilter(IIRFilter& f, double sr);
    void updateNotchCoeffs();
    float process2x(float input);
    float process4x(float input);
    float processNotch(float input);
    float processNotch2x(float input);
    float processNotch4x(float input);
};