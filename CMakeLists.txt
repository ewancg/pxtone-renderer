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

set(BUILD_PROGRAMS OFF)
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_DOCS OFF)
set(INSTALL_MANPAGES OFF)

find_package(Ogg)
if( NOT OGG_FOUND)
    message(STATUS "libOgg not found on system -- downloading local version.")
    FetchContent_Declare(
	Ogg
	GIT_REPOSITORY https://github.com/xiph/ogg.git
	GIT_TAG        e1774cd77f471443541596e09078e78fdc342e4f # 1.3.5
    )
    FetchContent_MakeAvailable(Ogg)
endif()

set(VorbisfileName "Vorbis::vorbisfile")
find_package(Vorbis)
if( NOT VORBIS_FOUND)
    message(STATUS "libVorbis not found on system -- downloading local version.")
    FetchContent_Declare(
	Vorbis
	GIT_REPOSITORY https://gitlab.xiph.org/xiph/vorbis.git
	GIT_TAG        0657aee69dec8508a0011f47f3b69d7538e9d262 # 1.3.7
    )
    FetchContent_MakeAvailable(Vorbis)
    set(VorbisfileName "vorbisfile") # ugh
endif()


find_package(FLAC)
if( NOT FLAC_FOUND )
    message(STATUS "libFLAC not found on system -- downloading local version.")
    FetchContent_Declare(
	FLAC
	GIT_REPOSITORY https://github.com/xiph/flac.git # mirror
	GIT_TAG        1151c93e992bb8c7c6394e04aa880d711c531c7f # 1.3.4
    )
    FetchContent_MakeAvailable(FLAC)
endif()

find_package(SndFile)
if( NOT SndFile_FOUND )
    message(STATUS "libSndFile not found on system -- downloading local version.")
    FetchContent_Declare(
	SndFile
	GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
	GIT_TAG        ea3ac90e98c6a98cd52cae39010446fba368a2e3 # 1.1.0
    )

    # set(ENABLE_MPEG ON)
    FetchContent_MakeAvailable(SndFile)
endif()

# Potentially add opus later -- it does not support conventional VBR/Compression levels

target_link_libraries(pxtone-renderer PRIVATE SndFile::sndfile ${VorbisfileName} FLAC::FLAC)
