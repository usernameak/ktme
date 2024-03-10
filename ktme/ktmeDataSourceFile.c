#include "ktmeDataSourceFile.h"

#include "ktmeDataSource.h"
#include <stdio.h>

struct ktmeDataSourceFile {
    const ktmeDataSourceFuncs *m_funcs;

    ktmeEngine *m_engine;
    uint32_t m_refCount;
    FILE *m_fp;
};

static const ktmeDataSourceFuncs g_ktmeDataSourceFileFuncs;

ktmeDataSourceFile *ktmeDataSourceFileCreate(ktmeEngine *engine, const char *filename, ktmeStatus *status) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        *status = KTME_STATUS_FOPEN_FAILED;
        return NULL;
    }

    ktmeDataSourceFile *self = ktmeEngineMemAlloc(engine, sizeof(ktmeDataSourceFile));
    self->m_funcs            = &g_ktmeDataSourceFileFuncs;
    self->m_engine           = engine;
    self->m_refCount         = 1;
    self->m_fp               = fp;

    *status = KTME_STATUS_OK;

    return self;
}

static void ktmeDataSourceFileAddRef(ktmeDataSource *ds) {
    ktmeDataSourceFile *self = (ktmeDataSourceFile *)ds;
    self->m_refCount++;
}

static void ktmeDataSourceFileRelease(ktmeDataSource *ds) {
    ktmeDataSourceFile *self = (ktmeDataSourceFile *)ds;
    if (--self->m_refCount == 0) {
        fclose(self->m_fp);
        ktmeEngineMemFree(self->m_engine, self);
    }
}

static ktmeStatus ktmeDataSourceFileRead(ktmeDataSource *ds, void *buf, uint32_t numBytes, uint32_t *numRead) {
    ktmeDataSourceFile *self = (ktmeDataSourceFile *)ds;

    errno = 0;

    size_t realNumRead = fread(buf, 1, numBytes, self->m_fp);
    if (realNumRead == -1) {
        if (numRead) *numRead = 0;
        return KTME_STATUS_FREAD_FAILED;
    }

    if (numRead) *numRead = realNumRead;
    return KTME_STATUS_OK;
}

ktmeStatus ktmeDataSourceFileSeek(ktmeDataSource *ds, ktmeSeekWhence whence, int32_t numBytes) {
    ktmeDataSourceFile *self = (ktmeDataSourceFile *)ds;

    errno = 0;

    int origin;
    switch (whence) {
    case KTME_SEEK_START: origin = SEEK_SET; break;
    case KTME_SEEK_END: origin = SEEK_END; break;
    case KTME_SEEK_CUR: origin = SEEK_CUR; break;
    default: return KTME_STATUS_INVALID_ARGUMENT;
    }

    if (fseek(self->m_fp, numBytes, origin) != 0) {
        return KTME_STATUS_FSEEK_FAILED;
    }

    return KTME_STATUS_OK;
}

uint32_t ktmeDataSourceFileTell(ktmeDataSource *ds) {
    ktmeDataSourceFile *self = (ktmeDataSourceFile *)ds;

    return ftell(self->m_fp);
}

static const ktmeDataSourceFuncs g_ktmeDataSourceFileFuncs = {
    .addRef  = ktmeDataSourceFileAddRef,
    .release = ktmeDataSourceFileRelease,
    .read    = ktmeDataSourceFileRead,
    .seek    = ktmeDataSourceFileSeek,
    .tell    = ktmeDataSourceFileTell,
};
