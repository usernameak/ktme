#include "ktmeAudioResampler.h"

#include <string.h>

static const ktmeAudioSourceFuncs g_audioResamplerFuncs;

ktmeAudioResampler *ktmeAudioResamplerCreate(ktmeEngine *engine) {
    ktmeAudioResampler *resampler = ktmeEngineMemAlloc(engine, sizeof(ktmeAudioResampler));
    memset(resampler, 0, sizeof(ktmeAudioResampler));
    resampler->m_funcs  = &g_audioResamplerFuncs;
    resampler->m_engine = engine;
    return resampler;
}

void ktmeAudioResamplerDestroy(ktmeAudioResampler *resampler) {
    ktmeEngineMemFree(resampler->m_engine, resampler);
}

ktmeStatus ktmeAudioResamplerPullAudio(ktmeAudioSourceBase *src, size_t numFrames, void *frames_) {
    ktmeAudioResampler *self = (ktmeAudioResampler *)src;

    if (self->m_source == NULL) {
        return KTME_STATUS_NO_DATA;
    }

    ktmeAudioSourceCaps caps;
    ktmeStatus status = ktmeAudioSourceGetCaps(self->m_source, &caps);
    if (status != KTME_STATUS_OK) {
        return status;
    }

    uint32_t framesPerBuffer = sizeof(self->m_buffer) / ktmeFrameSize(caps.sampleFormat, caps.numChannels);

    ktmeFrameS32 *framesOut  = frames_;
    uint32_t currentPosition = 0, framesRemaining = numFrames;
    while (framesRemaining > 0) {
        uint32_t framesToPull = framesRemaining > framesPerBuffer ? framesPerBuffer : framesRemaining;

        status = ktmeAudioSourcePullAudio(self->m_source, framesToPull, self->m_buffer);
        if (status == KTME_STATUS_NO_DATA) {
            if (currentPosition == 0) return KTME_STATUS_NO_DATA;

            memset(&framesOut[currentPosition], 0, framesRemaining * sizeof(ktmeFrameS32));
            goto finished;
        }
        if (status != KTME_STATUS_OK) return status;

        for (size_t i = 0; i < framesToPull; i++) {
            uint32_t k = i + currentPosition;

            if (caps.numChannels == 1) {
                if (caps.sampleFormat == KTME_SAMPLE_FORMAT_U8) {
                    framesOut[k].left  = (int32_t)(int8_t)(uint8_t)(self->m_buffer[i] + 128u) << 24;
                    framesOut[k].right = (int32_t)(int8_t)(uint8_t)(self->m_buffer[i] + 128u) << 24;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S16) {
                    framesOut[k].left = (int32_t)(int16_t)(self->m_buffer[i * 2] |
                                                           self->m_buffer[i * 2 + 1] << 8u)
                                     << 16;
                    framesOut[k].right = (int32_t)(int16_t)(self->m_buffer[i * 2] |
                                                            self->m_buffer[i * 2 + 1] << 8u)
                                      << 16;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S24) {
                    framesOut[k].left = (int32_t)(int16_t)(self->m_buffer[i * 3] |
                                                           self->m_buffer[i * 3 + 1] << 8u |
                                                           self->m_buffer[i * 3 + 2] << 16u)
                                     << 8;
                    framesOut[k].right = (int32_t)(int16_t)(self->m_buffer[i * 3] |
                                                            self->m_buffer[i * 3 + 1] << 8u |
                                                            self->m_buffer[i * 3 + 2] << 16u)
                                      << 8;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S32) {
                    framesOut[k].left = self->m_buffer[i * 4] |
                                        self->m_buffer[i * 4 + 1] << 8u |
                                        self->m_buffer[i * 4 + 2] << 16u |
                                        self->m_buffer[i * 4 + 3] << 24u;
                    framesOut[k].right = self->m_buffer[i * 4] |
                                         self->m_buffer[i * 4 + 1] << 8u |
                                         self->m_buffer[i * 4 + 2] << 16u |
                                         self->m_buffer[i * 4 + 3] << 24u;
                } else {
                    return KTME_STATUS_UNSUPPORTED_CODEC;
                }
            } else if (caps.numChannels == 2) {
                if (caps.sampleFormat == KTME_SAMPLE_FORMAT_U8) {
                    framesOut[k].left  = (int32_t)(int8_t)(uint8_t)(self->m_buffer[i * 2] + 128u) << 24;
                    framesOut[k].right = (int32_t)(int8_t)(uint8_t)(self->m_buffer[i * 2 + 1] + 128u) << 24;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S16) {
                    framesOut[k].left = (int32_t)(int16_t)(self->m_buffer[i * 4] |
                                                           self->m_buffer[i * 4 + 1] << 8u)
                                     << 16;
                    framesOut[k].right = (int32_t)(int16_t)(self->m_buffer[i * 4 + 2] |
                                                            self->m_buffer[i * 4 + 3] << 8u)
                                      << 16;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S24) {
                    framesOut[k].left = (int32_t)(int16_t)(self->m_buffer[i * 6] |
                                                           self->m_buffer[i * 6 + 1] << 8u |
                                                           self->m_buffer[i * 6 + 2] << 16u)
                                     << 8;
                    framesOut[k].right = (int32_t)(int16_t)(self->m_buffer[i * 6 + 3] |
                                                            self->m_buffer[i * 6 + 4] << 8u |
                                                            self->m_buffer[i * 6 + 5] << 16u)
                                      << 8;
                } else if (caps.sampleFormat == KTME_SAMPLE_FORMAT_S32) {
                    framesOut[k].left = self->m_buffer[i * 8] |
                                        self->m_buffer[i * 8 + 1] << 8u |
                                        self->m_buffer[i * 8 + 2] << 16u |
                                        self->m_buffer[i * 8 + 3] << 24u;
                    framesOut[k].right = self->m_buffer[i * 8 + 4] |
                                         self->m_buffer[i * 8 + 5] << 8u |
                                         self->m_buffer[i * 8 + 6] << 16u |
                                         self->m_buffer[i * 8 + 7] << 24u;
                } else {
                    return KTME_STATUS_UNSUPPORTED_CODEC;
                }
            } else {
                return KTME_STATUS_UNSUPPORTED_CODEC;
            }
        }

        currentPosition += framesToPull;
        framesRemaining -= framesToPull;
    }

finished:
    return KTME_STATUS_OK;
}

ktmeStatus ktmeAudioResamplerGetCaps(ktmeAudioSourceBase *src, ktmeAudioSourceCaps *caps) {
    caps->numChannels  = 2;
    caps->sampleRate   = 44100;
    caps->sampleFormat = KTME_SAMPLE_FORMAT_S32;

    return KTME_STATUS_OK;
}

void ktmeAudioResamplerSetSource(ktmeAudioResampler *resampler, ktmeAudioSourceBase *source) {
    resampler->m_source = source;
}

static const ktmeAudioSourceFuncs g_audioResamplerFuncs = {
    .pullAudio = ktmeAudioResamplerPullAudio,
    .seek      = ktmeAudioSourceUnsupportedSeek,
    .stop      = ktmeAudioSourceUnsupportedStop,
    .getCaps   = ktmeAudioResamplerGetCaps
};
