#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pipeOne[2];
  int pipeTwo[2];

  char buf[1];

  pipe(pipeOne);
  pipe(pipeTwo);

  int pid = fork();

  if (pid != 0)
  {
    write(pipeOne[1], "a", 1);
    read(pipeTwo[0], buf, sizeof(buf));
    printf("%d: received pong\n", getpid());
    exit(0);
  }

  else
  {
    read(pipeOne[0], buf, sizeof(buf));
    printf("%d: received ping\n", getpid());
    write(pipeTwo[1], "b", 1);
    exit(0);
  }
}
