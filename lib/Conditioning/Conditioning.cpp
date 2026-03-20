#include "Conditioning.h"
#include "Config.h"

// clampValue I use for saturation, it helps to keep the raw value within the expected range
static float clampValue(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// To avoid the median impuls noise affecting the weighted average too much
static float medianOfWindow(const float *buf, uint8_t count)
{
    float tmp[5];

    for (uint8_t i = 0; i < count; i++) {
        tmp[i] = buf[i];
    }

    // Small insertion sort is enough for a tiny fixed window.
    for (uint8_t i = 1; i < count; i++) {
        float key = tmp[i];
        int8_t j = (int8_t)i - 1;

        while (j >= 0 && tmp[j] > key) {
            tmp[j + 1] = tmp[j];
            j--;
        }
        tmp[j + 1] = key;
    }

    if ((count % 2U) == 0U) {
        uint8_t upper = count / 2U;
        uint8_t lower = upper - 1U;
        return (tmp[lower] + tmp[upper]) * 0.5f;
    }

    return tmp[count / 2U];
}

// to make the result smoother
static float weightedAverage(const ConditioningState *state)
{
    static const uint8_t weights[5] = {
        COND_WEIGHT_0,
        COND_WEIGHT_1,
        COND_WEIGHT_2,
        COND_WEIGHT_3,
        COND_WEIGHT_4
    };

    uint8_t count = state->sampleCount;
    if (count == 0) return 0.0f;

    float sum = 0.0f;
    uint16_t wsum = 0;

    // For startup (count < window), use the most recent 'count' weights.
    uint8_t wStart = (uint8_t)(COND_WINDOW_SIZE - count);

    for (uint8_t i = 0; i < count; i++) {
        int8_t idx = (int8_t)state->medianHead - 1 - (int8_t)i;
        while (idx < 0) idx += COND_WINDOW_SIZE;

        uint8_t w = weights[(uint8_t)(COND_WINDOW_SIZE - 1 - i)];
        if (count < COND_WINDOW_SIZE) {
            w = weights[(uint8_t)(wStart + (count - 1 - i))];
        }

        sum += state->medianBuffer[(uint8_t)idx] * (float)w;
        wsum += w;
    }

    if (wsum == 0) return 0.0f;
    return sum / (float)wsum;
}

void Conditioning::init(ConditioningState *state)
{
    for (uint8_t i = 0; i < COND_WINDOW_SIZE; i++) {
        state->satBuffer[i] = 0.0f;
        state->medianBuffer[i] = 0.0f;
    }

    state->satHead = 0;
    state->medianHead = 0;
    state->sampleCount = 0;
}

// aw -> sat -> median -> weighted
void Conditioning::processSample(ConditioningState *state, float rawValue, ConditioningSnapshot *out)
{
    out->raw = rawValue;
    out->saturated = clampValue(rawValue, COND_SAT_MIN_C, COND_SAT_MAX_C);

    state->satBuffer[state->satHead] = out->saturated;
    state->satHead = (uint8_t)((state->satHead + 1U) % COND_WINDOW_SIZE);

    if (state->sampleCount < COND_WINDOW_SIZE) {
        state->sampleCount++;
    }

    float satOrdered[5];
    for (uint8_t i = 0; i < state->sampleCount; i++) {
        int8_t idx = (int8_t)state->satHead - 1 - (int8_t)i;
        while (idx < 0) idx += COND_WINDOW_SIZE;
        satOrdered[i] = state->satBuffer[(uint8_t)idx];
    }

    out->median = medianOfWindow(satOrdered, state->sampleCount);

    state->medianBuffer[state->medianHead] = out->median;
    state->medianHead = (uint8_t)((state->medianHead + 1U) % COND_WINDOW_SIZE);

    out->weighted = weightedAverage(state);
    out->valid = true;
}
