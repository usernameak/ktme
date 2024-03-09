#pragma once

#include "ktmeCommon.h"
#include "ktmeDataSource.h"

#include <stdint.h>

ktmeStatus ktmeRIFFReadSubchunkHeader(ktmeDataSource *src, uint32_t *type, uint32_t *size);
