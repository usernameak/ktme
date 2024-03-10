#pragma once

#include "ktmeCommon.h"
#include "ktmeEngine.h"

typedef struct ktmeDataSourceFile ktmeDataSourceFile;

ktmeDataSourceFile *ktmeDataSourceFileCreate(ktmeEngine *engine, const char *filename, ktmeStatus *status);
void ktmeDataSourceFileDestroy(ktmeDataSourceFile *codec);
