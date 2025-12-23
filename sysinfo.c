// sysinfo.c - User-space kernel status monitoring tool
// Usage: sysinfo [-w] [-p] [-m] [-s] [-a]
//   -w: Watch mode (continuous update)
//   -p: Show process list
//   -m: Show memory info
//   -s: Show syscall stats
//   -a: Show all information
//   (no args): Show summary

#include "types.h"
#include "stat.h"
#include "user.h"
#include "sysinfo.h"

// State names for display
char *state_names[] = {
  "UNUSED  ",
  "EMBRYO  ",
  "SLEEPING",
  "RUNNABLE",
  "RUNNING ",
  "ZOMBIE  "
};

// System call names
char *syscall_names[] = {
  "",        "fork",   "exit",   "wait",   "pipe",
  "read",    "kill",   "exec",   "fstat",  "chdir",
  "dup",     "getpid", "sbrk",   "sleep",  "uptime",
  "open",    "write",  "mknod",  "unlink", "link",
  "mkdir",   "close"
};

void
print_header(void)
{
  printf(1, "\n============== XV6 KERNEL STATUS MONITOR ==============\n\n");
}

void
print_sysinfo(struct sysinfo *info)
{
  printf(1, "--- SYSTEM ---\n");
  printf(1, "Uptime: %d ticks (%d seconds)\n", info->uptime, info->uptime / 100);
  printf(1, "CPUs: %d\n", info->ncpu);
  printf(1, "\n");
  
  printf(1, "--- MEMORY ---\n");
  printf(1, "Page size:   %d bytes (4 KB)\n", info->mem.page_size);
  printf(1, "Total pages: %d (%d KB)\n", info->mem.total_pages, info->mem.total_pages * 4);
  printf(1, "Free pages:  %d (%d KB)\n", info->mem.free_pages, info->mem.free_pages * 4);
  printf(1, "Used pages:  %d (%d KB)\n", info->mem.used_pages, info->mem.used_pages * 4);
  
  // Calculate percentage
  int pct = (info->mem.used_pages * 100) / info->mem.total_pages;
  printf(1, "Memory usage: %d%%\n", pct);
  printf(1, "\n");
  
  printf(1, "--- PROCESS QUEUES ---\n");
  printf(1, "Total active processes: %d / 64\n", info->procq.total_count);
  printf(1, "  EMBRYO   (being created): %d\n", info->procq.embryo_count);
  printf(1, "  RUNNABLE (ready queue):   %d\n", info->procq.runnable_count);
  printf(1, "  RUNNING  (on CPU):        %d\n", info->procq.running_count);
  printf(1, "  SLEEPING (blocked):       %d\n", info->procq.sleeping_count);
  printf(1, "  ZOMBIE   (waiting reap):  %d\n", info->procq.zombie_count);
  printf(1, "\n");
}

void
print_proclist(void)
{
  struct procinfo procs[64];
  int count, i;
  char *state;
  
  count = getprocinfo(procs, 64);
  
  printf(1, "--- PROCESS LIST ---\n");
  printf(1, "PID   PPID  STATE     SIZE(KB)  KILLED  NAME\n");
  printf(1, "----  ----  --------  --------  ------  ----------------\n");
  
  for(i = 0; i < count; i++) {
    state = (procs[i].state >= 0 && procs[i].state <= 5) 
                  ? state_names[procs[i].state] : "???     ";
    printf(1, "%d     %d     %s  %d        %s      %s\n",
           procs[i].pid,
           procs[i].ppid,
           state,
           procs[i].sz / 1024,
           procs[i].killed ? "YES" : "NO",
           procs[i].name);
  }
  printf(1, "\n");
}

void
print_meminfo(void)
{
  struct meminfo mem;
  
  getmeminfo(&mem);
  
  printf(1, "--- MEMORY DETAILS ---\n");
  printf(1, "Page size:     %d bytes\n", mem.page_size);
  printf(1, "Kernel end:    0x%x\n", mem.kernel_end);
  printf(1, "Total pages:   %d\n", mem.total_pages);
  printf(1, "Free pages:    %d\n", mem.free_pages);
  printf(1, "Used pages:    %d\n", mem.used_pages);
  printf(1, "\n");
  
  // Visual memory bar
  printf(1, "Memory: [");
  int bar_width = 40;
  int used_bars = (mem.used_pages * bar_width) / mem.total_pages;
  int i;
  for(i = 0; i < bar_width; i++) {
    if(i < used_bars)
      printf(1, "#");
    else
      printf(1, "-");
  }
  printf(1, "] %d%%\n", (mem.used_pages * 100) / mem.total_pages);
  printf(1, "\n");
}

void
print_syscallstats(void)
{
  struct syscallstats stats;
  int i;
  
  getsyscallstats(&stats);
  
  printf(1, "--- SYSTEM CALL STATISTICS ---\n");
  printf(1, "Total system calls: %d\n\n", stats.total_calls);
  
  printf(1, "Syscall       Count\n");
  printf(1, "------------  ---------\n");
  
  for(i = 1; i <= 21; i++) {
    if(stats.calls[i] > 0) {
      printf(1, "%-12s  %d\n", syscall_names[i], stats.calls[i]);
    }
  }
  printf(1, "\n");
}

