qm_import(Preprocess)

if(NOT TARGET ChorusKit::AppCore)
    find_package(ChorusKit REQUIRED)
endif()

set(IDECORE_PROJECT_DESCRIPTION "ChorusKit IDE Application Foundation")
set(IDECORE_PROJECT_COPYRIGHT "Copyright 2023-2024 Team OpenVPI")
set(IDECORE_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
set(IDECORE_BUILD_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../etc/include)

# ----------------------------------
# Configurations
# ----------------------------------
set(QMSETUP_DEFINITION_SCOPE DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(QMSETUP_DEFINITION_NUMERICAL on)

qm_add_definition(IDECORE_TEST_CONFIG true)

qm_generate_config(${IDECORE_BUILD_INCLUDE_DIR}/idecoreCore/idecoreconfig.h)

if(IDECORE_INSTALL)
    install(FILES ${IDECORE_BUILD_INCLUDE_DIR}/idecoreCore/idecoreconfig.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${IDECORE_INSTALL_NAME}/idecoreCore
    )
endif()

# ----------------------------------
# CMake API
# ----------------------------------
macro(idecore_add_library _target)
    set(options AUTOGEN NO_SYNC_INCLUDE NO_WIN_RC)
    set(oneValueArgs SYNC_INCLUDE_PREFIX PREFIX)
    set(multiValueArgs SYNC_INCLUDE_OPTIONS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_AUTOGEN)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTOUIC ON)
        set(CMAKE_AUTORCC ON)
    endif()

    if(IDECORE_BUILD_STATIC)
        set(_type STATIC)
    else()
        set(_type SHARED)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT FUNC_NO_WIN_RC AND(${_type} STREQUAL "SHARED"))
        qm_add_win_rc(${_target}
            DESCRIPTION ${IDECORE_PROJECT_DESCRIPTION}
            COPYRIGHT ${IDECORE_PROJECT_COPYRIGHT}
        )
    endif()

    if(FUNC_PREFIX)
        set(_prefix_option PREFIX ${FUNC_PREFIX})
    else()
        set(_prefix_option)
    endif()

    # Set global definitions
    qm_export_defines(${_target} ${_prefix_option})

    # Configure target
    qm_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Add include directories
    target_include_directories(${_target} PRIVATE ${IDECORE_BUILD_INCLUDE_DIR})
    target_include_directories(${_target} PRIVATE .)

    # Library name
    if(${_target} MATCHES "^idecore(.+)")
        set(_name ${CMAKE_MATCH_1})
        set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})
    else()
        set(_name ${_target})
    endif()

    add_library(${IDECORE_INSTALL_NAME}::${_name} ALIAS ${_target})

    if(FUNC_SYNC_INCLUDE_PREFIX)
        set(_inc_name ${FUNC_SYNC_INCLUDE_PREFIX})
    else()
        set(_inc_name ${_target})
    endif()

    set(_install_options)

    if(IDECORE_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${IDECORE_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${_target} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${IDECORE_INSTALL_NAME}>"
        )

        set(_install_options
            INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${IDECORE_INSTALL_NAME}/${_inc_name}"
        )
    endif()

    if(NOT FUNC_NO_SYNC_INCLUDE)
        # Generate a standard include directory in build directory
        qm_sync_include(. "${IDECORE_GENERATED_INCLUDE_DIR}/${_inc_name}" ${_install_options}
            ${FUNC_SYNC_INCLUDE_OPTIONS} FORCE
        )
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${IDECORE_GENERATED_INCLUDE_DIR}>"
        )
    endif()

    # Add to translate targets
    set_property(GLOBAL APPEND PROPERTY IDECORE_TRANSLATE_TARGETS ${_target})
endmacro()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(3rdparty)

add_subdirectory(core)

add_subdirectory(widgets)

# ----------------------------------
# Documentation
# ----------------------------------
if(IDECORE_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen REQUIRED)
    endif()

    set(_install_options)

    if(IDECORE_INSTALL)
        set(_install_options INSTALL_DIR share/doc/${IDECORE_INSTALL_NAME})
    endif()

    set(_doc_targets idecoreCore)

    set(IDECORE_DOXYGEN_TAGFILE
        ${CMAKE_BUILD_SHARE_DIR}/doc/${IDECORE_INSTALL_NAME}/${IDECORE_INSTALL_NAME}_tagfile.xml
        CACHE FILEPATH "idecore doxygen tag file path" FORCE
    )

    qm_import(Doxygen)
    qm_setup_doxygen(idecore_RunDoxygen
        NAME "idecore"
        DESCRIPTION "${IDECORE_PROJECT_DESCRIPTION}"
        MDFILE ../README.md
        OUTPUT_DIR ${CMAKE_BUILD_SHARE_DIR}/doc/${IDECORE_INSTALL_NAME}
        INPUT core widgets
        TARGETS ${_doc_targets}
        DEPENDS ${_doc_targets}
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=Q_SIGNALS Q_SLOTS=Q_SLOTS
        GENERATE_TAGFILE "${IDECORE_DOXYGEN_TAGFILE}"
        ${_install_options}
    )
endif()

# ----------------------------------
# Install
# ----------------------------------
if(IDECORE_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${IDECORE_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${IDECORE_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${IDECORE_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${IDECORE_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${IDECORE_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${IDECORE_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${IDECORE_INSTALL_NAME}Targets
        FILE "${IDECORE_INSTALL_NAME}Targets.cmake"
        NAMESPACE ${IDECORE_INSTALL_NAME}::
        DESTINATION ${_install_dir}
    )
endif()