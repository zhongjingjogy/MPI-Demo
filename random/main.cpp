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
#include <cstdlib>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <vector>
#include <thread>
#include <chrono>

namespace boost {
namespace serialization {

template<class Archive>
void serialize(Archive & ar, std::vector<double> & info, const unsigned int version) {
    int s = info.size();
    for(int i=0; i<s; i++) {
        ar & info[i];
    }
}
}
}

void MakeArr(std::vector<double> &src, int size, double value) {
    src.resize(size);
    for(int i=0; i<size; i++) {
        src[i] = value;
    }
}

char * string2chararr(std::string src) {
    int length = src.length();
    char* temp = (char *) malloc(length+1);
    strcpy(temp, src.c_str());
    temp[length] = '\0';
    return temp;
}

class Manager {
    std::queue<int> _Tasks;
    int FinishedCount;
    int TaskCount;
    std::map<int, std::string> _Result;
    bool ServingOn;
    
public:

    void Init();
    void Serving(int);
    void CleanUp();

    ~Manager() {
    }
};

void Manager::Init() {
   
    int total = 1000;
    for(int i=0; i<total; i++) {
        _Tasks.push(i);
    }

    TaskCount = total;
    ServingOn = true;
    FinishedCount = 0;
}

void Manager::CleanUp() {
    for(auto each: _Result) {

        std::stringstream iss;
        iss << each.second;
        boost::archive::text_iarchive ia(iss);
        std::vector<double> src(2);
        ia & src;
        std::cout << each.first << ": ";
        for(auto fach: src) {
            std::cout << fach << " ";
        }
        std::cout << std::endl;
    }
}

void Manager::Serving(int world_size) {
    std::cout << "Serving thread in manager process is activated!\n";
    // A loop that query the worker sequence
    // if worker is available then new task will be assigned to the worker
    for(int workerindex=1; workerindex<world_size; workerindex++) {

        if(_Tasks.empty()) {
            int flag = 0;
            MPI_Send(&flag, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);
            continue;
        }

        // query a task
        int taskindex = 0;
        taskindex = _Tasks.front();
        _Tasks.pop();

        std::cout << "try to assign task: " << taskindex << ", to: " << workerindex << " \n";
        int flag = 1;
        MPI_Send(&flag, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);
        std::cout << "working flag : " << taskindex << " is sent to: " << workerindex << " \n";
        MPI_Send(&taskindex, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);
        std::cout << "taskindex : " << taskindex << " is sent to: " << workerindex << " \n";
 
        // update the status of the counters and working queue
    }

    while(ServingOn) {
        MPI_Status status_any;
        int taskindex = -1;
        MPI_Recv( &taskindex, 1, MPI_INT, MPI_ANY_SOURCE, 1,
                  MPI_COMM_WORLD, &status_any);
        int sender = status_any.MPI_SOURCE;
        // std::cout << "Status of any source in listening: " << status_any.MPI_ERROR << "\n";
        
        MPI_Status status;
        // Probe for an incoming message from process zero
        MPI_Probe(sender, 1, MPI_COMM_WORLD, &status);
        // std::cout << "status in listening: " << status.MPI_SOURCE << ", " << status.MPI_TAG << ", " << status.MPI_ERROR << "\n";

        // When probe returns, the status object has the size and other
        // attributes of the incoming message. Get the message size
        int number_amount;
        MPI_Get_count(&status, MPI_CHAR, &number_amount);
        // std::cout << "In listening, size to be recieved is: " << number_amount << "\n";

        // Allocate a buffer to hold the incoming numbers
        char* number_buf = (char*)malloc(sizeof(char) * number_amount);
        
        MPI_Status status_buf;
        // Now receive the message with the allocated buffer
        MPI_Recv(number_buf, number_amount, MPI_CHAR, sender, 1,
                MPI_COMM_WORLD, &status_buf);

        // std::cout << "Status of buf in listening: " << status_buf.MPI_ERROR << "\n";
 
        std::string json_str = std::string(number_buf);
        free(number_buf);
        // std::cout << "result is recieved in manager: " << json_str << "\n";

        _Result[taskindex] = json_str;
    
        FinishedCount++;

        if(!_Tasks.empty()) {
            int workerindex = sender;
            // query a task
            int taskindex = 0;
            taskindex = _Tasks.front();
            _Tasks.pop();

            // std::cout << "try to assign task: " << taskindex << ", to: " << workerindex << " \n";
            int flag = 1;
            MPI_Send(&flag, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);
            // std::cout << "working flag : " << taskindex << " is sent to: " << workerindex << " \n";
            MPI_Send(&taskindex, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);

        } else {
            int flag = 0;
            MPI_Send(&flag, 1, MPI_INT, sender, 0, MPI_COMM_WORLD);
            // std::cout << "working flag : " << taskindex << " is sent to: " << sender << " \n";
        }

        if(FinishedCount == TaskCount) {
            ServingOn = false;
        }
    }

}

int managerprocess(int world_size) {
    std::cout << "Manager process is activated!\n";
    Manager *m = new Manager;
    m->Init();
    std::cout << "Manager is initialized!\n";

    m->Serving(world_size);

    m->CleanUp();

    delete m;

    return 0;
}

int workerprocess(int world_rank, int world_size) {
    int Flag = 1;
    while(Flag) {
        std::cout << "Waiting for task in rank: " << world_rank << "\n";
        MPI_Recv(&Flag, 1, MPI_INT, 0, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(Flag == 0) {
            break;
        }
        std::cout << "process rank: " << world_rank << " is to procceed\n";
        // recieve task index
        int taskindex = 0;
        MPI_Recv(&taskindex, 1, MPI_INT, 0, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Taskindex: " << taskindex << " is recieved\n";
        // recieve the task
        
        // send the task to the worker
        std::vector<double> src;
        MakeArr(src, 2, 1.0);

        for(int i=0; i<2; i++) {
            src[i] = double(rand()) / RAND_MAX;
        }

        std::ostringstream oss;

        boost::archive::text_oarchive oa(oss);
        oa & src;
        std::string tstr = oss.str();
        int length = tstr.length()+1;
        char* temp = string2chararr(tstr);

        // processing on the task
 
        // std::this_thread::sleep_for (std::chrono::seconds(2));
        // std::cout << "result are sent to manager: " << result << "\n";

        // send the task index to the manager process
        MPI_Send(&taskindex, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

        // the length of char to be sent should be the same as that of char*
        // send the result to the manager process
        MPI_Send(temp, length, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        free(temp);
    }
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
    // seed for generating the random number should be executed only once.
    // the seeds of different processes should differ from each other.
    std::srand(world_rank);

    // asscess the name of the current process
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // print the information of the current process
    printf("Running MPI codes from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);

    if(world_rank == 0) {
        managerprocess(world_size);
    } else {
        workerprocess(world_rank, world_size);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
    return 0;
}