cmake_minimum_required(VERSION 3.5)

project(pxtone-renderer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(GNUInstallDirs)

set(RENDERER_EXE ${PROJECT_NAME})


### Single-file endian fix library
# set(FIXENDIAN_LIB fixendian)

# add_library(${FIXENDIAN_LIB} STATIC
#     endian.cpp
# )

# target_include_directories(${FIXENDIAN_LIB}
#     PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}
# )


# Pxtone library
# Sets PXTONE_LIB to its library target
add_subdirectory(pxtone)


# Renderer
# FIXME only provides a CMake module if it was built via CMake, maybe use pkg-config if not found this way
find_package(SndFile REQUIRED)
# Potentially add opus later -- it does not support conventional VBR/Compression levels

list(APPEND RENDERER_SRCS
    main.cpp
)

add_executable(${RENDERER_EXE}
    ${RENDERER_SRCS}
)

target_link_libraries(${RENDERER_EXE} PRIVATE
    ${PXTONE_LIB}
    #[[ ${FIXENDIAN_LIB} ]]
    SndFile::sndfile
)

install(TARGETS ${RENDERER_EXE} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
