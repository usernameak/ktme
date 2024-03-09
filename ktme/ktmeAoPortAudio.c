#include "ktmeAoPortAudio.h"

#include "ktmeMixer.h"

#include <portaudio.h>

static const ktmeAudioOutFuncs g_aoPortAudioFuncs;

static int g_portAudioRefCount = 0;

static int ktmeAudioOutPortAudioCallback(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {

    (void)input;
    (void)timeInfo;
    (void)statusFlags;

    ktmeAudioOutPortAudio *ao = userData;
    ktmeMixerMixPull(ao->m_mixer, frameCount, output);
    return 0;
}

ktmeAudioOutPortAudio *ktmeAudioOutPortAudioCreate(ktmeEngine *engine) {
    if (g_portAudioRefCount == 0) {
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            ktmeEngineLogMessage(engine,
                "ktmeAudioOutPortAudioCreate error in Pa_Initialize: %s\n",
                Pa_GetErrorText(err));
            return NULL; // TODO: properly handle the error
        }
        g_portAudioRefCount++;
    }

    ktmeAudioOutPortAudio *ao = ktmeEngineMemAlloc(engine, sizeof(ktmeAudioOutPortAudio));
    ao->m_funcs               = &g_aoPortAudioFuncs;
    ao->m_engine              = engine;
    ao->m_mixer               = NULL;
    ao->m_state               = KTME_AUDIO_OUT_STATE_INACTIVE;
    ao->m_stream              = NULL;

    PaError err = Pa_OpenDefaultStream(
        &ao->m_stream,
        0,
        2,
        paInt32,
        44100,
        paFramesPerBufferUnspecified,
        ktmeAudioOutPortAudioCallback,
        ao);

    if (err != paNoError) {
        ktmeEngineLogMessage(ao->m_engine,
            "ktmeAudioOutPortAudioCreate error in Pa_OpenDefaultStream: %s\n",
            Pa_GetErrorText(err));

        ao->m_stream = NULL;
        ktmeAudioOutPortAudioDestroy(ao);
        return NULL;
    }

    return ao;
}

void ktmeAudioOutPortAudioDestroy(ktmeAudioOutPortAudio *ao) {
    if (ao == NULL) return;

    if (ao->m_mixer != NULL) {
        ktmeMixerLinkAudioOutput(ao->m_mixer, NULL);
    }

    if (ao->m_stream != NULL) {
        Pa_CloseStream(ao->m_stream); // TODO: log status
    }

    ktmeEngineMemFree(ao->m_engine, ao);

    if (--g_portAudioRefCount == 0) {
        Pa_Terminate(); // TODO: log status
    }
}

static void ktmeAudioOutPortAudioSetMixer(ktmeAudioOutBase *baseao, ktmeMixer *mixer) {
    ktmeAudioOutPortAudio *ao = (ktmeAudioOutPortAudio *)baseao;
    ao->m_mixer               = mixer;
}

static void ktmeAudioOutPortAudioSetState(ktmeAudioOutBase *baseao, ktmeAudioOutState state) {
    ktmeAudioOutPortAudio *ao = (ktmeAudioOutPortAudio *)baseao;
    if (state == ao->m_state) return;

    PaError err = paInvalidFlag;
    if (state == KTME_AUDIO_OUT_STATE_ACTIVE) {
        err = Pa_StartStream(ao->m_stream);
    } else if (state == KTME_AUDIO_OUT_STATE_INACTIVE) {
        err = Pa_StopStream(ao->m_stream);
    }

    if (err == paNoError) {
        ao->m_state = state;
    } else {
        ktmeEngineLogMessage(ao->m_engine, "ktmeAudioOutPortAudioSetState error: %s\n", Pa_GetErrorText(err));
    }
}

static const ktmeAudioOutFuncs g_aoPortAudioFuncs = {
    .setMixer = ktmeAudioOutPortAudioSetMixer,
    .setState = ktmeAudioOutPortAudioSetState
};
