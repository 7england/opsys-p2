*********************************************************
Author: S. England
Date: 9/25/24
Project 2 - OpSys
*********************************************************

Invocation:
./oss
        -n: number of children/workers
        -s: number of simultaneous children
        -t: upper limit for random time (1-t) in seconds
        -i: interval between launching child processes (ms)
        -h: help

Issues:
        The first child does not automatically launch as it waits i ms before launching.
        The process table does not decrement current processes if other processes are terminated.
        Sometimes the output interrupts itself between oss and worker, or multiple workers.

Git link:
  https://github.com/7england/opsys-p2

Git log:
  oss.cpp: fixed intervals issue to launch children every i ms, changed increment to match real time more closely, debugging output, fixed issue with pcb table S. England 16 minutes ago
  oss.cpp: implemented intervals (replaced hard code with input) debugging: issue with int overflow- printing table correctly, simultaneous workers S. England Yesterday 8:00 PM
  oss.cpp: add random numbers S. England Yesterday 11:01 AM
  worker.cpp: updated loop to print info S. England 9/23/2024 1:56 PM
  oss.cpp: updated signal handler to kill children based on pcb table S. England 9/23/2024 10:59 AM
  oss.cpp: added print process table function S. England 9/23/2024 10:13 AM
  oss.cpp: added PCB table, added loops for forking (max simul), fixed exec call, increment clock S. England 9/22/2024 9:12 PM
  oss.cpp added signal to shut down after 60 seconds, fixed issue with shmget using SH_KEY, set up maximum number of children at a time like in p1, forking child processes, increment clock. worker.cpp accesses shared memory, outputs the necessary info. S. England 9/21/2024 8:12 PM
  added menu from p1, updated t and added i options S. England 9/17/2024 10:33 PM
  -set up files -copy old makefile and change user file to worker -implement shared memory in oss.cpp -make sure child can access shared memory -add clock struct S. England 9/16/2024 6:20 PM
