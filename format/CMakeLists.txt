cmake_minimum_required(VERSION 3.0)
project(format-mpi VERSION 2.0.0 LANGUAGES CXX)

find_package(MPI REQUIRED)

add_executable(formatmpidemo main.cpp)

target_link_libraries(formatmpidemo
  PRIVATE
    ${MPI_C_LIBRARIES}
    fmt::fmt
)
