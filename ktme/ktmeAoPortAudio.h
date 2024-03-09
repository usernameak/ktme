#pragma once

#include "ktmeAudioOut.h"

typedef void PaStream;

typedef struct ktmeAudioOutPortAudio {
    KTME_AUDIO_OUT_BASE_FIELDS;

    ktmeMixer *m_mixer;
    ktmeAudioOutState m_state;
    PaStream *m_stream;
} ktmeAudioOutPortAudio;

ktmeAudioOutPortAudio *ktmeAudioOutPortAudioCreate(ktmeEngine *engine);
void ktmeAudioOutPortAudioDestroy(ktmeAudioOutPortAudio *ao);
