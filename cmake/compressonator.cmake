#
# Copyright (c) 2024 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
# Locate Compressonator SDK
if (NOT DEFINED ENV{COMPRESSONATOR_ROOT})
    message(FATAL_ERROR "COMPRESSONATOR_ROOT environment variable is not defined. Please install the Compressonator SDK from https://github.com/GPUOpen-Tools/compressonator/releases.")
endif()
set(COMPRESSONATOR_ROOT "$ENV{COMPRESSONATOR_ROOT}")

find_path(Compressonator_INCLUDE_DIR
        NAMES Compressonator.h
        PATHS
        "${COMPRESSONATOR_ROOT}/include"
)

find_library(Compressonator_LIBRARY
        NAMES Compressonator_MT
        PATHS
        "${COMPRESSONATOR_ROOT}/lib/bin/x64"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Compressonator DEFAULT_MSG
        Compressonator_INCLUDE_DIR
        Compressonator_LIBRARY
)

if (!Compressonator_FOUND)
    message(FATAL_ERROR "Compressonator SDK not found")
endif()

target_include_directories(${GLB2ZSCENE_TARGET} PUBLIC ${Compressonator_INCLUDE_DIR})
target_link_libraries(${GLB2ZSCENE_TARGET} "${COMPRESSONATOR_ROOT}/lib/bin/x64/Compressonator_MT_DLL.dll")
