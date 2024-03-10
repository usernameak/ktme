#pragma once

#include <stdint.h>

typedef struct ktmeAdpcmYamahaState {
    int16_t history;
    int16_t stepSize;
} ktmeAdpcmYamahaState;

void ktmeAdpcmYamahaInitState(ktmeAdpcmYamahaState *state);
void ktmeAdpcmYamahaDecodeTwoSamples(ktmeAdpcmYamahaState *state, uint8_t inByte, int16_t *outSamples);
