#include "ktmeAudioSource.h"

ktmeStatus ktmeAudioSourceUnsupportedSeek(ktmeAudioSourceBase *src, ktmeSeekWhence whence, int32_t timeMS) {
    return KTME_STATUS_UNSUPPORTED;
}

ktmeStatus ktmeAudioSourceUnsupportedStop(ktmeAudioSourceBase *src) {
    return KTME_STATUS_UNSUPPORTED;
}
