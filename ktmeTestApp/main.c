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

    ktmeChannel *chan = ktmeMixerChannelAlloc(mixer);

    ktmeMixerChannelPlay(mixer, chan);

    Sleep(1000);

    ktmeMixerChannelFree(mixer, chan);

    ktmeAudioOutPortAudioDestroy(ao);
    ktmeMixerDestroy(mixer);
    ktmeEngineDestroy(engine);

    return 0;
}
