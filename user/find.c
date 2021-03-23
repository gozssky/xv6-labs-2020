#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

const char*
basename(const char* path) {
  const char* p;
  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

void
find(const char* directory, const char* filename) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(directory, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", directory);
    exit(1);
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", directory);
    close(fd);
    exit(1);
  }

  if (st.type != T_DIR) {
    if (st.type == T_FILE && strcmp(basename(directory), filename) == 0) {
      printf("%s\n", directory);
    }
    close(fd);
    return;
  }

  if (strlen(directory) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }
  strcpy(buf, directory);
  p = buf + strlen(buf);
  *p++ = '/';

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0 || strcmp(de.name, ".") == 0 ||
        strcmp(de.name, "..") == 0) {
      continue;
    }
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    find(buf, filename);
  }
  close(fd);
}

int
main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(2, "Usage: find directory filename\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
