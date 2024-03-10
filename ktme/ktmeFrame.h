#pragma once

#include <stdint.h>

typedef struct ktmeFrameS32 {
    int32_t left, right;
} ktmeFrameS32;

typedef struct ktmeFrameU32 {
    uint32_t left, right;
} ktmeFrameU32;

typedef enum ktmeSampleFormat {
    KTME_SAMPLE_FORMAT_U8,
    KTME_SAMPLE_FORMAT_S16,
    KTME_SAMPLE_FORMAT_S24,
    KTME_SAMPLE_FORMAT_S32,
} ktmeSampleFormat;

inline size_t ktmeFrameSize(ktmeSampleFormat format, int numChannels) {
    switch (format) {
    case KTME_SAMPLE_FORMAT_U8: return numChannels * 1;
    case KTME_SAMPLE_FORMAT_S16: return numChannels * 2;
    case KTME_SAMPLE_FORMAT_S24: return numChannels * 3;
    case KTME_SAMPLE_FORMAT_S32: return numChannels * 4;
    }
    return numChannels * 1;
}
