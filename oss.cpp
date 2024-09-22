#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

struct Clock
{
    int seconds;
    int nanoseconds;
};

const int SH_KEY = 74821;

void signal_handler(int sig)
{
    std::cerr << "Timeout... terminating..." << std::endl;
    // code to send kill signal to all children based on their PIDs in process table

    // code to free up shared memory
    int shmid = shmget(SH_KEY, sizeof(Clock), 0);
    if (shmid != -1)
    {
        shmctl(shmid, IPC_RMID, nullptr);
    }
    exit(1);
}

void increment_clock(Clock *shared_clock)
{
    shared_clock -> nanoseconds += 500000;
    //increment seconds if nanoseconds = second
    if (shared_clock -> nanoseconds >= 1000000000)
    {
        shared_clock -> nanoseconds -= 1000000000;
        shared_clock -> seconds++;
    }
}

int main(int argc, char* argv[])
{
    //set up alarm
    signal(SIGALRM, signal_handler);
    alarm(60);

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
                    std::cout << "-t: set time limit for children\n" ;
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

    //create shared mem: 0644 r/w to owner
    int shmid = shmget(SH_KEY, sizeof(Clock), 0644 | IPC_CREAT);
    if (shmid == -1)
    {
        std::cerr << "Error: Shared memory get failed" << std::endl;
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

    int activeChildren = 0;

    for (int i = 0; i < numChildren; i++)
    {
        if (activeChildren >= numSim)
        {
            wait(nullptr);
            activeChildren--;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
           std::cerr << "Error: fork issue." << std::endl;
           return 1;
        }
        else if (pid == 0)
        {
            execl("./worker", "worker", std::to_string(timeLim).c_str(), std::to_string(timeLim).c_str(), nullptr);
            std::cerr << "Error: execl failed" << std::endl;
            return 1;
        }
        else
        {
            //parent process
            activeChildren++;
        }
        increment_clock(shared_clock);
    }

    shmdt(shared_clock);
    shmctl(shmid, IPC_RMID, nullptr);
    return 0;
}
