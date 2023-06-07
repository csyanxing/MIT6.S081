#include "kernel/types.h"
#include "user/user.h"

#define MAX_ARGV_SIZE 20

int main(int argc, char* argv[]) {
  char* new_argv[MAX_ARGV_SIZE];
  for (int i = 1; i < argc; i++) {
    new_argv[i - 1] = argv[i];
  }
  char buf[MAX_ARGV_SIZE];
  memset(buf, 0, MAX_ARGV_SIZE);
  char* p = buf;
  while (read(0, p, 1) > 0) {
    if ((*p) == '\n') {
      *p = 0;
      new_argv[argc - 1] = buf;
      if (fork() == 0) {
        exec(new_argv[0], new_argv);
        exit(1);
      } else {
        wait(0);
      }
      memset(buf, 0, MAX_ARGV_SIZE);
      p = buf;
    } else {
      p++;
    }
  }
  exit(0);
}
