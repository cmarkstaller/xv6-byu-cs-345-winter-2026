#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static char*
basename(char *path)
{
  char *p = path + strlen(path);
  while(p >= path && *p != '/')
    p--;
  return p + 1;
}

static void
find(char *path, char *target)
{
  int fd;
  struct stat st;
  struct dirent de;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // If this path's last component matches target, print it.
  if(strcmp(basename(path), target) == 0){
    printf("%s\n", path);
  }

  if(st.type != T_DIR){
    close(fd);
    return;
  }

  // Recurse into directory entries
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;

    char name[DIRSIZ + 1];
    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    char next[512];
    int len = strlen(path);

    if(len + 1 + DIRSIZ + 1 > sizeof(next)){
      fprintf(2, "find: path too long\n");
      continue;
    }

    // next = path + "/" + name
    strcpy(next, path);
    if(len > 0 && next[len - 1] != '/'){
      next[len] = '/';
      next[len + 1] = 0;
      len++;
    }
    strcpy(next + len, name);

    find(next, target);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: find <path> <name>\n");
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}