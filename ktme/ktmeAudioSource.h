#pragma once

#include "ktmeCommon.h"
#include "ktmeFrame.h"

typedef struct ktmeAudioSourceBase ktmeAudioSourceBase;

#define KTME_AUDIO_SOURCE_BASE_FUNCS                                                             \
    ktmeStatus (*pullAudio)(ktmeAudioSourceBase * src, size_t numFrames, ktmeFrameS32 * frames); \
    ktmeStatus (*seek)(ktmeAudioSourceBase * src, ktmeSeekWhence whence, int32_t timeMS);        \
    ktmeStatus (*stop)(ktmeAudioSourceBase * src)

typedef struct ktmeAudioSourceFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;
} ktmeAudioSourceFuncs;

struct ktmeAudioSourceBase {
    const ktmeAudioSourceFuncs *m_funcs;
};
