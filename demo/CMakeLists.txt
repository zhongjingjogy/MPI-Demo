cmake_minimum_required(VERSION 3.0)
project(moddatacleaning-mpi VERSION 2.0.0 LANGUAGES CXX)

find_package(MPI REQUIRED)

add_executable(mpidemo main.cpp)

target_link_libraries(mpidemo
  PRIVATE
    ${MPI_C_LIBRARIES}
)
