#ifndef CONDITIONING_H
#define CONDITIONING_H

#include <stdint.h>

struct ConditioningSnapshot {
    float raw;
    float saturated;
    float median;
    float weighted;
    bool  valid;
};

struct ConditioningState {
    float satBuffer[5];
    float medianBuffer[5];
    uint8_t satHead;
    uint8_t medianHead;
    uint8_t sampleCount;
};

namespace Conditioning
{
    void init(ConditioningState *state);
    void processSample(ConditioningState *state, float rawValue, ConditioningSnapshot *out);
}

#endif
