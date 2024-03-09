#include "ktmeAudioOut.h"

void ktmeAudioOutSetMixer(ktmeAudioOutBase *ao, ktmeMixer *mixer) {
    ao->m_funcs->setMixer(ao, mixer);
}

void ktmeAudioOutSetState(ktmeAudioOutBase *ao, ktmeAudioOutState state) {
    ao->m_funcs->setState(ao, state);
}
