#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Not enough Arguments passed in\n");
    exit(1);
  }

  int timeToSleep = atoi(argv[1]);
  sleep(timeToSleep);
  exit(0);
}
