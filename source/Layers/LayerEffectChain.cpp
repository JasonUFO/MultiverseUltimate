#include "LayerEffectChain.h"

LayerEffectChain::LayerEffectChain()
{
    // Sensible defaults for each effect
    for (int c = 0; c < 2; ++c)
    {
        chorus[c].setRate(0.5f);
        chorus[c].setDepth(0.4f);
        distortion[c].setDrive(0.4f);
        distortion[c].setMix(1.0f);
        eq[c].setLowGain(0.0f);
        eq[c].setMidGain(0.0f);
        eq[c].setHighGain(0.0f);
        compressor[c].setThreshold(-12.0f);
        compressor[c].setRatio(4.0f);
        compressor[c].setAttack(10.0f);
        compressor[c].setRelease(100.0f);
    }
    for (int c = 0; c < 2; ++c) { delay[c].setTime(0.25f); delay[c].setFeedback(0.3f); delay[c].setMix(0.3f); }
    reverb.setRoomSize(0.5f);
    reverb.setWetLevel(0.3f);
    reverb.setDryLevel(0.7f);
}

void LayerEffectChain::prepare(double sampleRate, int samplesPerBlock)
{
    for (int c = 0; c < 2; ++c)
    {
        chorus[c].prepare(sampleRate, samplesPerBlock);
        distortion[c].prepare(sampleRate, samplesPerBlock);
        eq[c].prepare(sampleRate, samplesPerBlock);
        compressor[c].prepare(sampleRate, samplesPerBlock);
    }
    delay[0].prepare(sampleRate, samplesPerBlock);
    delay[1].prepare(sampleRate, samplesPerBlock);
    reverb.prepare(sampleRate, samplesPerBlock);
}

void LayerEffectChain::reset()
{
    for (int c = 0; c < 2; ++c)
    {
        chorus[c].reset();
        distortion[c].reset();
        eq[c].reset();
        compressor[c].reset();
    }
    delay[0].reset();
    delay[1].reset();
    reverb.reset();
}

void LayerEffectChain::processBlock(juce::AudioBuffer<float>& buffer, int numSamples)
{
    auto* L = buffer.getWritePointer(0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    // Process mono effects (chorus, distortion, EQ, compressor, delay) per channel
    auto processMono = [&](int effectIdx, auto& eff0, auto& eff1)
    {
        if (!slots[effectIdx].enabled) return;
        const float mix = slots[effectIdx].mix;
        const float dry = 1.0f - mix;
        for (int i = 0; i < numSamples; ++i)
        {
            L[i] = dry * L[i] + mix * eff0.process(L[i]);
            if (R) R[i] = dry * R[i] + mix * eff1.process(R[i]);
        }
    };

    processMono(Chorus,      chorus[0],     chorus[1]);
    processMono(Distortion,  distortion[0], distortion[1]);
    processMono(EQ,          eq[0],         eq[1]);
    processMono(Compressor,  compressor[0], compressor[1]);
    processMono(Delay,       delay[0],      delay[1]);

    // Reverb is stereo — handled separately
    if (slots[Reverb].enabled)
    {
        const float mix = slots[Reverb].mix;
        const float dry = 1.0f - mix;
        // Save dry signal, process wet, blend
        juce::AudioBuffer<float> wetBuf;
        wetBuf.setSize(2, numSamples);
        wetBuf.copyFrom(0, 0, buffer, 0, 0, numSamples);
        if (R) wetBuf.copyFrom(1, 0, buffer, 1, 0, numSamples);

        auto* wL = wetBuf.getWritePointer(0);
        auto* wR = wetBuf.getWritePointer(1);
        reverb.processBlock(wL, wR, numSamples);

        for (int i = 0; i < numSamples; ++i)
        {
            L[i] = dry * L[i] + mix * wL[i];
            if (R) R[i] = dry * R[i] + mix * wR[i];
        }
    }
}

void LayerEffectChain::setEnabled(int idx, bool on)
{
    if (idx >= 0 && idx < NumEffects) slots[idx].enabled = on;
}

void LayerEffectChain::setMix(int idx, float mix)
{
    if (idx >= 0 && idx < NumEffects) slots[idx].mix = juce::jlimit(0.0f, 1.0f, mix);
}

bool  LayerEffectChain::isEnabled(int idx) const { return (idx >= 0 && idx < NumEffects) ? slots[idx].enabled : false; }
float LayerEffectChain::getMix(int idx)    const { return (idx >= 0 && idx < NumEffects) ? slots[idx].mix : 0.0f; }

juce::ValueTree LayerEffectChain::getState() const
{
    juce::ValueTree tree("LayerFX");
    for (int i = 0; i < NumEffects; ++i)
    {
        juce::ValueTree slot("Slot");
        slot.setProperty("idx",     i,                   nullptr);
        slot.setProperty("enabled", slots[i].enabled,    nullptr);
        slot.setProperty("mix",     slots[i].mix,        nullptr);
        tree.appendChild(slot, nullptr);
    }
    return tree;
}

void LayerEffectChain::setState(const juce::ValueTree& tree)
{
    if (!tree.isValid()) return;
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        auto slot = tree.getChild(i);
        int idx = static_cast<int>(slot.getProperty("idx", i));
        if (idx >= 0 && idx < NumEffects)
        {
            slots[idx].enabled = static_cast<bool>(slot.getProperty("enabled", false));
            slots[idx].mix     = static_cast<float>(slot.getProperty("mix", 0.5f));
        }
    }
}
