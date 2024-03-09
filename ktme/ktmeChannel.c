#include "ktmeChannelP.h"

void ktmeChannelInit(ktmeChannel *chan) {
    chan->m_status = KTME_CHANNEL_STATUS_FREE;

    chan->m_volume      = 65535;
    chan->m_panning     = 0;
    chan->m_audioSource = NULL;
}

void ktmeChannelFree(ktmeChannel *chan) {
    chan->m_audioSource = NULL;
    chan->m_status      = KTME_CHANNEL_STATUS_FREE;
}

ktmeStatus ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames) {
    if (chan->m_status == KTME_CHANNEL_STATUS_PAUSED ||
        chan->m_status == KTME_CHANNEL_STATUS_FREE ||
        chan->m_audioSource == NULL) {
        return KTME_STATUS_NO_DATA;
    }

    ktmeStatus status = ktmeAudioSourcePullAudio(chan->m_audioSource, numFrames, frames);
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

void ktmeChannelSetAudioSource(ktmeChannel *chan, ktmeAudioSourceBase *source) {
    // TODO: technically we should reset the source, but... not sure
    chan->m_audioSource = source;
}
