cmake_minimum_required(VERSION 3.16)
project(reusedist)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BUILD_ACADL
    "ON"
    CACHE
      STRING
      "whether acadl's shared library should be built or assumed to be already built and merely linked"
)
set(BUILD_ASWDL
    "ON"
    CACHE
      STRING
      "whether aswdl's shared library should be built or assumed to be already built and merely linked"
)
set(LIBREUSEDIST_OUTPUT_DIRECTORY
    "${PROJECT_SOURCE_DIR}/reusedist/backend"
    CACHE STRING "directory where reusedist.cpython file is to be moved")

set(MOVE_LIBREUSEDIST
    "OFF"
    CACHE STRING "libreusedist is moved to reusedist/backend")
set(MOVE_REUSEDIST_SO
    "OFF"
    CACHE STRING "reusedist.cpython.so is moved to reusedist/backend")

set(BACKEND_SRC_DIR reusedist/backend)

# show compile commands
set(CMAKE_VERBOSE_MAKEFILE
    ON
    CACHE BOOL "ON")

# set path to ACADL repository
if(DEFINED ACADL_PATH)
  message("ACADL_PATH set to command line parameter: ${ACADL_PATH}")
else()
  set(ACADL_PATH ${PROJECT_SOURCE_DIR}/../acadl)
  message("ACADL_PATH set to default: ${ACADL_PATH}")
endif()
# set path to ASWDL repository
if(DEFINED ASWDL_PATH)
  message("ASWDL_PATH set to command line parameter: ${ASWDL_PATH}")
else()
  set(ASWDL_PATH ${PROJECT_SOURCE_DIR}/../aswdl)
  message("ASWDL_PATH set to default: ${ASWDL_PATH}")
endif()
# check compiler versions
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if(CXX_COMPILER_VERSION VERSION_LESS 9.0)
    message(STATUS "GCC version must be at least 9.0!")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if(CXX_COMPILER_VERSION VERSION_LESS 12.0.0)
    message(STATUS "Clang version must be at least 12.0.0!")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  if(CXX_COMPILER_VERSION VERSION_LESS 12.0.0)
    message(STATUS "AppleClang version must be at least 12.0.0!")
  endif()
else()
  message(FATAL_ERROR "Please, use GCC or (Apple)Clang compiler!")
endif()

find_package(Git)

# download submodules if they aren't present
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  # update submodules
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git submodule update --init --recursive failed with $GIT_SUBMOD_RESULT}, please check git log"
      )
    endif()
  endif()
endif()

# add pybind11 but only if its not already included in this project
if(NOT TARGET pybind11 OR NOT TARGET pybind11::pybind11)
  message("adding pybind11 from aswdl")
  add_subdirectory(external/pybind11)
else()
  message("not adding pybind11 from aswdl")
endif()

# shared reusedist lib
add_library(libreusedist SHARED ${BACKEND_SRC_DIR}/reuseAnalyzer.cpp)
# rename to libreusedist
set_target_properties(libreusedist PROPERTIES OUTPUT_NAME "reusedist")

pybind11_add_module(reusedist ${BACKEND_SRC_DIR}/reusedist_wrapper.cpp)
# don't show deprecation warnings for libpython
target_compile_options(reusedist PRIVATE "-Wno-deprecated-declarations")

target_link_libraries(reusedist PRIVATE libreusedist)
# use gold as linker when clang is used
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_link_options(reusedist PRIVATE "-fuse-ld=gold")
endif()

# depending on if acadl /aswdl should be built or not, either the subdirectories
# are added or merely linked to the .so
if(BUILD_ACADL)
  message("set to build acadl dependency for this project")
  if(NOT TARGET acadl OR NOT TARGET libacadl)
    message("adding acadl from reusedist")
    add_subdirectory(${ACADL_PATH} ${ACADL_PATH}/acadl/backend/)
  else()
    message("not adding acadl from reusedist")
  endif()
  target_link_libraries(libreusedist PRIVATE libacadl)
else()
  message("set to find acadl dependency and just link it")
  find_library(
    LIBACADL
    NAMES acadl
    HINTS ${ACADL_PATH}/acadl/backend NO_CACHE)
  message("acadl lib found at ${LIBACADL}")
  target_link_libraries(libreusedist PRIVATE ${LIBACADL})
endif()

if(BUILD_ASWDL)
  message("set to build aswdl dependency for this project")
  if(NOT TARGET aswdl OR NOT TARGET libaswdlforreusedist)
    message("adding aswdl from reusedist")
    add_subdirectory(${ASWDL_PATH} ${ASWDL_PATH}/aswdl/backend/)
  else()
    message("aswdl not added from reusedist")
  endif()
  target_link_libraries(libreusedist PRIVATE libaswdl)
else()
  message("set to find aswdl dependency and just link it")
  find_library(
    LIBASWDL
    NAMES aswdl
    HINTS ${ASWDL_PATH}/aswdl/backend NO_CACHE)
  message("aswdl lib found at ${LIBASWDL}")
  target_link_libraries(libreusedist PRIVATE ${LIBASWDL})
endif()

# add include directories of ACADL dependencies
target_include_directories(reusedist PRIVATE ${ACADL_PATH}/acadl/backend/)
target_include_directories(reusedist PRIVATE ${ASWDL_PATH}/aswdl/backend/)
target_include_directories(reusedist PRIVATE ${ACADL_PATH}/external/fmt/include)
target_include_directories(reusedist
                           PRIVATE ${ACADL_PATH}/external/json/include)
target_include_directories(reusedist PRIVATE external/pycachesim)

target_include_directories(libreusedist PRIVATE ${ACADL_PATH}/acadl/backend/)
target_include_directories(libreusedist PRIVATE ${ASWDL_PATH}/aswdl/backend/)
target_include_directories(libreusedist
                           PRIVATE ${ACADL_PATH}/external/fmt/include)
target_include_directories(libreusedist
                           PRIVATE ${ACADL_PATH}/external/json/include)
target_include_directories(libreusedist PRIVATE external/pycachesim)

# move libreusedist into reusedist/backend
if(MOVE_LIBREUSEDIST)
  set_target_properties(
    libreusedist PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                            ${LIBREUSEDIST_OUTPUT_DIRECTORY})
endif()
# move reusedist into reusedist/backend
if(MOVE_REUSEDIST_SO)
  set_target_properties(
    reusedist PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                         ${PROJECT_SOURCE_DIR}/reusedist/backend)
endif()
