cmake_minimum_required(VERSION 3.5)

project(pxtone-renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pxtone-renderer
    main.cpp
    global.h

    encoder.cpp
    encoder.h

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
