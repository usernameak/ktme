#pragma once

#include "ktmeAudioResampler.h"
#include "ktmeAudioSource.h"
#include "ktmeEngine.h"

typedef enum ktmeChannelStatus {
    KTME_CHANNEL_STATUS_FREE,
    KTME_CHANNEL_STATUS_ACTIVE,
    KTME_CHANNEL_STATUS_PAUSED
} ktmeChannelStatus;

typedef struct ktmeChannel {
    ktmeChannelStatus m_status;

    ktmeEngine *m_engine;

    uint16_t m_volume;
    int16_t m_panning;

    ktmeAudioSourceBase *m_audioSource;
    ktmeAudioSourceBase *m_effectiveAudioSource;
    ktmeAudioResampler *m_resampler;
} ktmeChannel;

ktmeStatus ktmeChannelSetAudioSource(ktmeChannel *chan, ktmeAudioSourceBase *source);
