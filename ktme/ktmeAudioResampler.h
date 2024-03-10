#include "ktmeEngine.h"

#include "ktmeAudioSource.h"

typedef struct ktmeAudioResampler {
    const ktmeAudioSourceFuncs *m_funcs;

    ktmeEngine *m_engine;
    ktmeAudioSourceBase *m_source;

    uint8_t m_buffer[1024];
    ktmeFrameS32 m_resampleInputBuffer[1024];
} ktmeAudioResampler;

ktmeAudioResampler *ktmeAudioResamplerCreate(ktmeEngine *engine);
void ktmeAudioResamplerDestroy(ktmeAudioResampler *resampler);

void ktmeAudioResamplerSetSource(ktmeAudioResampler *resampler, ktmeAudioSourceBase *source);
