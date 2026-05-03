#pragma once
#include <JuceHeader.h>
#include "../Effects/Chorus.h"
#include "../Effects/Distortion.h"
#include "../Effects/EQ.h"
#include "../Effects/Compressor.h"
#include "../Effects/Delay.h"
#include "../Effects/Reverb.h"

class LayerEffectChain
{
public:
    enum EffectIndex { Chorus = 0, Distortion = 1, EQ = 2, Compressor = 3, Delay = 4, Reverb = 5, NumEffects = 6 };

    LayerEffectChain();

    void prepare(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, int numSamples);
    void reset();

    void setEnabled(int idx, bool on);
    void setMix(int idx, float mix);   // 0..1
    bool isEnabled(int idx) const;
    float getMix(int idx) const;

    // Per-effect param setters (subset — most critical controls)
    void setChorusRate(float hz)     { chorus[0].setRate(hz);   chorus[1].setRate(hz); }
    void setChorusDepth(float d)     { chorus[0].setDepth(d);   chorus[1].setDepth(d); }
    void setDistDrive(float dr)      { distortion[0].setDrive(dr); distortion[1].setDrive(dr); }
    void setEQGains(float lo, float mid, float hi)
    {
        for (int c = 0; c < 2; ++c) { eq[c].setLowGain(lo); eq[c].setMidGain(mid); eq[c].setHighGain(hi); }
    }
    void setCompThreshold(float t)   { for (int c = 0; c < 2; ++c) compressor[c].setThreshold(t); }
    void setCompRatio(float r)       { for (int c = 0; c < 2; ++c) compressor[c].setRatio(r); }
    void setDelayTime(float s)       { delay[0].setTime(s); delay[1].setTime(s); }
    void setDelayFeedback(float f)   { delay[0].setFeedback(f); delay[1].setFeedback(f); }
    void setReverbRoom(float r)      { reverb.setRoomSize(r); }
    void setReverbWet(float w)       { reverb.setWetLevel(w); }

    juce::ValueTree getState() const;
    void setState(const juce::ValueTree& tree);

private:
    struct Slot
    {
        bool  enabled = false;
        float mix     = 0.5f;
    };

    Slot slots[NumEffects];

    ChorusEffect     chorus[2];
    DistortionEffect distortion[2];
    EQEffect         eq[2];
    CompressorEffect compressor[2];
    DelayEffect      delay[2];
    ReverbEffect     reverb;
};
