cmake_minimum_required(VERSION 3.15)
project(reuse_dist_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BACKEND_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/backend)

set(MOVE_LIB
    "ON"
    CACHE STRING "move libreuse_distance_analyzer.so into backend directory")

set(MOVE_CPYTHON_SO
    "ON"
    CACHE STRING "move cpython.so into backend directory")

# globally enable -fPIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

add_library(reuse_distance_analyzer SHARED
            ${BACKEND_SOURCE_DIR}/reuse_distance_analyzer.cc)

target_compile_options(reuse_distance_analyzer INTERFACE "-fsized-deallocation")

if(MOVE_LIB)
    set_target_properties(
        reuse_distance_analyzer PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                           ${BACKEND_SOURCE_DIR})
endif()

add_executable(test_cpp test/cpp/main.cc)
target_include_directories(test_cpp PRIVATE ${BACKEND_SOURCE_DIR})
target_link_libraries(test_cpp reuse_distance_analyzer)

add_subdirectory(external/pybind11)

pybind11_add_module(pybind_reuse_distance_analyzer
                    ${BACKEND_SOURCE_DIR}/reuse_distance_analyzer_wrapper.cc)

# don't show deprecation warnings for libpython
target_compile_options(pybind_reuse_distance_analyzer
                       PRIVATE "-Wno-deprecated-declarations")
target_link_libraries(pybind_reuse_distance_analyzer
                      PRIVATE reuse_distance_analyzer)

if(MOVE_CPYTHON_SO)
    set_target_properties(
        pybind_reuse_distance_analyzer PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                                  ${BACKEND_SOURCE_DIR})
endif()
