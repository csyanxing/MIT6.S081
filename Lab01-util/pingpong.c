#include "kernel/types.h"

#include "user/user.h"

int main(int argc, char* argv[]) {
  int p1[2];
  int p2[2];
  char buf[10];
  int pid;
  int n;
  pipe(p1);
  pipe(p2);

  pid = fork();
  if (pid < 0) {
    fprintf(2, "Fork error!\n");
    exit(1);
  }
  if (pid == 0) {
    // child process
    close(p1[1]);
    close(p2[0]);
    n = read(p1[0], buf, 1);
    if (n != 1) {
      fprintf(2, "Child read error!\n");
      exit(1);
    }
    printf("%d: received ping\n", getpid());
    n = write(p2[1], buf, 1);
    if (n != 1) {
      fprintf(2, "Child write error!\n");
      exit(1);
    }
    close(p1[0]);
    close(p2[1]);
    exit(0);
  } else {
    // parent process
    close(p1[0]);
    close(p2[1]);
    n = write(p1[1], "0", 1);
    if (n != 1) {
      fprintf(2, "Parent write error!\n");
      exit(1);
    }
    n = read(p2[0], buf, 1);
    if (n != 1) {
      fprintf(2, "Parent read error!\n");
      exit(1);
    }
    printf("%d: received pong\n", getpid());
    close(p1[1]);
    close(p2[0]);
    exit(0);
  }
  exit(0);
}
