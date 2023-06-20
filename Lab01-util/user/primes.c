#include "kernel/types.h"

#include "user/user.h"

int main(int argc, char* argv[]) {
  int p[35][2];
  int i = 0;
  pipe(p[0]);
  if (fork() == 0) {
    // child
    close(p[0][1]);
    while (1) {
      i++;
      pipe(p[i]);
      // read from p[i-1][0]
      int filter;
      if (read(p[i - 1][0], &filter, 4) != 4) {
        exit(0);
      }
      printf("prime %d\n", filter);
      // printf("fork\n");
      if (fork() == 0) {
        // child
        close(p[i][1]);
      } else {
        // parent
        // write to p[i][1]
        close(p[i][0]);
        int tmp;
        while (read(p[i - 1][0], &tmp, 4) != 0) {
          if (tmp % filter != 0) {
            write(p[i][1], &tmp, 4);
          }
        }
        close(p[i - 1][0]);
        close(p[i][1]);
        while(wait(0) != -1)
          ;
        exit(0);
      }
    }
  } else {
    // parent
    close(p[0][0]);
    for (int x = 2; x <= 35; x++) {
      write(p[0][1], &x, 4);
    }
    close(p[0][1]);
  }
  while (wait(0) != -1)
    ;
  exit(0);
}
