// procmon.c - Process Monitor for xv6
// Usage: procmon [command [args...]]
// Without args: shows current processes
// With command: runs command and monitors it

#include "types.h"
#include "stat.h"
#include "user.h"
#include "sysinfo.h"

char *states[] = {"----", "EMBR", "SLEP", "REDY", "RUN ", "ZOMB"};

void
show_procs(int highlight_pid)
{
  struct procinfo procs[64];
  struct meminfo mem;
  int count, i;
  
  getmeminfo(&mem);
  int pct = (mem.used_pages * 100) / mem.total_pages;
  
  printf(1, "\n=== Processes [Mem: %d%%] ===\n", pct);
  printf(1, "PID  STATE  NAME\n");
  printf(1, "---  -----  --------\n");
  
  count = getprocinfo(procs, 64);
  for(i = 0; i < count; i++) {
    if(procs[i].state >= 1 && procs[i].state <= 5) {
      if(procs[i].pid == highlight_pid)
        printf(1, "%d   >%s  %s<\n", procs[i].pid, states[procs[i].state], procs[i].name);
      else
        printf(1, "%d    %s  %s\n", procs[i].pid, states[procs[i].state], procs[i].name);
    }
  }
  printf(1, "=========================\n");
}

int
main(int argc, char *argv[])
{
  if(argc < 2) {
    // Just show current processes
    printf(1, "\nProcess Monitor for xv6\n");
    printf(1, "Usage: procmon [command [args...]]\n\n");
    show_procs(-1);
    exit();
  }
  
  // Run command with monitoring
  printf(1, "\n=== PROCMON: %s ===\n", argv[1]);
  
  int cmd_pid = fork();
  if(cmd_pid == 0) {
    sleep(5);
    exec(argv[1], &argv[1]);
    printf(1, "exec failed\n");
    exit();
  }
  
  int mon_pid = fork();
  if(mon_pid == 0) {
    int i;
    for(i = 0; i < 20; i++) {
      show_procs(cmd_pid);
      sleep(50);
    }
    exit();
  }
  
  printf(1, "Target PID: %d\n", cmd_pid);
  
  int wpid;
  while((wpid = wait()) >= 0) {
    if(wpid == cmd_pid) {
      kill(mon_pid);
      printf(1, "\n[%s finished]\n", argv[1]);
    }
  }
  
  exit();
}
