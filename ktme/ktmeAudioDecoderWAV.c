#include "ktmeAudioDecoderWAV.h"

#include "ktmeAudioDecoder.h"
#include "ktmeRIFF.h"

#include <string.h>

typedef enum ktmeWAVAudioCodec {
    KTME_WAV_AUDIO_CODEC_INVALID      = 0x0000,
    KTME_WAV_AUDIO_CODEC_PCM          = 0x0001,
    KTME_WAV_AUDIO_CODEC_YAMAHA_ADPCM = 0x0020,
} ktmeWAVAudioCodec;

typedef enum ktmeWAVStatus {
    KTME_WAV_STATUS_READY,
    KTME_WAV_STATUS_OUT_OF_DATA
} ktmeWAVStatus;

struct ktmeAudioDecoderWAV {
    const ktmeAudioDecoderFuncs *m_funcs;

    ktmeEngine *m_engine;
    ktmeDataSource *m_dataSource;

    ktmeWAVStatus m_status;

    ktmeWAVAudioCodec m_codec;
    uint16_t m_numChannels;
    uint32_t m_sampleRate;
    uint32_t m_byteRate;
    uint16_t m_blockAlign;
    uint16_t m_bitsPerSample;

    uint32_t m_dataOffset;
    uint32_t m_dataSize;
    uint32_t m_uncompressedDataSize;
};

static const ktmeAudioDecoderFuncs g_audioDecoderWAVFuncs;

ktmeAudioDecoderWAV *ktmeAudioDecoderWAVCreate(ktmeEngine *engine) {
    ktmeAudioDecoderWAV *codec = ktmeEngineMemAlloc(engine, sizeof(ktmeAudioDecoderWAV));
    memset(codec, 0, sizeof(ktmeAudioDecoderWAV));
    codec->m_funcs  = &g_audioDecoderWAVFuncs;
    codec->m_engine = engine;
    return codec;
}

void ktmeAudioDecoderWAVDestroy(ktmeAudioDecoderWAV *codec) {
    if (codec->m_dataSource) {
        ktmeDataSourceRelease(codec->m_dataSource);
    }

    ktmeEngineMemFree(codec->m_engine, codec);
}

static ktmeStatus ktmeAudioDecoderWAVSeek(ktmeAudioSourceBase *src, ktmeSeekWhence whence, int32_t timeMS) {
    return KTME_STATUS_UNSUPPORTED;
}

static ktmeStatus ktmeAudioDecoderWAVStop(ktmeAudioSourceBase *src) {
    return KTME_STATUS_UNSUPPORTED;
}

// shittiest macro in the world - worse than what i had in GroveEngine2
#define KTME_CHECK_STATUS(...)                   \
    do {                                         \
        status = (__VA_ARGS__);                  \
        if (status != KTME_STATUS_OK) goto fail; \
    } while (0)

static ktmeStatus ktmeAudioDecoderWAVPullAudio(ktmeAudioSourceBase *src, size_t numFrames, void *frames) {
    ktmeAudioDecoderWAV *self = (ktmeAudioDecoderWAV *)src;

    if (self->m_dataSource == NULL) {
        return KTME_STATUS_NO_DATA;
    }
    if (self->m_codec == KTME_WAV_AUDIO_CODEC_INVALID) {
        return KTME_STATUS_NO_DATA;
    }
    if (self->m_status == KTME_WAV_STATUS_OUT_OF_DATA) {
        return KTME_STATUS_NO_DATA;
    }

    ktmeStatus status;

    size_t frameSize = self->m_numChannels * self->m_bitsPerSample / 8;

    uint32_t numRead;
    KTME_CHECK_STATUS(ktmeDataSourceRead(self->m_dataSource, frames, numFrames * frameSize, &numRead));

    if (numRead == 0) {
        self->m_status = KTME_WAV_STATUS_OUT_OF_DATA;
        return KTME_STATUS_NO_DATA;
    }

    uint32_t firstUnfulfilledFrame = numRead / frameSize;
    if (firstUnfulfilledFrame < numFrames) {
        memset(&((char *)frames)[firstUnfulfilledFrame * frameSize], 0, (numFrames - firstUnfulfilledFrame) * frameSize);
    }

    return KTME_STATUS_OK;

fail:
    return status;
}

