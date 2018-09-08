cmake_minimum_required(VERSION 3.0)
project(mpi-random VERSION 2.0.0 LANGUAGES CXX)

find_package(MPI REQUIRED)
find_package(Boost 1.63 REQUIRED COMPONENTS serialization)

add_executable(mpidemo-random main.cpp)

target_link_libraries(mpidemo-random
  PRIVATE
    ${MPI_C_LIBRARIES}
    Boost::serialization
)
