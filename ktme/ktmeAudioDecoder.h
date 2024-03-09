#pragma once

#include "ktmeAudioSource.h"

typedef struct ktmeAudioDecoderFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;
} ktmeAudioDecoderFuncs;

struct ktmeAudioDecoderBase {
    const ktmeAudioDecoderFuncs *m_funcs;
};
