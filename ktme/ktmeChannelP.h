#pragma once

#include "ktmeEngine.h"
#include "ktmeChannel.h"
#include "ktmeFrame.h"

void ktmeChannelInit(ktmeChannel *chan);
void ktmeChannelFree(ktmeChannel *chan);

ktmeStatus ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames);
