############################################################################
# Copyright (c) 2020, QuantStack and xena contributors                     #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.8)
project(xena)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(XENA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XENA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(XENA_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

# Versionning
# ===========

# Project version
file(STRINGS "${XENA_INCLUDE_DIR}/xena/xena.hpp" xena_version_defines
     REGEX "#define XENA_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xena_version_defines})
    if(ver MATCHES "#define XENA_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XENA_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XENA_VERSION
    ${XENA_VERSION_MAJOR}.${XENA_VERSION_MINOR}.${XENA_VERSION_PATCH})
message(STATUS "xena version: v${XENA_VERSION}")

# Binary version
# See the following URL for explanations about the binary versionning
# https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html#Updating-version-info
file(STRINGS "${XENA_INCLUDE_DIR}/xena/xena.hpp" xena_version_defines
     REGEX "#define XENA_BINARY_(CURRENT|REVISION|AGE)")
foreach(ver ${xena_version_defines})
    if(ver MATCHES "#define XENA_BINARY_(CURRENT|REVISION|AGE) +([^ ]+)$")
        set(XENA_BINARY_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(XENA_BINARY_VERSION
    ${XENA_BINARY_CURRENT}.${XENA_BINARY_REVISION}.${XENA_BINARY_AGE})
message(STATUS "xena binary version: v${XENA_BINARY_VERSION}")

# Build options
# =============

# Compilation options
option(XENA_DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)

# Test options
option(XENA_BUILD_TESTS "xena test suite" OFF)
option(XENA_DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)

set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "Not linking statically with OpenSSL")

# Dependencies
# ============

set(nlohmann_json_REQUIRED_VERSION 3.2.0)
set(xtl_REQUIRED_VERSION 0.5)
set(cppzmq_REQUIRED_VERSION 4.4.1)
set(zeromq_REQUIRED_VERSION 4.3.2)
set(xeus_REQUIRED_VERSION 0.25.3)

if (NOT TARGET nlohmann_json)
    find_package(nlohmann_json ${nlohmann_json_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET xtl)
    find_package(xtl ${xtl_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET cppzmq)
    find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
endif ()

if (NOT TARGET libzmq AND NOT TARGET libzmq-static)
    if (WIN32)
        find_package(zeromq ${zeromq_REQUIRED_VERSION} REQUIRED)
    else ()
        find_package(zeromq ${zeromq_REQUIRED_VERSION} QUIET)

        if (NOT ZeroMQ_FOUND)
            message(STATUS "CMake libzmq package not found, trying again with pkg-config")
            find_package(PkgConfig)
            pkg_check_modules(ZeroMQ libzmq>=${zeromq_REQUIRED_VERSION} REQUIRED)
            set(ZeroMQ_VERSION ${PC_LIBZMQ_VERSION})
            find_library(ZeroMQ_LIBRARY NAMES libzmq.so libzmq.dylib libzmq.dll
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            find_library(ZeroMQ_STATIC_LIBRARY NAMES libzmq-static.a libzmq.a libzmq.dll.a
                 PATHS ${PC_LIBZMQ_LIBDIR} ${PC_LIBZMQ_LIBRARY_DIRS})
            message(STATUS "STATIC_LIBRARY" {ZeroMQ_LIBRARY})
            message(STATUS "STATIC_STATIC_LIBRARY" {ZeroMQ_STATIC_LIBRARY})
         endif ()
    endif ()
endif ()

if (NOT DEFINED OPENSSL_LIBRARY)
    set(OPENSSL_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
    find_package(OpenSSL REQUIRED)
endif ()

if (NOT TARGET xeus)
    find_package(xeus ${seux_REQUIRED_VERSION} REQUIRED)
endif ()

# Source files
# ============

set(XENA_HEADERS
    ${XENA_INCLUDE_DIR}/xena/xchannel.hpp
    ${XENA_INCLUDE_DIR}/xena/xena.hpp
    ${XENA_INCLUDE_DIR}/xena/xenum_iterator.hpp
)

set(XENA_SOURCES
    ${XENA_SOURCE_DIR}/xheartbeat.hpp
    ${XENA_SOURCE_DIR}/xheartbeat.cpp
    ${XENA_SOURCE_DIR}/xutils.hpp
    ${XENA_SOURCE_DIR}/xutils.cpp
    ${XENA_SOURCE_DIR}/xzmq_client.hpp
    ${XENA_SOURCE_DIR}/xzmq_multi_client.hpp
    ${XENA_SOURCE_DIR}/xzmq_multi_client.cpp
)

# Targets and link
# ================

include(CheckCXXCompilerFlag)

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

if (NOT APPLE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

set(target_name xena)
add_library(xena SHARED ${XENA_SOURCES} ${XENA_HEADERS})

if (APPLE)
    set_target_properties(${target_name} PROPERTIES MACOSX_RPATH ON)
else ()
    set_target_properties(${target_name} PROPERTIES BUILD_WITH_INSTALL_RPATH 1)
endif ()

target_include_directories(
    ${target_name}
    PUBLIC $<BUILD_INTERFACE:${XENA_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    ${target_name}
    PUBLIC cppzmq
    PUBLIC nlohmann_json::nlohmann_json
    PUBLIC xtl
    PUBLIC OpenSSL::Crypto
)

if (NOT MSVC)
    if (APPLE)
        target_link_libraries(xena PUBLIC "-framework CoreFoundation")
    else ()
        find_package(LibUUID REQUIRED)
        target_link_libraries(${target_name} PUBLIC LibUUID::LibUUID)
    endif ()
endif ()

set_target_properties(
    ${target_name}
    PROPERTIES
    PUBLIC_HEADER "${XENA_HEADERS}"
    COMPILE_DEFINITIONS "XENA_EXPORTS"
    PREFIX ""
    VERSION ${XENA_BINARY_VERSION}
    SOVERSION ${XENA_BINARY_CURRENT}
    OUTPUT_NAME "lib${target_name}"
)

# Compilation flags
# =================

target_compile_features(${target_name} PRIVATE cxx_std_11)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Intel")

    target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)

    if (NOT XENA_DISABLE_ARCH_NATIVE)
        target_compile_options(${target_name} PUBLIC -march=native)
    endif ()

    message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
endif()

if (MSVC)
    target_compile_definitions(${target_name} PUBLIC -DNOMINMAX)
    target_compile_options(${target_name} PUBLIC /DGUID_WINDOWS /MP /bigobj)
    target_compile_options(${target_name} PUBLIC /wd4251 /wd4996)
elseif (APPLE)
    target_compile_definitions(${target_name} PUBLIC -DGUID_CFUUID)
else ()
    target_compile_definitions(${target_name} PUBLIC -DGUID_LIBUUID)
endif ()

if(CMAKE_TARGET_SYSTEM MATCHES "Linux" AND OPENSSL_USE_STATIC_LIBS)
    # Do not reexport OpenSSL symbols from xena, for libraries
    #   Prevents conflicts with other versions of OpenSSL
    #   loaded in the same process namespace, which can cause
    #   crashes if the versions are not compatible.
    set_target_properties(xena PROPERTIES LINK_FLAGS "-Wl,--exclude-libs,libcrypto.a")
endif()

# Tests
# =====

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(XENA_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xenaConfig.cmake")

install(TARGETS ${target_name}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xena)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Configure 'xenaConfig.cmake' for a build tree
set(XENA_CONFIG_CODE "####### Expanded from \@XENA_CONFIG_CODE\@ #######\n")
set(XENA_CONFIG_CODE "${XENA_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XENA_CONFIG_CODE "${XENA_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xenaConfig.cmake' for an install tree
set(XENA_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XENA_CMAKECONFIG_INSTALL_DIR})


write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${XENA_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindLibUUID.cmake
              DESTINATION ${XENA_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XENA_CMAKECONFIG_INSTALL_DIR})
