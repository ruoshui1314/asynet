cmake_minimum_required (VERSION 3.1)
project (asynet)

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set (UTILS_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/src/utils")

# Use c++ 11
set (CMAKE_CXX_STANDARD 11)

# Add dependency library
include_directories (
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/thirdparty
)

aux_source_directory (src DIR_SRCS)
add_executable (asynet ${DIR_SRCS})

# add test
enable_testing ()
add_subdirectory (tests)

# install
set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})


include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_NAME "asynet")
set (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
set (CPACK_GENERATOR "TGZ")
include (CPack)