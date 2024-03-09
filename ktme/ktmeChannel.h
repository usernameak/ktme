#pragma once

#include "ktmeAudioSource.h"

typedef enum ktmeChannelStatus {
    KTME_CHANNEL_STATUS_FREE,
    KTME_CHANNEL_STATUS_ACTIVE,
    KTME_CHANNEL_STATUS_PAUSED
} ktmeChannelStatus;

typedef struct ktmeChannel {
    ktmeChannelStatus m_status;

    uint16_t m_volume;
    int16_t m_panning;

    ktmeAudioSourceBase *m_audioSource;
} ktmeChannel;

void ktmeChannelSetAudioSource(ktmeChannel *chan, ktmeAudioSourceBase *source);
