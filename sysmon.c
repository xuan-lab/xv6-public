// sysmon.c - Kernel Status Monitoring System
// Provides real-time kernel state monitoring functions

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sysinfo.h"
#include "x86.h"

// External references
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

extern char end[];  // First address after kernel (defined in kernel.ld)
extern int ncpu;
extern struct cpu cpus[NCPU];
extern uint ticks;
extern struct spinlock tickslock;

// System call statistics tracking
struct syscallstats syscall_stats;
static struct spinlock statslock;

// Initialize the monitoring system
void
sysmoninit(void)
{
  initlock(&statslock, "syscallstats");
  memset(&syscall_stats, 0, sizeof(syscall_stats));
}

// Record a system call (called from syscall.c)
void
record_syscall(int syscall_num)
{
  if(syscall_num > 0 && syscall_num < 30) {
    acquire(&statslock);
    syscall_stats.total_calls++;
    syscall_stats.calls[syscall_num]++;
    release(&statslock);
  }
}

// Count free memory pages (uses kalloc.c function)
int
count_free_pages(void)
{
  return kfreepages();
}

// Get memory information
void
getmeminfo(struct meminfo *info)
{
  info->page_size = PGSIZE;
  info->kernel_end = (uint)end;
  info->free_pages = count_free_pages();
  // PHYSTOP is the end of physical memory (224MB by default)
  info->total_pages = (PHYSTOP - (uint)end) / PGSIZE;
  info->used_pages = info->total_pages - info->free_pages;
}

// Get process queue statistics
void
getprocqueue(struct procqueue *pq)
{
  struct proc *p;
  
  memset(pq, 0, sizeof(*pq));
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    switch(p->state) {
    case UNUSED:
      pq->unused_count++;
      break;
    case EMBRYO:
      pq->embryo_count++;
      pq->total_count++;
      break;
    case SLEEPING:
      pq->sleeping_count++;
      pq->total_count++;
      break;
    case RUNNABLE:
      pq->runnable_count++;
      pq->total_count++;
      break;
    case RUNNING:
      pq->running_count++;
      pq->total_count++;
      break;
    case ZOMBIE:
      pq->zombie_count++;
      pq->total_count++;
      break;
    }
  }
  release(&ptable.lock);
}

// Get information about all processes
int
getprocinfo(struct procinfo *procs, int max)
{
  struct proc *p;
  int count = 0;
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC] && count < max; p++) {
    if(p->state != UNUSED) {
      procs[count].pid = p->pid;
      procs[count].ppid = p->parent ? p->parent->pid : 0;
      procs[count].state = p->state;
      procs[count].sz = p->sz;
      procs[count].chan = p->chan;
      procs[count].killed = p->killed;
      safestrcpy(procs[count].name, p->name, sizeof(procs[count].name));
      count++;
    }
  }
  release(&ptable.lock);
  
  return count;
}

// Get CPU information
int
getcpuinfo(struct cpuinfo *infos, int max)
{
  int i;
  struct cpu *c;
  
  for(i = 0; i < ncpu && i < max; i++) {
    c = &cpus[i];
    infos[i].cpuid = i;
    infos[i].apicid = c->apicid;
    if(c->proc) {
      infos[i].has_proc = 1;
      infos[i].proc_pid = c->proc->pid;
      safestrcpy(infos[i].proc_name, c->proc->name, sizeof(infos[i].proc_name));
    } else {
      infos[i].has_proc = 0;
      infos[i].proc_pid = 0;
      infos[i].proc_name[0] = '\0';
    }
  }
  
  return (ncpu < max) ? ncpu : max;
}

// Get complete system information
void
getsysinfo(struct sysinfo *info)
{
  acquire(&tickslock);
  info->uptime = ticks;
  release(&tickslock);
  
  getmeminfo(&info->mem);
  getprocqueue(&info->procq);
  info->ncpu = ncpu;
}

// Get system call statistics
void
getsyscallstats(struct syscallstats *stats)
{
  acquire(&statslock);
  memmove(stats, &syscall_stats, sizeof(*stats));
  release(&statslock);
}

