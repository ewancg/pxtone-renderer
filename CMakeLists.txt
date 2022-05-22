cmake_minimum_required(VERSION 3.5)
include(FetchContent)

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


set(INSTALL_MANPAGES OFF) # ??? why would i want that

FetchContent_Declare(
  Vorbis
  GIT_REPOSITORY https://gitlab.xiph.org/xiph/vorbis.git
  GIT_TAG        0657aee69dec8508a0011f47f3b69d7538e9d262 # 1.3.7
)
FetchContent_Declare(
  SndFile
  GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
  GIT_TAG        ea3ac90e98c6a98cd52cae39010446fba368a2e3 # 1.1.0
)
FetchContent_Declare(
  FLAC
  GIT_REPOSITORY https://github.com/xiph/flac.git # mirror
  GIT_TAG        1151c93e992bb8c7c6394e04aa880d711c531c7f # 1.3.4
)
set(BUILD_PROGRAMS OFF)
set(BUILD_TESTING OFF)
set(ENABLE_EXTERNAL_LIBS ON)
# set(ENABLE_MPEG ON)

#FetchContent_MakeAvailable(vorbis FLAC sndfile)
#add_subdirectory("${CMAKE_CACHEFILE_DIR}/_deps/vorbis-src")

find_package(SndFile REQUIRED)
find_package(Vorbis REQUIRED)
#find_package(Ogg REQUIRED)
 find_package(FLAC REQUIRED)
# Potentially add opus later -- it does not support conventional VBR/Compression levels

target_link_libraries(pxtone-renderer PRIVATE SndFile::sndfile Vorbis::vorbisfile FLAC::FLAC)