void
print_mini_status(struct sysinfo *info)
{
  printf(1, "[%ds] Mem: %d/%dKB | Procs: R:%d S:%d Z:%d\n",
         info->uptime / 100,
         info->mem.used_pages * 4, 
         info->mem.total_pages * 4,
         info->procq.runnable_count, 
         info->procq.sleeping_count, 
         info->procq.zombie_count);
}

// Real-time compact top-like display
void
print_top(void)
{
  struct sysinfo info;
  struct procinfo procs[64];
  int count, i;
  char *st;
  
  if(getsysinfo(&info) < 0)
    return;
  
  count = getprocinfo(procs, 64);
  
  // Compact header
  printf(1, "=== XV6 TOP [%ds] CPU:%d Mem:%d%% ===\n",
         info.uptime / 100,
         info.ncpu,
         (info.mem.used_pages * 100) / info.mem.total_pages);
  
  // Process queue summary
  printf(1, "Procs: %d | RUN:%d READY:%d SLEEP:%d ZOMBIE:%d\n",
         info.procq.total_count,
         info.procq.running_count,
         info.procq.runnable_count,
         info.procq.sleeping_count,
         info.procq.zombie_count);
  
  // Memory bar
  printf(1, "Mem: [");
  int bar_width = 20;
  int used_bars = (info.mem.used_pages * bar_width) / info.mem.total_pages;
  for(i = 0; i < bar_width; i++) {
    if(i < used_bars)
      printf(1, "#");
    else
      printf(1, "-");
  }
  printf(1, "] %dKB/%dKB\n",
         info.mem.used_pages * 4,
         info.mem.total_pages * 4);
  
  // Process table header
  printf(1, "\nPID  STATE    MEM(KB)  NAME\n");
  printf(1, "---- -------- -------- ----------------\n");
  
  // Show active processes (limit to fit screen)
  for(i = 0; i < count && i < 10; i++) {
    switch(procs[i].state) {
    case 1: st = "EMBRYO  "; break;
    case 2: st = "SLEEP   "; break;
    case 3: st = "READY   "; break;
    case 4: st = "RUNNING "; break;
    case 5: st = "ZOMBIE  "; break;
    default: st = "???     ";
    }
    printf(1, "%d    %s %d        %s\n",
           procs[i].pid,
           st,
           procs[i].sz / 1024,
           procs[i].name);
  }
  printf(1, "------------------------------------\n");
}

int
main(int argc, char *argv[])
{
  struct sysinfo info;
  int watch_mode = 0;
  int top_mode = 0;
  int show_procs = 0;
  int show_mem = 0;
  int show_syscalls = 0;
  int show_all = 0;
  int i;
  int interval = 100;  // default 1 second for top mode
  
  // Parse arguments
  for(i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      switch(argv[i][1]) {
      case 'w':
        watch_mode = 1;
        break;
      case 't':
        top_mode = 1;
        break;
      case 'p':
        show_procs = 1;
        break;
      case 'm':
        show_mem = 1;
        break;
      case 's':
        show_syscalls = 1;
        break;
      case 'a':
        show_all = 1;
        break;
      case 'h':
        printf(1, "Usage: sysinfo [-w] [-t] [-p] [-m] [-s] [-a] [-h]\n");
        printf(1, "  -t: Top mode (compact real-time, updates every 1s)\n");
        printf(1, "  -w: Watch mode (full info, updates every 2s)\n");
        printf(1, "  -p: Show process list\n");
        printf(1, "  -m: Show detailed memory info\n");
        printf(1, "  -s: Show syscall statistics\n");
        printf(1, "  -a: Show all information\n");
        printf(1, "  -h: Show this help\n");
        printf(1, "\nKeyboard shortcuts in console:\n");
        printf(1, "  Ctrl+P: Quick process dump\n");
        printf(1, "  Ctrl+S: Full kernel status display\n");
        exit();
      }
    }
  }
  
  // Top mode - compact real-time display
  if(top_mode) {
    while(1) {
      print_top();
      sleep(interval);
      // "Clear" by printing newlines
      for(i = 0; i < 20; i++)
        printf(1, "\n");
    }
  }
  
  if(show_all) {
    show_procs = show_mem = show_syscalls = 1;
  }
  
  do {
    if(getsysinfo(&info) < 0) {
      printf(2, "sysinfo: failed to get system info\n");
      exit();
    }
    
    print_header();
    print_sysinfo(&info);
    
    if(show_procs) {
      print_proclist();
    }
    
    if(show_mem) {
      print_meminfo();
    }
    
    if(show_syscalls) {
      print_syscallstats();
    }
    
    printf(1, "======================================================\n");
    printf(1, "Tip: Press Ctrl+S in console for instant kernel status\n\n");
    
    if(watch_mode) {
      printf(1, "Refreshing in 2 seconds... (Ctrl+C to stop)\n");
      sleep(200);  // 2 seconds (100 ticks = 1 second)
      
      // Clear screen effect (print many newlines)
      for(i = 0; i < 25; i++) {
        printf(1, "\n");
      }
    }
  } while(watch_mode);
  
  exit();
}
