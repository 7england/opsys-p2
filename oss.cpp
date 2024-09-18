#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct Clock
{
    int seconds;
    int nanoseconds;
};

int main(int argc, char* argv[])
{
    int opt;
    int numChildren = 0;
    int numSim = 0;
    int timeLim = 0;
    int interval = 0;

    while((opt = getopt(argc, argv, ":hn:s:t:i:")) != -1) //set optional args
        {
            switch(opt)
            {
                //help menu
                case 'h':
                    std::cout << "Help menu:\n" ;
                    std::cout << "**********************\n" ;
                    std::cout << "-h: display help menu\n" ;
                    std::cout << "-n: set number of child processes\n" ;
                    std::cout << "-s: set number of simultaneous children\n" ;
                    std::cout << "-t: set iterations\n" ;
                    std::cout << "-i: set interval in ms between launching children\n" ;
                    std::cout << "**********************\n" ;
                    std::cout << "Example invocation: \n" ;
                    std::cout << "./oss -n 5 -s 3 -t 7 -i 100\n" ;
                    std::cout << "Example will launch 5 child processes, with time limit between 1s and 7s,";
                    std::cout << "\nwith a time delay between new children of 100 ms\n" ;
                    std::cout << "\nand never allow more than 3 child processes to run simultaneously.\n" ;
                    return 0;
                case 'n':
                    numChildren = atoi(optarg); //assign arg value to numChildren
                    break;
                case 's':
                    numSim = atoi(optarg); //assign arg value to numSim
                    break;
                case 't': //assign arg value to iter
                    timeLim = atoi(optarg);
                    break;
                case 'i':
                    interval = atoi(optarg);
                    break;
                default:
                    std::cerr << "Please choose an option!\n" ;
                    std::cout << "Example invocation: \n" ;
                    std::cout << "./oss -n 5 -s 3 -t 7 -i 100\n" ;
                    return 1;
            }
        }

    //create shared mem key
    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0644 | IPC_CREAT);

    if (shmid == -1)
    {
        std::cerr << "Error: Shared memory get failed\n";
        return 1;
    }

    //attach shared mem
    Clock *shared_clock = static_cast<Clock*>(shmat(shmid, nullptr, 0));
        if (shared_clock == (void*)-1)
        {
            std::cerr << "Error: shmat" << std::endl;
            return 1;
        }

    shared_clock->seconds = 0;
    shared_clock->nanoseconds = 0;

    pid_t pid = fork();

    if (pid < 0)
    {
        std::cerr << "Error: fork issue.\n";
        return 1;
    }
    else if (pid == 0)
    {
        //child process
        int *child_clock = static_cast<int*>(shmat(shmid, nullptr, 0));
        if (child_clock == (void*)-1)
        {
            std::cerr << "Child: Error: shmat\n";
            return 1;
        }
        //simulate clock here
        //detach from shared memory
        if (shmdt(child_clock) == -1)
        {
            std::cerr << "Child: error: shmdt" << std::endl;
            return 1;
        }
    }
    else
    {
        //parent process
        wait(nullptr);
        //detach from shared mem
        if (shmdt(shared_clock) == -1)
        {
            std::cerr << "Parent: error: shmdt" << std::endl;
            return 1;
        }
        //remove shared mem
        if (shmctl(shmid, IPC_RMID, nullptr) == -1)
        {
            std::cerr << "Parent: error: shmctl" << std::endl;
            return 1;
        }
    }

    return 0;
}
