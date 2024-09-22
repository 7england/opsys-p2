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

const int SH_KEY = 74821;

int main(int argc, char *argv[])
{
    int maxSec = std::atoi(argv[1]);
    int maxNsec = std::atoi(argv[2]);

    //https://stackoverflow.com/questions/55833470/accessing-key-t-generated-by-ipc-private
    int shmid = shmget(SH_KEY, sizeof(Clock), 0666); //<-----
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

    int termSec = shared_clock -> seconds + maxSec;
    int termNsec = shared_clock -> nanoseconds + maxNsec;

    std::cout << "Worker PID: " << getpid() << " PPID: " << getppid() << std::endl;
    std::cout << "SysClockS: " << shared_clock -> seconds <<  "SysClockNano: " << shared_clock -> nanoseconds << std::endl;
    std::cout << "TermTimeS: " << termSec << "TermTimeNano: " << termNsec << std::endl;
    std::cout << "\n Starting.......\n\n" << std::endl;

    while (shared_clock -> seconds < termSec ||
    shared_clock -> seconds == termSec && shared_clock -> nanoseconds < termNsec)
    {

        //add loop here to check for reasonable
        //print info again
        std::cout << "Worker PID: " << getpid() << " PPID: " << getppid() << std::endl;
        std::cout << "SysClockS: " << shared_clock -> seconds <<  "SysClockNano: " << shared_clock -> nanoseconds << std::endl;
        std::cout << "TermTimeS: " << termSec << "TermTimeNano: " << termNsec << std::endl;
    }

    std::cout << "Worker PID: " << getpid() << " PPID: " << getppid() << std::endl;
    std::cout << "SysClockS: " << shared_clock -> seconds <<  "SysClockNano: " << shared_clock -> nanoseconds << std::endl;
    std::cout << "TermTimeS: " << termSec << "TermTimeNano: " << termNsec << std::endl;
    std::cout << "\n Terminating.......\n\n" << std::endl;

    if (shmdt(shared_clock) == -1)
    {
        std::cerr << "Worker: error: shmdt" << std::endl;
        return 1;
    }

    return 0;

}