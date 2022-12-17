#ifndef FILES_H_
#define FILES_H_

#include <stdbool.h>
#include <stdint.h>

void *
files_read_whole_file (const char *file_name, bool bin, size_t *size_out);

void
files_write_whole_file (const char *file_name, bool bin, void *data, size_t size);

#endif
