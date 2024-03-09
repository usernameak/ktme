#include "ktmeMixer.h"

#include "ktmeChannel.h"
#include "ktmeChannelP.h"

#include <assert.h>
#include <string.h>

ktmeMixer *ktmeMixerCreate(ktmeEngine *engine, uint32_t numChannels) {
    ktmeMixer *mixer = ktmeEngineMemAlloc(engine, sizeof(ktmeMixer));

    mixer->m_engine = engine;
    mixer->m_ao     = NULL;

    mixer->m_numChannels       = numChannels;
    mixer->m_numActiveChannels = 0;
    mixer->m_channels          = ktmeEngineMemAlloc(engine, sizeof(ktmeChannel) * mixer->m_numChannels);
    for (uint32_t i = 0; i < mixer->m_numChannels; i++) {
        ktmeChannelInit(&mixer->m_channels[i]);
    }

    mixer->m_mixBufferSize = 1024;
    mixer->m_mixBuffer     = ktmeEngineMemAlloc(engine, sizeof(ktmeFrameS32) * mixer->m_mixBufferSize);

    return mixer;
}

void ktmeMixerDestroy(ktmeMixer *mixer) {
    ktmeMixerLinkAudioOutput(mixer, NULL);

    ktmeEngineMemFree(mixer->m_engine, mixer->m_mixBuffer);
    ktmeEngineMemFree(mixer->m_engine, mixer->m_channels);

    ktmeEngineMemFree(mixer->m_engine, mixer);
}

ktmeChannel *ktmeMixerChannelAlloc(ktmeMixer *mixer) {
    for (uint32_t i = 0; i < mixer->m_numChannels; i++) {
        ktmeChannel *chan = &mixer->m_channels[i];

        if (chan->m_status == KTME_CHANNEL_STATUS_FREE) {
            chan->m_status = KTME_CHANNEL_STATUS_PAUSED;
            return chan;
        }
    }

    return NULL;
}

void ktmeMixerChannelFree(ktmeMixer *mixer, ktmeChannel *chan) {
    ktmeMixerChannelPause(mixer, chan);
    ktmeChannelFree(chan);
}

void ktmeMixerChannelPlay(ktmeMixer *mixer, ktmeChannel *chan) {
    if (chan->m_status == KTME_CHANNEL_STATUS_ACTIVE) {
        return;
    }

    assert(chan->m_status != KTME_CHANNEL_STATUS_FREE);
    chan->m_status = KTME_CHANNEL_STATUS_ACTIVE;

    if (mixer->m_numActiveChannels == 0) {
        ktmeAudioOutSetState(mixer->m_ao, KTME_AUDIO_OUT_STATE_ACTIVE);
    }
    mixer->m_numActiveChannels++;
}

void ktmeMixerChannelPause(ktmeMixer *mixer, ktmeChannel *chan) {
    if (chan->m_status == KTME_CHANNEL_STATUS_PAUSED) {
        return;
    }

    assert(chan->m_status != KTME_CHANNEL_STATUS_FREE);
    chan->m_status = KTME_CHANNEL_STATUS_PAUSED;

    assert(mixer->m_numActiveChannels != 0);
    mixer->m_numActiveChannels--;
    if (mixer->m_numActiveChannels == 0) {
        ktmeAudioOutSetState(mixer->m_ao, KTME_AUDIO_OUT_STATE_INACTIVE);
    }
}

void ktmeMixerChannelStop(ktmeMixer *mixer, ktmeChannel *chan) {
    ktmeMixerChannelPause(mixer, chan);
}

void ktmeMixerLinkAudioOutput(ktmeMixer *mixer, ktmeAudioOutBase *ao) {
    if (mixer->m_ao) {
        ktmeAudioOutSetMixer(mixer->m_ao, NULL);
    }

    mixer->m_ao = ao;
    if (ao) {
        ktmeAudioOutSetMixer(ao, mixer);
    }
}

KTME_FORCEINLINE static int32_t ktmeAddAndClampS32(int32_t a, int32_t b) {
    if (b >= 0) {
        return (a <= INT32_MAX - b) ? a + b : INT32_MAX;
    }
    return (a >= INT32_MIN - b) ? a + b : INT32_MIN;
}

void ktmeMixerMixPull(ktmeMixer *mixer, size_t numFrames, ktmeFrameS32 *frames) {
    memset(frames, 0, sizeof(ktmeFrameS32) * numFrames);

    for (uint32_t i = 0; i < mixer->m_numChannels; i++) {
        ktmeChannel *chan = &mixer->m_channels[i];

        size_t currentOffset = 0;
        size_t framesLeft    = numFrames;
        while (framesLeft != 0) {
            size_t framesProcessed = framesLeft > mixer->m_mixBufferSize ? mixer->m_mixBufferSize : framesLeft;

            ktmeStatus status = ktmeChannelPullAudioData(chan, framesProcessed, mixer->m_mixBuffer);

            // KTME_STATUS_NO_DATA is technically not a failure,
            // but it aborts mixing for this channel
            if (status != KTME_STATUS_OK) {
                break;
            }

            for (size_t j = 0; j < framesProcessed; j++) {
                frames[j + currentOffset].left =
                    ktmeAddAndClampS32(frames[j + currentOffset].left, mixer->m_mixBuffer[j].left);
                frames[j + currentOffset].right =
                    ktmeAddAndClampS32(frames[j + currentOffset].right, mixer->m_mixBuffer[j].right);
            }

            framesLeft -= framesProcessed;
            currentOffset += framesProcessed;
        }
    }
}
