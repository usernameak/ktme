#pragma once

#include "ktmeEngine.h"
#include "ktmeFrame.h"

typedef struct ktmeAudioSourceBase ktmeAudioSourceBase;

typedef struct ktmeAudioSourceFuncs {
    void (*pullAudio)(ktmeAudioSourceBase *mixer, size_t numFrames, ktmeFrameS32 *frames);
} ktmeAudioSourceFuncs;

#define KTME_AUDIO_SOURCE_BASE_FIELDS    \
    const ktmeAudioSourceFuncs *m_funcs; \
    ktmeEngine *m_engine

struct ktmeAudioSourceBase {
    KTME_AUDIO_SOURCE_BASE_FIELDS;
};