static ktmeStatus ktmeAudioDecoderWAVLinkDataSource(ktmeAudioDecoderBase *dec, ktmeDataSource *dataSource) {
    ktmeStatus status;

    ktmeAudioDecoderWAV *self = (ktmeAudioDecoderWAV *)dec;

    if (self->m_dataSource) {
        ktmeDataSourceRelease(self->m_dataSource);
    }

    self->m_codec      = KTME_WAV_AUDIO_CODEC_INVALID;
    self->m_status     = KTME_WAV_STATUS_READY;
    self->m_dataSource = dataSource;
    if (self->m_dataSource) {
        ktmeDataSourceAddRef(self->m_dataSource);

        // read in the RIFF header...
        uint32_t riffHeader, riffSize, riffFormat;
        KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &riffHeader));
        KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &riffSize));
        KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &riffFormat));

        if (riffHeader != 0x46464952u || riffFormat != 0x45564157u) { // 'RIFF', 'WAVE'
            status = KTME_STATUS_FORMAT_ERROR;
            goto fail;
        }

        // now read the actual subchunks...
        while (1) {
            uint32_t type, size;
            KTME_CHECK_STATUS(ktmeRIFFReadSubchunkHeader(self->m_dataSource, &type, &size));

            if (type == 0x20746D66) { // 'fmt '
                if (size < 16) {
                    status = KTME_STATUS_FORMAT_ERROR;
                    goto fail;
                }

                uint16_t audioFormat, numChannels;
                uint32_t sampleRate, byteRate;
                uint16_t blockAlign; // what is this?
                uint16_t bitsPerSample;

                KTME_CHECK_STATUS(ktmeDataSourceReadU16LE(self->m_dataSource, &audioFormat));
                KTME_CHECK_STATUS(ktmeDataSourceReadU16LE(self->m_dataSource, &numChannels));
                KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &sampleRate));
                KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &byteRate));
                KTME_CHECK_STATUS(ktmeDataSourceReadU16LE(self->m_dataSource, &blockAlign));
                KTME_CHECK_STATUS(ktmeDataSourceReadU16LE(self->m_dataSource, &bitsPerSample));

                if (size > 16) {
                    KTME_CHECK_STATUS(ktmeDataSourceSkip(self->m_dataSource, size - 16));
                }

                if (audioFormat != KTME_WAV_AUDIO_CODEC_PCM ||
                    numChannels == 0 || numChannels > 2 ||
                    sampleRate == 0 || byteRate == 0 ||
                    blockAlign == 0 ||
                    (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)) {

                    status = KTME_STATUS_UNSUPPORTED_CODEC;
                    goto fail;
                }

                self->m_codec         = audioFormat;
                self->m_numChannels   = numChannels;
                self->m_sampleRate    = sampleRate;
                self->m_byteRate      = byteRate;
                self->m_blockAlign    = blockAlign;
                self->m_bitsPerSample = bitsPerSample;
            } else if (type == 0x61746164) { // 'data'
                // check if we have a valid codec
                // (otherwise, it might be that fmt is missing)
                if (self->m_codec == KTME_WAV_AUDIO_CODEC_INVALID) {
                    status = KTME_STATUS_FORMAT_ERROR;
                    goto fail;
                }

                // save the offset and size
                self->m_dataOffset = ktmeDataSourceTell(self->m_dataSource);
                self->m_dataSize   = size;
                if (self->m_codec == KTME_WAV_AUDIO_CODEC_PCM) {
                    // PCM doesn't have 'fact' record
                    self->m_uncompressedDataSize = size;
                }

                // at this point we found the data, so bail out

                // ... or maybe not - i've seen some WAV files that
                // had more data in the end? but that requires random
                // access, which is not guaranteed due to streaming
                // concerns ...
                break;
            } else {
                // unknown chunk!
                ktmeDataSourceSkip(self->m_dataSource, size);
            }
        }
    }

    return KTME_STATUS_OK;

fail:
    return status;
}

ktmeStatus ktmeAudioDecoderWAVGetCaps(ktmeAudioSourceBase *src, ktmeAudioSourceCaps *caps) {
    ktmeAudioDecoderWAV *self = (ktmeAudioDecoderWAV *)src;

    if (self->m_codec == KTME_WAV_AUDIO_CODEC_INVALID) {
        return KTME_STATUS_NOT_READY;
    }

    caps->numChannels = self->m_numChannels;
    caps->sampleRate  = self->m_sampleRate;
    switch (self->m_bitsPerSample) {
    case 8: caps->sampleFormat = KTME_SAMPLE_FORMAT_U8; break;
    case 16: caps->sampleFormat = KTME_SAMPLE_FORMAT_S16; break;
    case 24: caps->sampleFormat = KTME_SAMPLE_FORMAT_S24; break;
    case 32: caps->sampleFormat = KTME_SAMPLE_FORMAT_S32; break;
    default: return KTME_STATUS_UNSUPPORTED_CODEC; // nope.
    }

    return KTME_STATUS_OK;
}

static const ktmeAudioDecoderFuncs g_audioDecoderWAVFuncs = {
    .pullAudio      = ktmeAudioDecoderWAVPullAudio,
    .seek           = ktmeAudioDecoderWAVSeek,
    .stop           = ktmeAudioDecoderWAVStop,
    .linkDataSource = ktmeAudioDecoderWAVLinkDataSource,
    .getCaps        = ktmeAudioDecoderWAVGetCaps
};
