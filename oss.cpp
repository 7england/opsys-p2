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

int main()
{
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
