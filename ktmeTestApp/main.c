#include "ktmeAudioDecoder.h"
#include "ktmeAudioDecoderWAV.h"
#include "ktmeChannel.h"
#include "ktmeDataSourceFile.h"

#include <stdlib.h>

#include <ktmeMixer.h>
#include <ktmeAoPortAudio.h>
#include <stdio.h>
#include <windows.h>

static void sysFuncPrintf(const char *format, va_list args) {
    vfprintf(stderr, format, args);
}

int main(int argc, char **argv) {
    ktmeEngineSysFuncs sysFuncs = {
        .memAlloc   = malloc,
        .memFree    = free,
        .logMessage = sysFuncPrintf
    };

    ktmeEngine *engine = ktmeEngineCreate(&sysFuncs);
    ktmeMixer *mixer   = ktmeMixerCreate(engine, 8);

    ktmeAudioOutPortAudio *ao = ktmeAudioOutPortAudioCreate(engine);
    ktmeMixerLinkAudioOutput(mixer, (ktmeAudioOutBase *)ao);

    ktmeStatus status;

    ktmeDataSourceFile *wavFile = ktmeDataSourceFileCreate(engine, "test.wav", &status);
    if (status != KTME_STATUS_OK) {
        printf("error: file load failed\n");
        return 1;
    }

    ktmeAudioDecoderWAV *wavDecoder = ktmeAudioDecoderWAVCreate(engine);
    ktmeAudioDecoderLinkDataSource((ktmeAudioDecoderBase *)wavDecoder, (ktmeDataSource *)wavFile);

    ktmeDataSourceRelease((ktmeDataSource *)wavFile);

    ktmeChannel *chan = ktmeMixerChannelAlloc(mixer);
    ktmeChannelSetAudioSource(chan, (ktmeAudioSourceBase *)wavDecoder);
    ktmeMixerChannelPlay(mixer, chan);
    Sleep(10000);
    ktmeMixerChannelFree(mixer, chan);

    ktmeAudioDecoderWAVDestroy(wavDecoder);

    ktmeAudioOutPortAudioDestroy(ao);
    ktmeMixerDestroy(mixer);
    ktmeEngineDestroy(engine);

    return 0;
}
