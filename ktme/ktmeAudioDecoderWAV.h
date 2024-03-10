#pragma once

#include "ktmeEngine.h"

typedef struct ktmeAudioDecoderWAV ktmeAudioDecoderWAV;

ktmeAudioDecoderWAV *ktmeAudioDecoderWAVCreate(ktmeEngine *engine);
void ktmeAudioDecoderWAVDestroy(ktmeAudioDecoderWAV *codec);
