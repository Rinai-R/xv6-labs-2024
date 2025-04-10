#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"


int main(int argc, char *argv[]) {
  char *end = sbrk(PGSIZE*32);
  end = end + 16 * PGSIZE;
  printf("secret: %s\n", end+32);
  write(2, end+32, 8);
  exit(0);
}
