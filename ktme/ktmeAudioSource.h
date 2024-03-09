#pragma once

#include "ktmeEngine.h"
#include "ktmeFrame.h"

typedef struct ktmeAudioSourceBase ktmeAudioSourceBase;

typedef enum ktmeSeekWhence {
    KTME_SEEK_START = 0,
    KTME_SEEK_END,
    KTME_SEEK_CUR
} ktmeSeekWhence;

#define KTME_AUDIO_SOURCE_BASE_FUNCS                                                             \
    ktmeStatus (*pullAudio)(ktmeAudioSourceBase * src, size_t numFrames, ktmeFrameS32 * frames); \
    ktmeStatus (*seek)(ktmeAudioSourceBase * src, ktmeSeekWhence whence, int32_t timeMS);        \
    ktmeStatus (*stop)(ktmeAudioSourceBase * src)

#define KTME_AUDIO_SOURCE_BASE_FIELDS \
    const ktmeAudioSourceFuncs *m_funcs;

typedef struct ktmeAudioSourceFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;
} ktmeAudioSourceFuncs;

struct ktmeAudioSourceBase {
    KTME_AUDIO_SOURCE_BASE_FIELDS;
};
