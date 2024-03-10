#pragma once

typedef enum ktmeStatus {
    KTME_STATUS_OK = 0,

    KTME_STATUS_INVALID_ARGUMENT,
    KTME_STATUS_NO_DATA, // TODO: split into NO_DATA for mixer audio and for IO
    KTME_STATUS_UNSUPPORTED,
    KTME_STATUS_FORMAT_ERROR,
    KTME_STATUS_UNSUPPORTED_CODEC,
    KTME_STATUS_UNSUPPORTED_SAMPLE_FORMAT,
    KTME_STATUS_NOT_READY,

    // TODO: supersede with more specific codes
    KTME_STATUS_FOPEN_FAILED,
    KTME_STATUS_FREAD_FAILED,
    KTME_STATUS_FSEEK_FAILED,
} ktmeStatus;

typedef enum ktmeSeekWhence {
    KTME_SEEK_START = 0,
    KTME_SEEK_END,
    KTME_SEEK_CUR
} ktmeSeekWhence;

#if defined(_MSC_VER) || defined(__ARMCC_VERSION)
#define KTME_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define KTME_FORCEINLINE __attribute__((always_inline))
#else
#define KTME_FORCEINLINE inline
#endif

// shittiest macro in the world - worse than what i had in GroveEngine2
#define KTME_CHECK_STATUS(...)                   \
    do {                                         \
        status = (__VA_ARGS__);                  \
        if (status != KTME_STATUS_OK) goto fail; \
    } while (0)
