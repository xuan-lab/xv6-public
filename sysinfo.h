// sysinfo.h - Kernel Status Monitoring System
// This header defines structures for real-time kernel state monitoring

#ifndef _SYSINFO_H
#define _SYSINFO_H

#include "types.h"
#include "param.h"

// Process information structure for monitoring
struct procinfo {
  int pid;                     // Process ID
  int ppid;                    // Parent Process ID
  char name[16];               // Process name
  int state;                   // Process state (enum procstate)
  uint sz;                     // Memory size in bytes
  void *chan;                  // Sleep channel (if sleeping)
  int killed;                  // Killed flag
};

// Memory information structure
struct meminfo {
  uint total_pages;            // Total available pages
  uint free_pages;             // Free pages count
  uint used_pages;             // Used pages count
  uint page_size;              // Size of each page (4096 bytes)
  uint kernel_end;             // End of kernel in memory
};

// CPU information structure
struct cpuinfo {
  int cpuid;                   // CPU ID
  int apicid;                  // Local APIC ID
  int has_proc;                // Is running a process?
  int proc_pid;                // PID of running process (if any)
  char proc_name[16];          // Name of running process (if any)
};

// Process queue statistics
struct procqueue {
  int unused_count;            // UNUSED processes
  int embryo_count;            // EMBRYO processes
  int sleeping_count;          // SLEEPING processes
  int runnable_count;          // RUNNABLE processes (ready queue)
  int running_count;           // RUNNING processes
  int zombie_count;            // ZOMBIE processes
  int total_count;             // Total non-unused processes
};

// System call statistics
struct syscallstats {
  uint total_calls;            // Total system calls made
  uint calls[30];              // Per-syscall counts (indexed by syscall number)
};

// Complete system information structure
struct sysinfo {
  uint uptime;                 // System uptime in ticks
  struct meminfo mem;          // Memory information
  struct procqueue procq;      // Process queue statistics
  int ncpu;                    // Number of CPUs
};

// State name strings for display
#define STATE_UNUSED   0
#define STATE_EMBRYO   1
#define STATE_SLEEPING 2
#define STATE_RUNNABLE 3
#define STATE_RUNNING  4
#define STATE_ZOMBIE   5

#endif // _SYSINFO_H
