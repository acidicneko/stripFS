#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strip.h>

int main(int argc, char *argv[]) {
  if (argc < 4 || argc % 2 != 0) {
    fprintf(
        stderr,
        "Usage: %s output.img infile1 targetname1 [infile2 targetname2 ...]\n",
        argv[0]);
    return 1;
  }

  int num_files = (argc - 2) / 2;
  FILE *out = fopen(argv[1], "wb");
  if (!out) {
    perror("Failed to open output file");
    return 1;
  }

  struct stripfs_header header = {.magic = {MAGIC_1, MAGIC_2},
                                  .num_files = num_files};
  fwrite(&header, sizeof(header), 1, out);

  int current_offset = align_up(
      sizeof(header) + (num_files * sizeof(struct file_header)), ALIGN_TO);

  for (int i = 0; i < num_files; i++) {
    const char *infile = argv[2 + i * 2];
    const char *targetname = argv[3 + i * 2];

    FILE *in = fopen(infile, "rb");
    if (!in) {
      fprintf(stderr, "Failed to open input file: %s\n", infile);
      fclose(out);
      return 1;
    }

    fseek(in, 0, SEEK_END);
    int file_size = ftell(in);
    fclose(in);

    struct file_header fheader = {0};
    strncpy(fheader.filename, targetname, 255);
    fheader.filename[255] = '\0';
    fheader.length = file_size;
    fheader.offset = current_offset;

    fwrite(&fheader, sizeof(fheader), 1, out);
    current_offset = align_up(current_offset + file_size, ALIGN_TO);
  }

  for (int i = 0; i < num_files; i++) {
    const char *infile = argv[2 + i * 2];

    FILE *in = fopen(infile, "rb");
    if (!in) {
      fprintf(stderr, "Warning: Skipping missing file: %s\n", infile);
      continue;
    }

    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in)) > 0) {
      fwrite(buffer, 1, bytes_read, out);
    }

    // Pad to alignment
    int padding = align_up(ftell(out), ALIGN_TO) - ftell(out);
    char padbuf[8] = {0};
    fwrite(padbuf, 1, padding, out);

    fclose(in);
  }

  printf("Wrote %d files into %s successfully!\n", num_files, argv[1]);

  fclose(out);
  return 0;
}
