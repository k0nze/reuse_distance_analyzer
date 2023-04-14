cmake_minimum_required(VERSION 3.15)
project(reuse_dist_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BACKEND_DIR_PATH ${PROJECT_SOURCE_DIR}/reuse_distance_analyzer/backend)

set(MOVE_LIB
    "ON"
    CACHE STRING "move TODO")

add_library(reuse_distance_analyzer SHARED
            reuse_distance_analyzer/backend/reuseAnalyzer.cpp)

if(MOVE_LIB)
    set_target_properties(
        reuse_distance_analyzer PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                           ${BACKEND_DIR_PATH})
endif()

add_executable(tests_cpp tests/cpp/main.cc)
target_include_directories(tests_cpp PRIVATE ${BACKEND_DIR_PATH})
target_link_libraries(tests_cpp reuse_distance_analyzer)
