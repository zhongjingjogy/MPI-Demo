#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <mpi.h>
#include <queue>
#include <stdio.h>
#include <string.h>

#include <vector>
#include <thread>
#include <chrono>

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/serialization/map.hpp>

void Test(int world_rank) {
    std::string BaseSrc, CacheSrc;
    BaseSrc = "./";
    std::string src = "tests";
    boost::filesystem::path p{BaseSrc.c_str()};
    p /= src.c_str();
    CacheSrc = p.string();
    std::cout << "rank " << world_rank << ": " << "join path successfully\n";
    if (!boost::filesystem::exists(p.string())) {
        boost::filesystem::create_directory(p.string());
    }
    std::cout << "rank " << world_rank << ": " << "succeed to create folder successfully\n";
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // asscess the name of the current process
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // print the information of the current process
    printf("Running MPI codes from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    Test(world_rank);

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}