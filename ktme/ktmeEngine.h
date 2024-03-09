#pragma once

#include <stddef.h>

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
