cmake_minimum_required(VERSION 3.5)

project(pxtone-renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

set(RENDERER_EXE ${PROJECT_NAME})


# Pxtone library
# Sets PXTONE_LIB to its library target
add_subdirectory(pxtone)


# Renderer
set(DEPENDENCIES_COMPILE_OPTIONS "")
set(DEPENDENCIES_LIBRARY_DIRS "")
set(DEPENDENCIES_LINK_OPTIONS "")
set(DEPENDENCIES_LEGACY_LDFLAGS "")

set(SNDFILE_LIB "")
find_package(SndFile)
if(SndFile_FOUND)
    set(SNDFILE_LIB SndFile::sndfile)
else()
    # only provides a CMake module if it was built via CMake, try pkg-config as fallback
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SNDFILE_PKGCONFIG REQUIRED sndfile)
    list(APPEND DEPENDENCIES_INCLUDE_DIRS ${SNDFILE_PKGCONFIG_INCLUDE_DIRS})
    list(APPEND DEPENDENCIES_COMPILE_OPTIONS ${SNDFILE_PKGCONFIG_CFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LIBRARY_DIRS ${SNDFILE_PKGCONFIG_LIBRARY_DIRS})
    list(APPEND DEPENDENCIES_LINK_OPTIONS ${SNDFILE_PKGCONFIG_LDFLAGS_OTHER})
    list(APPEND DEPENDENCIES_LEGACY_LDFLAGS ${SNDFILE_PKGCONFIG_LDFLAGS})
    set(SNDFILE_LIB ${SNDFILE_PKGCONFIG_LIBRARIES})
endif()
# Potentially add opus later -- it does not support conventional VBR/Compression levels

list(APPEND RENDERER_SRCS
    main.cpp
)

add_executable(${RENDERER_EXE}
    ${RENDERER_SRCS}
)

target_link_libraries(${RENDERER_EXE} PRIVATE
    ${PXTONE_LIB}
    ${SNDFILE_LIB}
)

if (SNDFILE_PKGCONFIG_FOUND)
    target_include_directories(${RENDERER_EXE} SYSTEM PRIVATE ${DEPENDENCIES_INCLUDE_DIRS})
    target_compile_definitions(${RENDERER_EXE} PRIVATE ${DEPENDENCIES_DEFINES})
    target_compile_options(${RENDERER_EXE} PRIVATE ${DEPENDENCIES_COMPILE_OPTIONS})
    if ("${CMAKE_VERSION}" VERSION_LESS "3.13")
        message(WARNING
            "CMake version is <3.13, using old pkg-config LDFLAGS. "
            "You may encounter linking problems with these!"
        )
        target_link_libraries(${RENDERER_EXE} PRIVATE ${DEPENDENCIES_LEGACY_LDFLAGS})
    else()
        target_link_directories(${RENDERER_EXE} PRIVATE ${DEPENDENCIES_LIBRARY_DIRS})
        target_link_options(${RENDERER_EXE} PRIVATE ${DEPENDENCIES_LINK_OPTIONS})
    endif()
endif()

install(TARGETS ${RENDERER_EXE} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
