#include "ktmeAdpcmYamaha.h"

#include <stdint.h>

/*
Encode and decode algorithms for
YMZ280B / AICA ADPCM.

2019 by superctr.
2024 adapt to ktme by usernameak
*/

#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

static int16_t ktmeAdpcmYamahaStep(uint8_t step, ktmeAdpcmYamahaState *state) {
    static const int step_table[8] = {
        230, 230, 230, 230, 307, 409, 512, 614
    };

    int sign = step & 8;
    int delta = step & 7;
    int diff = ((1+(delta<<1)) * state->stepSize) >> 3;
    int newval = state->history;
    int nstep = (step_table[delta] * state->stepSize) >> 8;
    // Only found in the official AICA encoder
    // but it's possible all chips (including ADPCM-B) does this.
    diff = CLAMP(diff, 0, 32767);
    if (sign > 0)
        newval -= diff;
    else
        newval += diff;
    //state->stepSize = CLAMP(nstep, 511, 32767);
    state->stepSize = CLAMP(nstep, 127, 24576);
    state->history = newval = CLAMP(newval, -32768, 32767);
    return newval;
}

void ktmeAdpcmYamahaInitState(ktmeAdpcmYamahaState *state) {
    state->stepSize = 127;
    state->history  = 0;
}

void ktmeAdpcmYamahaDecodeTwoSamples(ktmeAdpcmYamahaState *state, uint8_t inByte, int16_t *outSamples) {
    uint8_t nibble = 4;

    for (int i = 0; i < 2; i++) {
        int8_t step = inByte << nibble;
        step >>= 4;
        nibble ^= 4;
        *outSamples++ = ktmeAdpcmYamahaStep(step, state);
    }
}
