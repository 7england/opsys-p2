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
#include <iomanip>

struct PCB
{
    int occupied; // either true or false
    pid_t pid; // process id of this child
    int startSeconds; // time when it was forked
    int startNano; // time when it was forked
};

struct Clock
{
    int seconds;
    int nanoseconds;
};

const int SH_KEY = 74821;
const int MAX_PROCESSES = 18; //as specified in class no more than 18 at a time

PCB pcb_table[MAX_PROCESSES];

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

void print_process_table(PCB pcb_table[], Clock* shared_clock)
{
    std::cout << "OSS PID: " << getpid() << std::endl;
    std::cout << "SysClockS: " << shared_clock -> seconds << std::endl;
    std::cout << "SysCLockNano: " << shared_clock -> nanoseconds << std::endl;
    std::cout << "Process Table:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::setw(10) << "Entry" <<
    std::setw(10) << "Occupied" <<
    std::setw(10) << "PID" <<
    std::setw(10) << "StartS" <<
    std::setw(10) << "StartN" <<
    std::endl;

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        std::cout << std::setw(10) << i <<
        std::setw(10) << pcb_table[i].occupied <<
        std::setw(10) << pcb_table[i].pid <<
        std::setw(10) << pcb_table[i].startSeconds <<
        std::setw(10) << pcb_table[i].startNano <<
        std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
}

int main(int argc, char* argv[])
{
    //set up alarm
    signal(SIGALRM, signal_handler);
    alarm(60);

    int opt;
    int numChildren = 1;
    int numSim = 1;
    int timeLimSec = 1;
    int timeLimNano = 5000000;
    int interval = 100;

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
                    std::cout << "-t: set time limit for children in seconds\n" ;
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
                    timeLimSec = atoi(optarg);
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
    int launchedChildren = 0;

    while (launchedChildren < numChildren)
    {
        increment_clock(shared_clock);
        if (shared_clock -> nanoseconds % 500000 == 0)
        {
            //print_process_table(pcb_table, shared_clock); <- write function
        }
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);

        if (pid > 0)
        {
            for (int i = 0; i < MAX_PROCESSES; i++)
            {
                if (pcb_table[i].pid == pid)
                {
                    //line for debugging
                    std::cout << "Process " << pid << " has terminated." << std::endl;
                    pcb_table[i].occupied = 0;
                    activeChildren--;
                    break;
                }
            }
        }
        if (activeChildren < numSim)
        {
            for (int i = 0; i < MAX_PROCESSES; i++)
            {
                if (!pcb_table[i].occupied)
                {
                    pid_t new_pid = fork();

                    if (new_pid < 0)
                    {
                        std::cerr << "Error: fork issue." << std::endl;
                        return 1;
                    }
                    else if (new_pid ==0)
                    {
                        //child process
                        execl("./worker", "worker", std::to_string(timeLimSec).c_str(), std::to_string(timeLimNano).c_str(), nullptr);
                        std::cerr << "Error: execl failed" << std::endl;
                        return 1;
                    }
                    else
                    {
                        //parent process
                        pcb_table[i].occupied = 1;
                        pcb_table[i].pid = new_pid;
                        pcb_table[i].startSeconds = shared_clock -> seconds;
                        pcb_table[i].startNano = shared_clock -> nanoseconds;
                        activeChildren++;
                        launchedChildren++;
                        break;
                    }
                }
            }
        }
        increment_clock(shared_clock);
    }

    while (activeChildren > 0)
    {
        int status;
        pid_t pid = wait(&status);

        if (pid > 0)
        {
            for (int i = 0; i < MAX_PROCESSES; i++)
            {
                if (pcb_table[i].pid == pid)
                {
                    //for debugging
                    std::cout << "Process " << pid << " has terminated." << std::endl;
                    pcb_table[i].occupied = 0; //entry is free
                    activeChildren--;
                    break;
                }
            }
        }
    }

    shmdt(shared_clock);
    shmctl(shmid, IPC_RMID, nullptr);
    return 0;
}
