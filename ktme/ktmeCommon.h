#pragma once

typedef enum ktmeStatus {
    KTME_STATUS_OK = 0,

    KTME_STATUS_NO_DATA, // TODO: split into NO_DATA for mixer audio and for IO
    KTME_STATUS_UNSUPPORTED,
    KTME_STATUS_FORMAT_ERROR,
    KTME_STATUS_UNSUPPORTED_CODEC
} ktmeStatus;

typedef enum ktmeSeekWhence {
    KTME_SEEK_START = 0,
    KTME_SEEK_END,
    KTME_SEEK_CUR
} ktmeSeekWhence;

#ifdef _MSC_VER
#define KTME_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define KTME_FORCEINLINE __attribute__((always_inline))
#else
#define KTME_FORCEINLINE inline
#endif
