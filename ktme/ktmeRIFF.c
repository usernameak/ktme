#include "ktmeRIFF.h"

ktmeStatus ktmeRIFFReadSubchunkHeader(ktmeDataSource *src, uint32_t *type, uint32_t *size) {
    ktmeStatus status = ktmeDataSourceReadU32LE(src, type);
    if (status != KTME_STATUS_OK) return status;

    status = ktmeDataSourceReadU32LE(src, size);
    if (status != KTME_STATUS_OK) return status;

    return KTME_STATUS_OK;
}
