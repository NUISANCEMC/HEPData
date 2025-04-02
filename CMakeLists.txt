################################################################################
# MIT License

# Copyright (c) 2024 NUISANCE/HEPData

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
################################################################################

cmake_minimum_required (VERSION 3.17 FATAL_ERROR)

#Use the compilers found in the path
find_program(CMAKE_C_COMPILER NAMES $ENV{CC} gcc PATHS ENV PATH NO_DEFAULT_PATH)
find_program(CMAKE_CXX_COMPILER NAMES $ENV{CXX} g++ PATHS ENV PATH NO_DEFAULT_PATH)

SET(NUISANCEHEPData_VERSION 0.9.1)
project(NUISANCEHEPData VERSION ${NUISANCEHEPData_VERSION} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

option(NUISANCEHEPData_PYTHON_ENABLED "Whether to build python bindings" OFF)
option(NUISANCEHEPData_ENABLE_TESTS "Whether to enable test suite" OFF)
option(NUISANCEHEPData_ENABLE_SANITIZERS_CLI "Whether to enable ASAN LSAN and UBSAN" OFF)
option(NUISANCEHEPData_ENABLE_GCOV_CLI "Whether to enable GCOV" OFF)

if(NUISANCEHEPData_ENABLE_TESTS)
    SET(NUISANCEHEPData_ENABLE_SANITIZERS ON)
    SET(NUISANCEHEPData_ENABLE_GCOV ON)
else()
  if(NUISANCEHEPData_ENABLE_SANITIZERS_CLI)
    SET(NUISANCEHEPData_ENABLE_SANITIZERS ON)
  endif()
  if(NUISANCEHEPData_ENABLE_GCOV_CLI)
    SET(NUISANCEHEPData_ENABLE_GCOV ON)
  endif()
endif()

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}/"
             CACHE PATH "default install path" FORCE)
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/Modules)

######################### Dependencies

add_library(nuishpd_private_compile_options INTERFACE)
target_compile_options(nuishpd_private_compile_options INTERFACE -Wall -Werror -Wextra)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    target_compile_options(nuishpd_private_compile_options INTERFACE -Wno-class-memaccess)
endif()

if(NUISANCEHEPData_ENABLE_SANITIZERS)
  target_compile_options(nuishpd_private_compile_options BEFORE INTERFACE -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined)
  target_link_options(nuishpd_private_compile_options BEFORE INTERFACE -fsanitize=address -fsanitize=leak -fsanitize=undefined)
endif()

if(NUISANCEHEPData_ENABLE_GCOV)
  target_compile_options(nuishpd_private_compile_options BEFORE INTERFACE -fprofile-arcs -ftest-coverage)
  target_link_options(nuishpd_private_compile_options BEFORE INTERFACE -fprofile-arcs -ftest-coverage)
endif()

add_library(nuishpd_options INTERFACE)

target_include_directories(nuishpd_options INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
  $<INSTALL_INTERFACE:include>)

set_target_properties(nuishpd_options PROPERTIES EXPORT_NAME Options)
install(TARGETS nuishpd_options
    EXPORT NUISANCEHEPData-targets)

add_library(NUISANCEHEPData::Options ALIAS nuishpd_options)

include(get_cpm)

set(NUISANCEHEPData_CPR_USE_SYSTEM_CURL ON)
find_package(CURL QUIET)
if(NOT CURL_FOUND)
  set(NUISANCEHEPData_CPR_USE_SYSTEM_CURL OFF)
endif()

CPMFindPackage(
    NAME cpr
    VERSION 1.10.4
    GITHUB_REPOSITORY libcpr/cpr
    GIT_TAG "1.10.4"
    OPTIONS "CPR_USE_SYSTEM_CURL ${NUISANCEHEPData_CPR_USE_SYSTEM_CURL}"
)

if(NOT "${cpr_SOURCE_DIR}x" STREQUAL "x")
  # cpr doesn't include its own export set, if we are building it, add it to ours
  install(TARGETS cpr
          EXPORT NUISANCEHEPData-targets)
else()
  if(NOT TARGET CURL::libcurl_shared)
    get_target_property(LIBCURLLOC CURL::libcurl IMPORTED_LOCATION)
    #hack in an alias because we found a shared library
    get_filename_component(LIBCURLEXT ${LIBCURLLOC} EXT)
    if("${LIBCURLEXT}" STREQUAL ".so")
      add_library(CURL::libcurl_shared ALIAS CURL::libcurl)
    endif()
  endif()

  if(NOT TARGET CURL::libcurl_shared)
    message(FATAL_ERROR "Failed to find libcurl_shared")
  endif()
endif()

CPMFindPackage(
    NAME yaml-cpp
    VERSION 0.8.0
    GITHUB_REPOSITORY "jbeder/yaml-cpp"
    GIT_TAG "0.8.0"
    OPTIONS "YAML_CPP_INSTALL ON"
            "YAML_CPP_BUILD_TESTS OFF"
            "YAML_CPP_BUILD_CONTRIB OFF"
            "YAML_BUILD_SHARED_LIBS ON"
)

CPMAddPackage(
    NAME fmt
    VERSION 10.2.1
    GIT_TAG 10.2.1
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS 
        "FMT_INSTALL ON"
)

CPMAddPackage(
    NAME spdlog
    VERSION 1.14.1
    GIT_TAG v1.14.1
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS 
        "SPDLOG_COMPILED_LIB ON"
        "SPDLOG_BUILD_SHARED ON"
        "SPDLOG_FMT_EXTERNAL ON"
        "SPDLOG_INSTALL ON"
)

CPMAddPackage(
    NAME docopt
    GIT_TAG v0.6.3
    GITHUB_REPOSITORY docopt/docopt.cpp
)
add_library(docopt::docopt ALIAS docopt)

if(NUISANCEHEPData_PYTHON_ENABLED)

  CPMFindPackage(
      NAME pybind11
      VERSION 2.13.6
      GITHUB_REPOSITORY pybind/pybind11
      GIT_TAG "v2.13.6"
  )

  find_package(Python3 REQUIRED)
endif()

add_subdirectory(src/nuis/HEPData)
add_subdirectory(app)

if(NUISANCEHEPData_PYTHON_ENABLED)
  # PYTHON PATHS
  set(NUISANCEHEPData_PYSITEARCH "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
  set(NUISANCEHEPData_PYTHONPATH "python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")

  add_subdirectory(python)
endif()

install(EXPORT NUISANCEHEPData-targets
  FILE NUISANCEHEPDataTargets.cmake
  NAMESPACE NUISANCEHEPData::
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/NUISANCEHEPData
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/NUISANCEHEPDataConfig.cmake.in 
  ${CMAKE_BINARY_DIR}/NUISANCEHEPDataConfig.cmake
  INSTALL_DESTINATION 
    /this/is/ignored/for/some/reason/thanks/kitware
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/NUISANCEHEPDataConfigVersion.cmake
  VERSION ${NUISANCEHEPData_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES 
    ${CMAKE_BINARY_DIR}/NUISANCEHEPDataConfig.cmake 
    ${CMAKE_BINARY_DIR}/NUISANCEHEPDataConfigVersion.cmake 
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/NUISANCEHEPData)
