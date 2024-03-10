#include "ktmeChannelP.h"

void ktmeChannelInit(ktmeEngine *engine, ktmeChannel *chan) {
    chan->m_status = KTME_CHANNEL_STATUS_FREE;

    chan->m_volume               = 65535;
    chan->m_panning              = 0;
    chan->m_audioSource          = NULL;
    chan->m_effectiveAudioSource = NULL;
    chan->m_resampler            = NULL;
    chan->m_engine               = engine;
}

void ktmeChannelFree(ktmeChannel *chan) {
    ktmeAudioResamplerDestroy(chan->m_resampler);

    chan->m_audioSource          = NULL;
    chan->m_effectiveAudioSource = NULL;
    chan->m_resampler            = NULL;
    chan->m_status               = KTME_CHANNEL_STATUS_FREE;
}

ktmeStatus ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames) {
    if (chan->m_status == KTME_CHANNEL_STATUS_PAUSED ||
        chan->m_status == KTME_CHANNEL_STATUS_FREE ||
        chan->m_audioSource == NULL) {
        return KTME_STATUS_NO_DATA;
    }

    ktmeStatus status = ktmeAudioSourcePullAudio(chan->m_effectiveAudioSource, numFrames, frames);
    if (status != KTME_STATUS_OK) {
        return status;
    }

    if (chan->m_status == KTME_CHANNEL_STATUS_ACTIVE) {
        // now scale by volume
        for (size_t i = 0; i < numFrames; i++) {
            frames[i].left  = (int32_t)((int64_t)frames[i].left * chan->m_volume / 65535);
            frames[i].right = (int32_t)((int64_t)frames[i].right * chan->m_volume / 65535);
        }

        // TODO: pan
    }

    return KTME_STATUS_OK;
}

ktmeStatus ktmeChannelSetAudioSource(ktmeChannel *chan, ktmeAudioSourceBase *source) {
    if (chan->m_resampler != NULL) {
        ktmeAudioResamplerDestroy(chan->m_resampler);
        chan->m_resampler = NULL;
    }

    // TODO: technically i should reset the source, but... not sure
    chan->m_audioSource = source;
    if (source == NULL) {
        return KTME_STATUS_OK;
    }

    ktmeAudioSourceCaps caps;
    ktmeStatus status = ktmeAudioSourceGetCaps(source, &caps);
    if (status != KTME_STATUS_OK) return status;

    if (caps.numChannels == 2 &&
        caps.sampleRate == 44100 &&
        caps.sampleFormat == KTME_SAMPLE_FORMAT_S32) {

        chan->m_effectiveAudioSource = source;
        return KTME_STATUS_OK;
    }

    chan->m_resampler = ktmeAudioResamplerCreate(chan->m_engine);
    ktmeAudioResamplerSetSource(chan->m_resampler, source);

    chan->m_effectiveAudioSource = (ktmeAudioSourceBase *)chan->m_resampler;

    return KTME_STATUS_OK;
}
