#ifndef __STRIP_FS__
#define __STRIP_FS__

#include <stdint.h>

#define MAGIC_1 0x69
#define MAGIC_2 0x42

#define ALIGN_TO 8

struct stripfs_header {
  uint8_t magic[2];
  int num_files;
};

struct file_header {
  char filename[256];
  int length;
  int offset;
};

static int align_up(int n, int align) { return (n + align - 1) & ~(align - 1); }

#endif
