#include "ktmeChannelP.h"

#include <string.h>

void ktmeChannelInit(ktmeChannel *chan) {
    chan->m_status = KTME_CHANNEL_STATUS_FREE;

    chan->m_volume  = 65535;
    chan->m_panning = 0;
}

void ktmeChannelFree(ktmeChannel *chan) {
    chan->m_status = KTME_CHANNEL_STATUS_FREE;
}

void ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames) {
    // TODO: actually pull data instead of generating noise

    if (chan->m_status == KTME_CHANNEL_STATUS_PAUSED ||
        chan->m_status == KTME_CHANNEL_STATUS_FREE) {

        memset(frames, 0, sizeof(ktmeFrameS32) * numFrames);
        return; // TODO: maybe skip mixing?
    }

    uint32_t x = 1;
    for (size_t i = 0; i < numFrames; i++) {
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;

        if (chan->m_status == KTME_CHANNEL_STATUS_ACTIVE) {
            frames[i].left  = x;
            frames[i].right = x;
        }
    }

    if (chan->m_status == KTME_CHANNEL_STATUS_ACTIVE) {
        // now scale by volume
        for (size_t i = 0; i < numFrames; i++) {
            frames[i].left  = (int32_t)((int64_t)frames[i].left * chan->m_volume / 65535);
            frames[i].right = (int32_t)((int64_t)frames[i].right * chan->m_volume / 65535);
        }

        // TODO: pan
    }
}
