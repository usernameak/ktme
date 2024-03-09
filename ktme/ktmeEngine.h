#pragma once

#include <stddef.h>

typedef enum ktmeStatus {
    KTME_STATUS_OK = 0,

    KTME_STATUS_NO_DATA,
    KTME_STATUS_UNSUPPORTED
} ktmeStatus;

typedef struct ktmeEngineSysFuncs {
    void *(*memAlloc)(size_t size);
    void (*memFree)(void *ptr);
    void (*logMessage)(const char *format, va_list args);
} ktmeEngineSysFuncs;

typedef struct ktmeEngine ktmeEngine;

ktmeEngine *ktmeEngineCreate(const ktmeEngineSysFuncs *sysFuncs);
void ktmeEngineDestroy(ktmeEngine *engine);

void *ktmeEngineMemAlloc(ktmeEngine *engine, size_t size);
void ktmeEngineMemFree(ktmeEngine *engine, void *ptr);
void ktmeEngineLogMessage(ktmeEngine *engine, const char *format, ...);
void ktmeEngineLogMessageV(ktmeEngine *engine, const char *format, va_list args);

#ifdef _MSC_VER
#define KTME_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define KTME_FORCEINLINE __attribute__((always_inline))
#else
#define KTME_FORCEINLINE inline
#endif
