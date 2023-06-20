#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

char* fmtname(char* path) {
  static char buf[DIRSIZ + 1];
  memset(buf, 0, DIRSIZ + 1);
  char* p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  return buf;
}

void find(char* path, char* name) {
  int fd;
  char buf[512], *p;
  struct dirent de;
  struct stat st;

  if (strcmp(name, fmtname(path)) == 0) {
    // find success
    printf("%s\n", path);
  }

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
//   printf("path: %s, type: %d, fmtname(path):%s, name:%s, strcmp:%d\n", path, st.type,
//          fmtname(path), name, strcmp(name, fmtname(path)));

  if (st.type == T_DEVICE || st.type == T_FILE) {
    close(fd);
    return;
  }
  // T_DIR
  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
    printf("find: path too long\n");
    close(fd);
    return;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;
    if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
      continue;
    }
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    find(buf, name);
  }
  close(fd);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(2, "Error!\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
