#pragma once

#include <stdint.h>

#include "ktmeCommon.h"

typedef struct ktmeDataSource ktmeDataSource;

typedef struct ktmeDataSourceFuncs {
    void (*addRef)(ktmeDataSource *self);
    void (*release)(ktmeDataSource *self);

    ktmeStatus (*read)(ktmeDataSource *self, void *buf, uint32_t numBytes, uint32_t *numRead);
    ktmeStatus (*seek)(ktmeDataSource *self, ktmeSeekWhence whence, int32_t numBytes);
    uint32_t (*tell)(ktmeDataSource *self);
} ktmeDataSourceFuncs;

struct ktmeDataSource {
    const ktmeDataSourceFuncs *m_funcs;
};

inline void ktmeDataSourceAddRef(ktmeDataSource *self) {
    self->m_funcs->addRef(self);
}

inline void ktmeDataSourceRelease(ktmeDataSource *self) {
    self->m_funcs->release(self);
}

inline ktmeStatus ktmeDataSourceRead(ktmeDataSource *self, void *buf, uint32_t numBytes, uint32_t *numRead) {
    return self->m_funcs->read(self, buf, numBytes, numRead);
}

inline ktmeStatus ktmeDataSourceSeek(ktmeDataSource *self, ktmeSeekWhence whence, int32_t numBytes) {
    return self->m_funcs->seek(self, whence, numBytes);
}

inline uint32_t ktmeDataSourceTell(ktmeDataSource *self) {
    return self->m_funcs->tell(self);
}

inline ktmeStatus ktmeDataSourceReadFully(ktmeDataSource *self, void *buf, uint32_t numBytes) {
    uint32_t numRead  = 0;
    ktmeStatus status = ktmeDataSourceRead(self, buf, numBytes, &numRead);
    if (status != KTME_STATUS_OK) {
        return status;
    }
    if (numRead == 0) {
        return KTME_STATUS_NO_DATA;
    }
    return status;
}

ktmeStatus ktmeDataSourceSkip(ktmeDataSource *self, uint32_t numBytes);

inline ktmeStatus ktmeDataSourceReadU16LE(ktmeDataSource *self, uint16_t *value) {
    // TODO: support big-endian systems
    return ktmeDataSourceReadFully(self, value, sizeof(uint16_t));
}

inline ktmeStatus ktmeDataSourceReadU32LE(ktmeDataSource *self, uint32_t *value) {
    // TODO: support big-endian systems
    return ktmeDataSourceReadFully(self, value, sizeof(uint32_t));
}
