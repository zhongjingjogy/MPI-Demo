cmake_minimum_required(VERSION 3.0)
project(mpidemo VERSION 2.0.0 LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(./demo)
add_subdirectory(./serialization)
add_subdirectory(./random)
add_subdirectory(./format)

add_subdirectory(./3rd/fmt)