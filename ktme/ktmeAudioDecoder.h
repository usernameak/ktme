#pragma once

#include "ktmeAudioSource.h"

typedef struct ktmeDataSource ktmeDataSource;

typedef struct ktmeDataSourceFuncs {
    void (*addRef)(ktmeDataSource *self);
    void (*release)(ktmeDataSource *self);

    ktmeStatus (*read)(ktmeDataSource *self, uint32_t numBytes);
    ktmeStatus (*seek)(ktmeDataSource *self, ktmeSeekWhence whence, int32_t numBytes);
} ktmeDataSourceFuncs;

struct ktmeDataSource {
    const ktmeDataSourceFuncs *m_funcs;
};

// --- //

typedef struct ktmeAudioDecoderBase ktmeAudioDecoderBase;

typedef struct ktmeAudioDecoderFuncs {
    KTME_AUDIO_SOURCE_BASE_FUNCS;

    ktmeStatus (*linkDataSource)(ktmeAudioDecoderBase *self, ktmeDataSource *dataSource);
} ktmeAudioDecoderFuncs;

struct ktmeAudioDecoderBase {
    const ktmeAudioDecoderFuncs *m_funcs;
};
