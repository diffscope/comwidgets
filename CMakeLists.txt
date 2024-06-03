cmake_minimum_required(VERSION 3.17)

project(idecore VERSION 0.0.1.1 LANGUAGES CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(IDECORE_BUILD_STATIC "Build static libraries" OFF)
option(IDECORE_BUILD_TESTS "Build test cases" OFF)
option(IDECORE_BUILD_WIDGETS "Build widgets module" ON)
option(IDECORE_BUILD_TRANSLATIONS "Build translations" ON)
option(IDECORE_BUILD_DOCUMENTATIONS "Build documentations" OFF)
option(IDECORE_INSTALL "Install library" ON)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
endif()

if(IDECORE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(IDECORE_VERSION ${PROJECT_VERSION})
set(IDECORE_INSTALL_NAME ${PROJECT_NAME})

# ----------------------------------
# Find basic dependencies
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)
qm_init_directories()

# ----------------------------------
# Add source modules
# ----------------------------------
add_subdirectory(src)

add_subdirectory(share)

if(IDECORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()