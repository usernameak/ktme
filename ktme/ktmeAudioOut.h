#pragma once

#include "ktmeEngine.h"

typedef struct ktmeMixer ktmeMixer;
typedef struct ktmeAudioOutBase ktmeAudioOutBase;

typedef enum ktmeAudioOutState {
    KTME_AUDIO_OUT_STATE_ACTIVE,
    KTME_AUDIO_OUT_STATE_INACTIVE
} ktmeAudioOutState;

typedef struct ktmeAudioOutFuncs {
    void (*setMixer)(ktmeAudioOutBase *baseao, ktmeMixer *mixer);
    void (*setState)(ktmeAudioOutBase *baseao, ktmeAudioOutState state);
} ktmeAudioOutFuncs;

#define KTME_AUDIO_OUT_BASE_FIELDS    \
    const ktmeAudioOutFuncs *m_funcs; \
    ktmeEngine *m_engine

struct ktmeAudioOutBase {
    KTME_AUDIO_OUT_BASE_FIELDS;
};

void ktmeAudioOutSetMixer(ktmeAudioOutBase *ao, ktmeMixer *mixer);
void ktmeAudioOutSetState(ktmeAudioOutBase *ao, ktmeAudioOutState state);
