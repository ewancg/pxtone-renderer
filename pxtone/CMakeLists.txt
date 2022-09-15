project(pxtone)

set(PXTONE_LIB ${PROJECT_NAME})
# let parent use this target
set(PXTONE_LIB ${PXTONE_LIB} PARENT_SCOPE)

find_package(Vorbis)

list(APPEND PXTONE_SRCS
    pxtnData.cpp
    pxtnDelay.cpp
    pxtnError.cpp
    pxtnEvelist.cpp
    pxtnMaster.cpp
    pxtnMem.cpp
    pxtnOverDrive.cpp
    pxtnPulse_Frequency.cpp
    pxtnPulse_Noise.cpp
    pxtnPulse_NoiseBuilder.cpp
    pxtnPulse_Oggv.cpp
    pxtnPulse_Oscillator.cpp
    pxtnPulse_PCM.cpp
    pxtnService.cpp
    pxtnService_moo.cpp
    pxtnText.cpp
    pxtnUnit.cpp
    pxtnWoice.cpp
    pxtnWoicePTV.cpp
    pxtnWoice_io.cpp
    pxtoneNoise.cpp
)

add_library(${PXTONE_LIB} STATIC
    ${PXTONE_SRCS}
)

target_include_directories(${PXTONE_LIB}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PXTONE_LIB}
    PRIVATE
    ${FIXENDIAN_LIB}
)

if(Vorbis_FOUND)
    target_compile_definitions(${PXTONE_LIB}
        PRIVATE
        pxINCLUDE_OGGVORBIS
    )
    target_link_libraries(${PXTONE_LIB}
        PRIVATE
        Vorbis::vorbisfile
    )
endif()
