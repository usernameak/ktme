cmake_minimum_required(VERSION 3.17)

find_package(portaudio)

add_library(ktme STATIC
        ktmeMixer.c
        ktmeMixer.h
        ktmeChannel.c
        ktmeChannel.h
        ktmeChannelP.h
        ktmeFrame.h
        ktmeEngine.c
        ktmeEngine.h
        ktmeAudioOut.h
        ktmeAudioOut.c
        ktmeAudioSource.h
        ktmeAudioDecoder.c
        ktmeAudioDecoder.h
        ktmeCommon.h)

if (portaudio_FOUND)
    target_link_libraries(ktme PUBLIC portaudio)

    target_sources(ktme PRIVATE
            ktmeAoPortAudio.c
            ktmeAoPortAudio.h)
endif ()

target_include_directories(ktme INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")
