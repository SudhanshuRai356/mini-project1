#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void cburst(int n) {
  int start = uptime();
  while (uptime() - start < n)
    ;
}

int main() {
  int burst[4]  = {10, 5, 20, 33};
  int iowait[4] = {3, 7, 4, 8};
  char name[16];

  for (int i = 0; i < 4; i++) {
    int pid = fork();
    if (pid == 0) {
      name[0] = 'l'; name[1] = 'o'; name[2] = 'g';
      name[3] = '0' + i; name[4] = 0;
      int fd = open(name, O_CREATE | O_WRONLY);

#ifdef mlfq
      fprintf(fd, "Uptime\tProcessID\tQueue\n");
#else
      fprintf(fd, "Uptime\tProcessID\n");
#endif

      int start = uptime();
      while (uptime() - start < 300) {
#ifdef mlfq
        fprintf(fd, "%d\t%d\t%d\n", uptime(), getpid(), getqueue());
#else
        fprintf(fd, "%d\t%d\n", uptime(), getpid());
#endif
        cburst(burst[i]);
#ifdef mlfq
        fprintf(fd, "%d\t%d\t%d\n", uptime(), getpid(), getqueue());
#else
        fprintf(fd, "%d\t%d\n", uptime(), getpid());
#endif
        pause(iowait[i]);
      }
      close(fd);
      exit(0);
    }
  }
  for (int i = 0; i < 4; i++)
    wait(0);
  exit(0);
}