// Print detailed kernel status to console (for debugging, triggered by Ctrl+S)
void
kernelstatus(void)
{
  struct meminfo mem;
  struct procqueue pq;
  struct proc *p;
  uint uptime_val;
  static char *states[] = {
    [UNUSED]    "UNUSED  ",
    [EMBRYO]    "EMBRYO  ",
    [SLEEPING]  "SLEEPING",
    [RUNNABLE]  "RUNNABLE",
    [RUNNING]   "RUNNING ",
    [ZOMBIE]    "ZOMBIE  "
  };
  
  acquire(&tickslock);
  uptime_val = ticks;
  release(&tickslock);
  
  getmeminfo(&mem);
  getprocqueue(&pq);
  
  cprintf("\n");
  cprintf("============== KERNEL STATUS MONITOR ==============\n");
  cprintf("\n");
  
  // System uptime
  cprintf("--- SYSTEM ---\n");
  cprintf("Uptime: %d ticks (%d seconds)\n", uptime_val, uptime_val / 100);
  cprintf("CPUs: %d\n", ncpu);
  cprintf("\n");
  
  // Memory status
  cprintf("--- MEMORY ---\n");
  cprintf("Page size:   %d bytes (4 KB)\n", mem.page_size);
  cprintf("Total pages: %d (%d KB)\n", mem.total_pages, mem.total_pages * 4);
  cprintf("Free pages:  %d (%d KB)\n", mem.free_pages, mem.free_pages * 4);
  cprintf("Used pages:  %d (%d KB)\n", mem.used_pages, mem.used_pages * 4);
  cprintf("Memory usage: %d%%\n", (mem.used_pages * 100) / mem.total_pages);
  cprintf("\n");
  
  // Process queue statistics
  cprintf("--- PROCESS QUEUES ---\n");
  cprintf("Total active processes: %d / %d\n", pq.total_count, NPROC);
  cprintf("  EMBRYO   (being created): %d\n", pq.embryo_count);
  cprintf("  RUNNABLE (ready queue):   %d\n", pq.runnable_count);
  cprintf("  RUNNING  (on CPU):        %d\n", pq.running_count);
  cprintf("  SLEEPING (blocked):       %d\n", pq.sleeping_count);
  cprintf("  ZOMBIE   (waiting reap):  %d\n", pq.zombie_count);
  cprintf("\n");
  
  // Process list with details
  cprintf("--- PROCESS LIST ---\n");
  cprintf("PID   PPID  STATE     SIZE(KB)  NAME\n");
  cprintf("----  ----  --------  --------  ----------------\n");
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state != UNUSED) {
      cprintf("%-4d  %-4d  %s  %-8d  %s", 
              p->pid,
              p->parent ? p->parent->pid : 0,
              (p->state >= 0 && p->state < NELEM(states) && states[p->state]) 
                ? states[p->state] : "???     ",
              p->sz / 1024,
              p->name);
      
      // Show sleep channel for sleeping processes
      if(p->state == SLEEPING && p->chan) {
        cprintf(" [chan: %p]", p->chan);
      }
      // Show killed flag
      if(p->killed) {
        cprintf(" [KILLED]");
      }
      cprintf("\n");
    }
  }
  release(&ptable.lock);
  
  // System call statistics
  cprintf("\n");
  cprintf("--- SYSCALL STATS ---\n");
  cprintf("Total system calls: %d\n", syscall_stats.total_calls);
  
  // Show top syscalls
  static char *syscall_names[] = {
    [1]  "fork",    [2]  "exit",    [3]  "wait",    [4]  "pipe",
    [5]  "read",    [6]  "kill",    [7]  "exec",    [8]  "fstat",
    [9]  "chdir",   [10] "dup",     [11] "getpid",  [12] "sbrk",
    [13] "sleep",   [14] "uptime",  [15] "open",    [16] "write",
    [17] "mknod",   [18] "unlink",  [19] "link",    [20] "mkdir",
    [21] "close"
  };
  
  acquire(&statslock);
  int i;
  for(i = 1; i <= 21; i++) {
    if(syscall_stats.calls[i] > 0) {
      cprintf("  %-8s: %d\n", syscall_names[i], syscall_stats.calls[i]);
    }
  }
  release(&statslock);
  
  cprintf("\n");
  cprintf("====================================================\n");
  cprintf("Press Ctrl+P for process dump, Ctrl+S for full status\n");
  cprintf("\n");
}

// Mini status display (shows just key metrics, for frequent updates)
void
ministatus(void)
{
  struct meminfo mem;
  struct procqueue pq;
  uint uptime_val;
  
  acquire(&tickslock);
  uptime_val = ticks;
  release(&tickslock);
  
  getmeminfo(&mem);
  getprocqueue(&pq);
  
  cprintf("[%ds] Mem: %d/%dKB | Procs: R:%d S:%d Z:%d | Syscalls: %d\n",
          uptime_val / 100,
          mem.used_pages * 4, mem.total_pages * 4,
          pq.runnable_count, pq.sleeping_count, pq.zombie_count,
          syscall_stats.total_calls);
}
