#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

static int
readline(char *buf, int max)
{
  int i = 0;
  while(i < max - 1){
    char c;
    int n = read(0, &c, 1);
    if(n < 0) return -1;
    if(n == 0) break;
    buf[i++] = c;
    if(c == '\n') break;
  }
  buf[i] = 0;
  return i;
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "usage: xargs command [args...]\n");
    exit(1);
  }

  // Base arguments: argv[1..argc-1]
  char *base[MAXARG];
  int basec = 0;
  for(int i = 1; i < argc && basec < MAXARG - 1; i++){
    base[basec++] = argv[i];
  }
  base[basec] = 0;

  char line[512];

  while(1){
    int n = readline(line, sizeof(line));
    if(n < 0){
      fprintf(2, "xargs: read error\n");
      exit(1);
    }
    if(n == 0) break;

    char *execv[MAXARG];
    int ec = 0;

    for(int i = 0; i < basec; i++)
      execv[ec++] = base[i];

    int i = 0;
    while(line[i] != 0){
      while(line[i] == ' ' || line[i] == '\t' || line[i] == '\n')
        i++;
      if(line[i] == 0)
        break;

      if(ec >= MAXARG - 1)
        break;

      execv[ec++] = &line[i];

      while(line[i] != 0 && line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
        i++;

      if(line[i] != 0){
        line[i] = 0;
        i++;
      }
    }

    execv[ec] = 0;

    int pid = fork();
    if(pid < 0){
      fprintf(2, "xargs: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec(execv[0], execv);
      fprintf(2, "xargs: exec %s failed\n", execv[0]);
      exit(1);
    } else {
      wait(0);
    }
  }

  exit(0);
}