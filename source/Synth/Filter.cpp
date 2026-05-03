#include "Filter.h"

Filter::Filter()
{
    auto initFilter = [](SVFFilter& f)
    {
        f.setType(SVFFilter::Type::lowpass);
        f.setCutoffFrequency(20000.0f);
        f.setResonance(0.707f);
    };
    initFilter(filter);
    initFilter(filter2x);
    initFilter(filter4x);
}

void Filter::prepareFilter(SVFFilter& f, double sr)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;
    f.prepare(spec);
    f.reset();
}

void Filter::setCutoff(float c)
{
    cutoff = juce::jlimit(20.0f, 20000.0f, c);
    filter.setCutoffFrequency(cutoff);
    filter2x.setCutoffFrequency(cutoff);
    filter4x.setCutoffFrequency(cutoff);
    if (filterType == FilterType::Notch) updateNotchCoeffs();
}

void Filter::setResonance(float r)
{
    resonance = juce::jlimit(0.1f, 10.0f, r);
    filter.setResonance(resonance);
    filter2x.setResonance(resonance);
    filter4x.setResonance(resonance);
    if (filterType == FilterType::Notch) updateNotchCoeffs();
}

void Filter::setSampleRate(float sr)
{
    sampleRate = (sr > 0.0f && sr <= 1000000.0f) ? sr : 44100.0f;
    prepareFilter(filter,   static_cast<double>(sampleRate));
    prepareFilter(filter2x, static_cast<double>(sampleRate) * 2.0);
    prepareFilter(filter4x, static_cast<double>(sampleRate) * 4.0);
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);
    filter2x.setCutoffFrequency(cutoff);
    filter2x.setResonance(resonance);
    filter4x.setCutoffFrequency(cutoff);
    filter4x.setResonance(resonance);
    prepareNotchFilter(notchFilter,   static_cast<double>(sampleRate));
    prepareNotchFilter(notchFilter2x, static_cast<double>(sampleRate) * 2.0);
    prepareNotchFilter(notchFilter4x, static_cast<double>(sampleRate) * 4.0);
    updateNotchCoeffs();
}

void Filter::setOversamplingMode(OversamplingMode mode)
{
    osMode = mode;
}

void Filter::setFilterType(FilterType t)
{
    filterType = t;
    if (t == FilterType::Notch)
    {
        updateNotchCoeffs();
        return;
    }
    SVFFilter::Type svfType;
    switch (t)
    {
        case FilterType::HP: svfType = SVFFilter::Type::highpass; break;
        case FilterType::BP: svfType = SVFFilter::Type::bandpass; break;
        default:             svfType = SVFFilter::Type::lowpass;  break;
    }
    filter.setType(svfType);
    filter2x.setType(svfType);
    filter4x.setType(svfType);
}

void Filter::prepareNotchFilter(IIRFilter& f, double sr)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;
    f.prepare(spec);
    f.reset();
}

void Filter::updateNotchCoeffs()
{
    const float Q = juce::jmax(0.1f, resonance);
    *notchFilter.coefficients   = *juce::dsp::IIR::Coefficients<float>::makeNotch(static_cast<double>(sampleRate), cutoff, Q);
    *notchFilter2x.coefficients = *juce::dsp::IIR::Coefficients<float>::makeNotch(static_cast<double>(sampleRate) * 2.0, cutoff, Q);
    *notchFilter4x.coefficients = *juce::dsp::IIR::Coefficients<float>::makeNotch(static_cast<double>(sampleRate) * 4.0, cutoff, Q);
}

float Filter::processNotch(float input)  { return notchFilter.processSample(input); }
float Filter::processNotch2x(float input)
{
    notchFilter2x.processSample(input * 2.0f);
    return notchFilter2x.processSample(0.0f);
}
float Filter::processNotch4x(float input)
{
    notchFilter4x.processSample(input * 4.0f);
    notchFilter4x.processSample(0.0f);
    notchFilter4x.processSample(0.0f);
    return notchFilter4x.processSample(0.0f);
}

float Filter::process2x(float input)
{
    // Zero-insertion upsampling at 2x: gain-compensate, then insert zero; decimate by taking last output.
    filter2x.processSample(0, input * 2.0f);
    return filter2x.processSample(0, 0.0f);
}

float Filter::process4x(float input)
{
    filter4x.processSample(0, input * 4.0f);
    filter4x.processSample(0, 0.0f);
    filter4x.processSample(0, 0.0f);
    return filter4x.processSample(0, 0.0f);
}

float Filter::process(float input)
{
    OversamplingMode effectiveMode = osMode;
    if (osMode == OversamplingMode::Auto)
        effectiveMode = (cutoff > 5000.0f) ? OversamplingMode::X2 : OversamplingMode::Off;

    if (filterType == FilterType::Notch)
    {
        switch (effectiveMode)
        {
            case OversamplingMode::X2: return processNotch2x(input);
            case OversamplingMode::X4: return processNotch4x(input);
            default:                   return processNotch(input);
        }
    }

    switch (effectiveMode)
    {
        case OversamplingMode::X2:  return process2x(input);
        case OversamplingMode::X4:  return process4x(input);
        default:                    return filter.processSample(0, input);
    }
}

void Filter::reset()
{
    filter.reset();
    filter2x.reset();
    filter4x.reset();
    notchFilter.reset();
    notchFilter2x.reset();
    notchFilter4x.reset();
}
