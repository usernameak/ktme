#include "ktmeAudioDecoderWAV.h"

#include "ktmeAdpcmYamaha.h"
#include "ktmeAudioDecoder.h"
#include "ktmeRIFF.h"

#include <assert.h>
#include <string.h>

#define KTME_ADPCM_DECODER_BUFFER_SIZE 512

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

    ktmeAdpcmYamahaState m_adpcmYamahaState;
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

    if (self->m_codec == KTME_WAV_AUDIO_CODEC_PCM) {
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
    } else if (self->m_codec == KTME_WAV_AUDIO_CODEC_YAMAHA_ADPCM) {
        assert(numFrames % 2 == 0); // FIXME: improve in-place decode so that we don't need this

        size_t pcmFrameSize = 2;
        uint8_t *adpcmData  = (uint8_t *)frames +
                             numFrames * pcmFrameSize / 2 +
                             numFrames * pcmFrameSize / 4;
        int16_t *pcmData = (int16_t *)frames;

        uint32_t numRead;
        KTME_CHECK_STATUS(ktmeDataSourceRead(self->m_dataSource, adpcmData, numFrames >> 1, &numRead));

        if (numRead == 0) {
            self->m_status = KTME_WAV_STATUS_OUT_OF_DATA;
            return KTME_STATUS_NO_DATA;
        }

        for (uint32_t i = 0; i < numRead; i++) {
            ktmeAdpcmYamahaDecodeTwoSamples(&self->m_adpcmYamahaState, adpcmData[i], &pcmData[i << 1]);
        }

        uint32_t firstUnfulfilledFrame = numRead * 2;
        if (firstUnfulfilledFrame < numFrames) {
            memset(&((char *)frames)[firstUnfulfilledFrame * pcmFrameSize], 0,
                (numFrames - firstUnfulfilledFrame) * pcmFrameSize);
        }
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

    self->m_uncompressedDataSize = 0;
    self->m_codec                = KTME_WAV_AUDIO_CODEC_INVALID;
    self->m_status               = KTME_WAV_STATUS_READY;
    self->m_dataSource           = dataSource;
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

                if (numChannels == 0 || numChannels > 2 ||
                    blockAlign == 0 || bitsPerSample == 0 ||
                    sampleRate == 0 || byteRate == 0)
                {
                    status = KTME_STATUS_UNSUPPORTED_CODEC;
                    goto fail;
                }

                if (audioFormat == KTME_WAV_AUDIO_CODEC_PCM) {
                    if (bitsPerSample != 8 &&
                        bitsPerSample != 16 &&
                        bitsPerSample != 24 &&
                        bitsPerSample != 32)
                    {
                        status = KTME_STATUS_UNSUPPORTED_CODEC;
                        goto fail;
                    }
                } else if (audioFormat == KTME_WAV_AUDIO_CODEC_YAMAHA_ADPCM) {
                    if (bitsPerSample != 4) {
                        status = KTME_STATUS_UNSUPPORTED_CODEC;
                        goto fail;
                    } else if (numChannels != 1) {
                        // TODO: how is this even supposed to work?
                        status = KTME_STATUS_UNSUPPORTED_CODEC;
                        goto fail;
                    }
                } else {
                    status = KTME_STATUS_UNSUPPORTED_CODEC;
                    goto fail;
                }

                self->m_codec         = audioFormat;
                self->m_numChannels   = numChannels;
                self->m_sampleRate    = sampleRate;
                self->m_byteRate      = byteRate;
                self->m_blockAlign    = blockAlign;
                self->m_bitsPerSample = bitsPerSample;
            } else if (type == 0x74636166) { // 'fact'
                if (size < 4) {
                    status = KTME_STATUS_FORMAT_ERROR;
                    goto fail;
                }

                uint32_t uncompressedDataSize;
                KTME_CHECK_STATUS(ktmeDataSourceReadU32LE(self->m_dataSource, &uncompressedDataSize));
                if (size > 4) {
                    KTME_CHECK_STATUS(ktmeDataSourceSkip(self->m_dataSource, size - 4));
                }

                self->m_uncompressedDataSize = uncompressedDataSize;
            } else if (type == 0x61746164) { // 'data'
                // check if we have a valid codec
                // (otherwise, it might be that fmt is missing)
                if (self->m_codec == KTME_WAV_AUDIO_CODEC_INVALID) {
                    status = KTME_STATUS_FORMAT_ERROR;
                    goto fail;
                }

                if (self->m_codec == KTME_WAV_AUDIO_CODEC_YAMAHA_ADPCM) {
                    // initialize the ADPCM decoder state
                    ktmeAdpcmYamahaInitState(&self->m_adpcmYamahaState);
                }

                // save the offset and size
                self->m_dataOffset = ktmeDataSourceTell(self->m_dataSource);
                self->m_dataSize   = size;
                if (self->m_codec == KTME_WAV_AUDIO_CODEC_PCM || self->m_uncompressedDataSize == 0) {
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

    caps->numChannels = self->m_numChannels;
    caps->sampleRate  = self->m_sampleRate;
    if (self->m_codec == KTME_WAV_AUDIO_CODEC_PCM) {
        switch (self->m_bitsPerSample) {
        case 8: caps->sampleFormat = KTME_SAMPLE_FORMAT_U8; break;
        case 16: caps->sampleFormat = KTME_SAMPLE_FORMAT_S16; break;
        case 24: caps->sampleFormat = KTME_SAMPLE_FORMAT_S24; break;
        case 32: caps->sampleFormat = KTME_SAMPLE_FORMAT_S32; break;
        default: return KTME_STATUS_UNSUPPORTED_CODEC; // nope.
        }
        return KTME_STATUS_OK;
    }
    if (self->m_codec == KTME_WAV_AUDIO_CODEC_YAMAHA_ADPCM) {
        caps->sampleFormat = KTME_SAMPLE_FORMAT_S16;
        return KTME_STATUS_OK;
    }

    return KTME_STATUS_NOT_READY;
}

static const ktmeAudioDecoderFuncs g_audioDecoderWAVFuncs = {
    .pullAudio      = ktmeAudioDecoderWAVPullAudio,
    .seek           = ktmeAudioDecoderWAVSeek,
    .stop           = ktmeAudioDecoderWAVStop,
    .linkDataSource = ktmeAudioDecoderWAVLinkDataSource,
    .getCaps        = ktmeAudioDecoderWAVGetCaps
};
