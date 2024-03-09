#include "ktmeDataSource.h"

#include <assert.h>

ktmeStatus ktmeDataSourceSkip(ktmeDataSource *self, uint32_t numBytes) {
    if (numBytes == 0) return KTME_STATUS_OK;

    assert(numBytes <= INT32_MAX);

    ktmeStatus status = ktmeDataSourceSeek(self, KTME_SEEK_CUR, numBytes);
    if (status != KTME_STATUS_UNSUPPORTED) return status;

    // slow and dumb read loop
    uint32_t bytesRemaining = numBytes;
    while (bytesRemaining) {
        char tmp[1024]; // maybe less?
        uint32_t bytesRequired = bytesRemaining > sizeof(tmp) ? sizeof(tmp) : bytesRemaining;
        uint32_t bytesRead     = 0;
        status                 = ktmeDataSourceRead(self, tmp, bytesRequired, &bytesRead);
        if (bytesRead == 0) {
            return KTME_STATUS_OK; // EOF
        }
        if (status != KTME_STATUS_OK) return status;

        bytesRemaining -= bytesRead;
    }
    return KTME_STATUS_OK;
}