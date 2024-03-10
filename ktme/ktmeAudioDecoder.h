#pragma once

#include "ktmeAudioSource.h"
#include "ktmeDataSource.h"

typedef struct ktmeAudioDecoderBase ktmeAudioDecoderBase;

typedef struct ktmeAudioDecoderFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;

    ktmeStatus (*linkDataSource)(ktmeAudioDecoderBase *self, ktmeDataSource *dataSource);
} ktmeAudioDecoderFuncs;

struct ktmeAudioDecoderBase {
    const ktmeAudioDecoderFuncs *m_funcs;
};

inline ktmeStatus ktmeAudioDecoderLinkDataSource(ktmeAudioDecoderBase *self, ktmeDataSource *dataSource) {
    return self->m_funcs->linkDataSource(self, dataSource);
}
