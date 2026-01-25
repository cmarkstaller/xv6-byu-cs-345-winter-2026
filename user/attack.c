#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int main(int argc, char *argv[]) {
  // Get heap start
  char *heap_start = sbrk(0);
  
  sbrk(4096 * 40);
  
  // The secret is on page 16
  char *page_16 = heap_start + (16 * 4096);
  
  write(2, page_16 + 32, 8);
  
  exit(0);
}