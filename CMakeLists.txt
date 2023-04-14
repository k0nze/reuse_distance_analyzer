cmake_minimum_required(VERSION 3.15)
project(reuse_dist_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BACKEND_DIR_PATH ${PROJECT_SOURCE_DIR}/reuse_distance_analyzer/backend)

set(MOVE_LIB
    "ON"
    CACHE STRING "move TODO")

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
            reuse_distance_analyzer/backend/reuse_distance_analyzer.cc)

if(MOVE_LIB)
    set_target_properties(
        reuse_distance_analyzer PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                           ${BACKEND_DIR_PATH})
endif()

add_executable(tests_cpp tests/cpp/main.cc)
target_include_directories(tests_cpp PRIVATE ${BACKEND_DIR_PATH})
target_link_libraries(tests_cpp reuse_distance_analyzer)
