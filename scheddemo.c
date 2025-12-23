// scheddemo.c - Round Robin Scheduling Demo for xv6
// Usage: scheddemo [nprocs]
// Demonstrates process scheduling with timing metrics

#include "types.h"
#include "stat.h"
#include "user.h"
#include "sysinfo.h"

#define MAX_PROCS 6
#define TIME_QUANTUM 10

struct procstat {
  int pid;
  int at, bt, ct, tat, wt;
};

struct procstat stats[MAX_PROCS];
int start_time;

void
dowork(int ticks)
{
  int target = uptime() + ticks;
  volatile int x = 0;
  while(uptime() < target)
    x++;
}

int
main(int argc, char *argv[])
{
  int i, pid, nprocs = 4;
  int burst[] = {15, 10, 20, 12, 18, 8};
  
  if(argc > 1) {
    nprocs = atoi(argv[1]);
    if(nprocs < 2) nprocs = 2;
    if(nprocs > MAX_PROCS) nprocs = MAX_PROCS;
  }
  
  printf(1, "\n");
  printf(1, "================================================\n");
  printf(1, "      XV6 Round Robin Scheduling Demo\n");
  printf(1, "================================================\n\n");
  printf(1, "Time Quantum: ~%d ticks\n", TIME_QUANTUM);
  printf(1, "Processes: %d\n\n", nprocs);
  
  printf(1, "Planned Burst Times:\n");
  for(i = 0; i < nprocs; i++)
    printf(1, "  P%d: %d ticks\n", i, burst[i]);
  
  start_time = uptime();
  printf(1, "\n--- Execution ---\n\n");
  
  for(i = 0; i < nprocs; i++) {
    stats[i].at = uptime() - start_time;
    stats[i].bt = burst[i];
    
    pid = fork();
    if(pid < 0) {
      printf(1, "fork failed\n");
      exit();
    }
    
    if(pid == 0) {
      int id = i, bt = burst[i], st = uptime();
      printf(1, "[P%d] Start at %d\n", id, st - start_time);
      dowork(bt);
      printf(1, "[P%d] End at %d\n", id, uptime() - start_time);
      exit();
    }
    stats[i].pid = pid;
  }
  
  for(i = 0; i < nprocs; i++) {
    int wpid = wait();
    int ct = uptime() - start_time;
    int j;
    for(j = 0; j < nprocs; j++) {
      if(stats[j].pid == wpid) {
        stats[j].ct = ct;
        stats[j].tat = ct - stats[j].at;
        stats[j].wt = stats[j].tat - stats[j].bt;
        if(stats[j].wt < 0) stats[j].wt = 0;
        break;
      }
    }
  }
  
  printf(1, "\n--- Statistics ---\n\n");
  printf(1, "Proc  AT   BT   CT   TAT  WT\n");
  printf(1, "----  ---  ---  ---  ---  ---\n");
  
  int tot_tat = 0, tot_wt = 0;
  for(i = 0; i < nprocs; i++) {
    printf(1, "P%d    %d    %d    %d    %d    %d\n",
           i, stats[i].at, stats[i].bt, stats[i].ct,
           stats[i].tat, stats[i].wt);
    tot_tat += stats[i].tat;
    tot_wt += stats[i].wt;
  }
  
  printf(1, "\nAvg TAT: %d  Avg WT: %d\n", tot_tat/nprocs, tot_wt/nprocs);
  printf(1, "\n");
  printf(1, "Legend:\n");
  printf(1, "  AT=Arrival  BT=Burst  CT=Completion\n");
  printf(1, "  TAT=Turnaround(CT-AT)  WT=Wait(TAT-BT)\n");
  printf(1, "================================================\n\n");
  
  exit();
}
