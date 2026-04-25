#pragma once
#include "FMOperator.h"
#include "FMAlgorithm.h"
#include <array>

class FMVoice
{
public:
    void setSampleRate(double sr);
    void setAlgorithm(int index);

    // Per-operator parameter control (opIndex 0-3)
    void setOperatorParams(int opIndex,
                           float ratio,
                           float level,
                           float feedback,
                           float attack,
                           float decay,
                           float sustain,
                           float release);

    void noteOn(int midiNote, float velocity);
    void noteOff();
    bool isActive() const;
    int getMidiNote() const { return midiNote; }

    float process();

private:
    std::array<FMOperator, FM_OP_COUNT> operators;
    int algorithmIndex = 0;
    int midiNote = -1;
    float velocity = 0.0f;
};
