#include "ktmeEngine.h"

#include <stdarg.h>

struct ktmeEngine {
    ktmeEngineSysFuncs m_sysFuncs;
};

ktmeEngine *ktmeEngineCreate(const ktmeEngineSysFuncs *allocRoutines) {
    ktmeEngine *engine = allocRoutines->memAlloc(sizeof(ktmeEngine));
    engine->m_sysFuncs = *allocRoutines;
    return engine;
}

void ktmeEngineDestroy(ktmeEngine *engine) {
    engine->m_sysFuncs.memFree(engine);
}

void *ktmeEngineMemAlloc(ktmeEngine *engine, size_t size) {
    return engine->m_sysFuncs.memAlloc(size);
}

void ktmeEngineMemFree(ktmeEngine *engine, void *ptr) {
    engine->m_sysFuncs.memFree(ptr);
}

void ktmeEngineLogMessage(ktmeEngine *engine, const char *format, ...) {
    va_list args;
    va_start(args, format);
    ktmeEngineLogMessageV(engine, format, args);
    va_end(args);
}

void ktmeEngineLogMessageV(ktmeEngine *engine, const char *format, va_list args) {
    engine->m_sysFuncs.logMessage(format, args);
}
