#pragma once

#include "ktmeCommon.h"
#include "ktmeFrame.h"

typedef struct ktmeAudioSourceBase ktmeAudioSourceBase;

typedef struct ktmeAudioSourceCaps {
    uint32_t sampleRate;
    ktmeSampleFormat sampleFormat;
    uint16_t numChannels;
} ktmeAudioSourceCaps;

#define KTME_AUDIO_SOURCE_BASE_FUNCS                                                      \
    ktmeStatus (*pullAudio)(ktmeAudioSourceBase * src, size_t numFrames, void *frames);   \
    ktmeStatus (*seek)(ktmeAudioSourceBase * src, ktmeSeekWhence whence, int32_t timeMS); \
    ktmeStatus (*stop)(ktmeAudioSourceBase * src);                                        \
    ktmeStatus (*getCaps)(ktmeAudioSourceBase * src, ktmeAudioSourceCaps * caps)

typedef struct ktmeAudioSourceFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;
} ktmeAudioSourceFuncs;

struct ktmeAudioSourceBase {
    const ktmeAudioSourceFuncs *m_funcs;
};

ktmeStatus ktmeAudioSourceUnsupportedSeek(ktmeAudioSourceBase *src, ktmeSeekWhence whence, int32_t timeMS);
ktmeStatus ktmeAudioSourceUnsupportedStop(ktmeAudioSourceBase *src);

inline ktmeStatus ktmeAudioSourcePullAudio(ktmeAudioSourceBase *src, size_t numFrames, ktmeFrameS32 *frames) {
    return src->m_funcs->pullAudio(src, numFrames, frames);
}

inline ktmeStatus ktmeAudioSourceGetCaps(ktmeAudioSourceBase *src, ktmeAudioSourceCaps *caps) {
    return src->m_funcs->getCaps(src, caps);
}
