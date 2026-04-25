#pragma once

class Effect
{
public:
    virtual ~Effect() = default;
    
    virtual void prepare(double sampleRate, int samplesPerBlock) = 0;
    virtual float process(float input) = 0;
    virtual void reset() = 0;
    
    virtual void setBypass(bool bypass) { bypassed = bypass; }
    bool isBypassed() const { return bypassed; }
    
protected:
    double sampleRate = 44100.0;
    bool bypassed = false;
};