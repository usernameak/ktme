#pragma once

#include "ktmeCommon.h"
#include "ktmeChannel.h"
#include "ktmeFrame.h"

void ktmeChannelInit(ktmeEngine *engine, ktmeChannel *chan);
void ktmeChannelFree(ktmeChannel *chan);

ktmeStatus ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames);
