## A demo project of the MPI implementation in Manager / Worker mode

### How MPI program runs
```
/* Part I: this part will be executed by all processes. */
char buf[256];
int world_rank, world_size;

/* Part II: From MPI_Init to MPI_Finalize */
/* Initialize the infrastructure necessary for communication */
MPI_Init(&argc, &argv);

/* Identify this process */
MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

/* Find out how many total processes are active */
MPI_Comm_size(MPI_COMM_WORLD, &world_size);

/* Sending and Recieving */
if(world_rank == 0) {
    // actions for manager
    managerprocessor(world_size, world_rank);
} else {
    // actions for workers
    workerprocessor(world_size, world_rank);
}

/* Tear down the communication infrastructure */
MPI_Finalize();
```

Program with MPI implementation should be run with `mpirun`, for example,
```
mpirun -n 8 -f host_file ./name-of-excutable
```
`mpirun` would make several copies of `./name-of-excutable` independently, and run each copy of `./name-of-excutable` as a seperate process. 

* In Part I, the code is bound to be executed independently, where utilities of MPI are not available. 
* In Part II, the utilities of MPI are now available, and identification infomation for each process can be retrieved by the utilities.

In Part II, the behaviors of different processors can be controlled according to the information of each processors. For example, `world_rank` is the identification number of the processes, and it is designed that: for process of `world_rank == 0`, only the `managerprocessor` will be executed; for process of `world_rank != 0`, only the `workerprocessor` will be executed. 

As it is mentioned, `mpirun` would make copies of the executable. If `8` processors have been assigned for the task, then `8` copies of the executable will be produced and distributed to `8` different processors. In the case above, only one processor will take the task to procceed `managerprocessor`, while the other `7` will execute `workerprocessor`.

As a result, `mpirun` forces program with MPI implementation to run in a parallel mode. And one of the challenging issues is that: each processor runs indepedently with sharing the memories. This is definitely different the program with `thread` implementation, where the program is runing by a single process, whose memories are shared by all the threads created by the programs. In order to complete the parallel computation task, communication between different proccesors of program with MPI implementation is dispensable.

### Communication between different processors
Different processors can communicate with each other by `sending` and `recieving` messages. There are to modes for the MPI utilities: blocking and non-blocking, when handling with messages. Two basic functions are usefully, i.e., `MPI_Send` and `MPI_Recv`. Several built-in types can be transported using these two functions. ([learn more](http://mpitutorial.com/tutorials/mpi-send-and-receive/)) One of the challenges in communication of MPI program is to send/recieve data dynamically, i.e., bytes of char without specific length. Here comes with one of the demo that dynamically sends and recieves string.

*Note* Dynamicaly sending data is important for the case that custom data type is to be handled. One of the feasible solution is serializing the custom data type as string in the sender and deserializing the string as custom data type in the reciever. One of the Demo in this project uses `boost::serialization` library to do this. 

```
char * string2chararr(std::string src) {
    int length = src.length();
    char* temp = (char *) malloc(length+1);
    strcpy(temp, src.c_str());
    temp[length] = '\0';
    return temp;
}

void managerproccessor(int world_size, int world_rank) {
    for(int workerindex=1; workerindex<world_size; workerindex++) {
        int flag = 1;
        MPI_Send(&flag, 1, MPI_INT, workerindex, 0, MPI_COMM_WORLD);
        std::string task_str = "task from manager";
        // MPI has no implementation to send string directly.
        char *temp = string2chararr(task_str);
        MPI_Send(temp, task_str.length()+1, MPI_CHAR, workerindex, 0, MPI_COMM_WORLD);
    }
}
void workerproccessor(int world_size, int world_rank) {

    int flag = -1;
    MPI_Recv(&flag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    MPI_Status status;
    // Probe for an incoming message from process zero
    MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

    // When probe returns, the status object has the size and other
    // attributes of the incoming message. Get the message size
    int number_amount;
    MPI_Get_count(&status, MPI_CHAR, &number_amount);

    // Allocate a buffer to hold the incoming numbers
    char* number_buf = (char*)malloc(sizeof(char) * number_amount);

    MPI_Status status_buf;
    // Now receive the message with the allocated buffer
    MPI_Recv(number_buf, number_amount, MPI_CHAR, 0, 0,
            MPI_COMM_WORLD, &status_buf);

    std::string task_str = std::string(number_buf);
    free(number_buf);
}

```

### Manager and Worker mode
For a big task, the manager can divide the task into small tasks, and distribute these parts to some workers. Computation will be executed by the workers, while the manager is responsible for distributing tasks and collecting the results. Generally speaking, the number of small tasks is (much) larger than the number of workers. And thus, the workers might have to be run repeatedly, until all the tasks in the working queue have been consumed. A paradigm for implementing such a design can be shown with the following pseudo code,
```
void managerproccessor(int world_size, int world_rank) {
    std::queue<int> _Tasks;
    // 1. Populate the queue of the tasks

    // produces seeds for workers
    for(int workerindex=1; workerindex<world_size; workerindex++) {
        // 1. Send flag to worker to inform them that task is available
        // 2. Send the index of the tasks
        // 3. Send the content of a task to worker
    }

    while(true) {
        // 1. Recieve the index of the task
        // Note: MPI_ANY_SOURCE is the key to the manager / worker mode
        MPI_Status status_any;
        int taskindex = -1;
        MPI_Recv(&taskindex, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status_any);
        int sender = status_any.MPI_SOURCE;
        // 2. Recieve the result of the task
        // 3. Assign new task to the worker
        if(_Tasks.empty()) {
            // 1. Send Terminated signal to the worker
        } else {
            // 1. Send continue flag to worker  
            // 2. Dequeue from the queue of task, and Send it to the worker
        }
    }
}
void workerproccessor(int world_size, int world_rank) {
    
   while(true) {
       // 1. Recieve the flag to decide whether the worker should be terminated
       // 2. Recieve the index of the task
       // 3. Recieve the content of the task
       // 4. Computation
       /* Implementation of the computation processes */
       // 5. Send the index of the task to manager
       // 6. Send the result to the manager
   }

}
```

Recieving message from any source is the key to implement a Manager / Worker MPI program when using the blocking mode of MPI. In the Manager processor, some seeds of tasks should be created, when the tasks are sent to the workers. Then Manager goes into a loop, which would wait for results from any of the workers. Once a result is recieved, the manager would query the queue of task to figure out whether there are more tasked to be executed. If it is true, a task would be dequeued from the task queue and sent to the same worker. 

More details are available in the demo codes of this project.

### More
Non-Blocking mode.