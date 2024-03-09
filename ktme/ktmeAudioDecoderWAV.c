#include "ktmeAudioDecoderWAV.h"

#include "ktmeAudioDecoder.h"

typedef struct ktmeAudioDecoderWAV {
    const ktmeAudioDecoderFuncs *m_funcs;

    ktmeDataSource *m_dataSource;
} ktmeAudioDecoderWAV;

static const ktmeAudioDecoderFuncs g_audioDecoderWAVFuncs;

static ktmeStatus ktmeAudioDecoderWAVSeek(ktmeAudioSourceBase *src, ktmeSeekWhence whence, int32_t timeMS) {
    return KTME_STATUS_UNSUPPORTED;
}

static ktmeStatus ktmeAudioDecoderWAVStop(ktmeAudioSourceBase *src) {
    return KTME_STATUS_UNSUPPORTED;
}

static ktmeStatus ktmeAudioDecoderWAVPullAudio(ktmeAudioSourceBase *src, size_t numFrames, ktmeFrameS32 *frames) {
    ktmeAudioDecoderWAV *self = (ktmeAudioDecoderWAV *)src;

    if (self->m_dataSource) {
        return KTME_STATUS_NO_DATA;
    }

    return KTME_STATUS_NO_DATA; // TODO:
}

static ktmeStatus ktmeAudioDecoderWAVLinkDataSource(ktmeAudioDecoderBase *dec, ktmeDataSource *dataSource) {
    ktmeStatus status;

    ktmeAudioDecoderWAV *self = (ktmeAudioDecoderWAV *)dec;

    if (self->m_dataSource) {
        ktmeDataSourceRelease(self->m_dataSource);
    }

    self->m_dataSource = dataSource;
    if (self->m_dataSource) {
        ktmeDataSourceAddRef(self->m_dataSource);

        // read in the RIFF header...
        uint32_t riffHeader;
        status = ktmeDataSourceReadU32LE(self->m_dataSource, &riffHeader);
        if (status != KTME_STATUS_OK) goto fail;
        if (riffHeader != 0x46464952u) { // 'RIFF'
            status = KTME_STATUS_FORMAT_ERROR;
            goto fail;
        }

        uint32_t riffSize;
        status = ktmeDataSourceReadU32LE(self->m_dataSource, &riffSize);
        if (status != KTME_STATUS_OK) goto fail;

        uint32_t riffFormat;
        status = ktmeDataSourceReadU32LE(self->m_dataSource, &riffFormat);
        if (status != KTME_STATUS_OK) goto fail;
        if (riffFormat != 0x45564157u) { // 'WAVE'
            status = KTME_STATUS_FORMAT_ERROR;
            goto fail;
        }
    }

    return KTME_STATUS_OK;

fail:
    return status;
}

static const ktmeAudioDecoderFuncs g_audioDecoderWAVFuncs = {
    .pullAudio      = ktmeAudioDecoderWAVPullAudio,
    .seek           = ktmeAudioDecoderWAVSeek,
    .stop           = ktmeAudioDecoderWAVStop,
    .linkDataSource = ktmeAudioDecoderWAVLinkDataSource
};