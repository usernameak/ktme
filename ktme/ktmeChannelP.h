#pragma once

#include "ktmeChannel.h"
#include "ktmeFrame.h"

void ktmeChannelInit(ktmeChannel *chan);
void ktmeChannelFree(ktmeChannel *chan);

void ktmeChannelPullAudioData(ktmeChannel *chan, size_t numFrames, ktmeFrameS32 *frames);
