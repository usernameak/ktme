#pragma once

typedef enum ktmeStatus {
    KTME_STATUS_OK = 0,

    KTME_STATUS_NO_DATA,
    KTME_STATUS_UNSUPPORTED
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
