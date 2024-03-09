#pragma once

#include "ktmeEngine.h"
#include "ktmeFrame.h"
#include "ktmeAudioOut.h"

#include <stdint.h>

typedef struct ktmeChannel ktmeChannel;

typedef struct ktmeMixer {
    ktmeEngine *m_engine;

    ktmeAudioOutBase *m_ao;

    uint32_t m_numChannels;
    ktmeChannel *m_channels;
    uint32_t m_numActiveChannels;

    size_t m_mixBufferSize;
    ktmeFrameS32 *m_mixBuffer;
} ktmeMixer;

ktmeMixer *ktmeMixerCreate(ktmeEngine *engine, uint32_t numChannels);
void ktmeMixerDestroy(ktmeMixer *mixer);

ktmeChannel *ktmeMixerChannelAlloc(ktmeMixer *mixer);
void ktmeMixerChannelFree(ktmeMixer *mixer, ktmeChannel *chan);

void ktmeMixerChannelPlay(ktmeMixer *mixer, ktmeChannel *chan);
void ktmeMixerChannelPause(ktmeMixer *mixer, ktmeChannel *chan);
void ktmeMixerChannelStop(ktmeMixer *mixer, ktmeChannel *chan);

void ktmeMixerLinkAudioOutput(ktmeMixer *mixer, ktmeAudioOutBase *ao);
void ktmeMixerMixPull(ktmeMixer *mixer, size_t numFrames, ktmeFrameS32 *frames);
