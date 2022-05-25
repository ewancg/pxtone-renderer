cmake_minimum_required(VERSION 3.5)

project(pxtone-renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "-DpxINCLUDE_OGGVORBIS")
add_executable(pxtone-renderer
    main.cpp

    pxtone/pxtnDelay.cpp
    pxtone/pxtnDelay.h
    pxtone/pxtnDescriptor.cpp
    pxtone/pxtnDescriptor.h
    pxtone/pxtnError.cpp
    pxtone/pxtnError.h
    pxtone/pxtnEvelist.cpp
    pxtone/pxtnEvelist.h
    pxtone/pxtn.h
    pxtone/pxtnMaster.cpp
    pxtone/pxtnMaster.h
    pxtone/pxtnMax.h
    pxtone/pxtnMem.cpp
    pxtone/pxtnMem.h
    pxtone/pxtnOverDrive.cpp
    pxtone/pxtnOverDrive.h
    pxtone/pxtnPulse_Frequency.cpp
    pxtone/pxtnPulse_Frequency.h
    pxtone/pxtnPulse_NoiseBuilder.cpp
    pxtone/pxtnPulse_NoiseBuilder.h
    pxtone/pxtnPulse_Noise.cpp
    pxtone/pxtnPulse_Noise.h
    pxtone/pxtnPulse_Oggv.cpp
    pxtone/pxtnPulse_Oggv.h
    pxtone/pxtnPulse_Oscillator.cpp
    pxtone/pxtnPulse_Oscillator.h
    pxtone/pxtnPulse_PCM.cpp
    pxtone/pxtnPulse_PCM.h
    pxtone/pxtnService.cpp
    pxtone/pxtnService.h
    pxtone/pxtnService_moo.cpp
    pxtone/pxtnText.cpp
    pxtone/pxtnText.h
    pxtone/pxtnUnit.cpp
    pxtone/pxtnUnit.h
    pxtone/pxtnWoice.cpp
    pxtone/pxtnWoice.h
    pxtone/pxtnWoice_io.cpp
    pxtone/pxtnWoicePTV.cpp
    pxtone/pxtoneNoise.cpp
    pxtone/pxtoneNoise.h
)

find_package(Ogg)
find_package(Vorbis)
find_package(FLAC)
find_package(SndFile)

# Potentially add opus later -- it does not support conventional VBR/Compression levels

target_link_libraries(pxtone-renderer PRIVATE SndFile::sndfile Vorbis::vorbisfile FLAC::FLAC)
