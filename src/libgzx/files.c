#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files.h"

__attribute__((noreturn)) static void
files_fatal_error (const char *msgfmt, ...)
{
    va_list args;

    fputs("error: ", stderr);

    va_start(args, msgfmt);
    vfprintf(stderr, msgfmt, args);
    va_end(args);

    fputc('\n', stderr);

    exit(EXIT_FAILURE);
}

void *
files_read_whole_file (const char *file_name, bool bin, size_t *size_out)
{
    FILE *file = fopen(file_name, (bin) ? "rb" : "r");
    uint8_t *buffer = NULL;
    size_t size;

    if (file == NULL)
        files_fatal_error("failed to open file '%s' for reading: %s", file_name, strerror(errno));

    // get size
    fseek(file, 0, SEEK_END);
    size = ftell(file);

    // if the file is empty, return NULL buffer and 0 size
    if (size == 0)
        goto end;

    // allocate buffer
    buffer = malloc(size + 1);
    if (buffer == NULL)
        files_fatal_error("could not allocate buffer for file '%s'", file_name);

    // read file
    fseek(file, 0, SEEK_SET);
    if (fread(buffer, size, 1, file) != 1)
        files_fatal_error("error reading from file '%s': %s", file_name, strerror(errno));

    // null-terminate the buffer (in case of text files)
    buffer[size] = '\0';

    fclose(file);

end:
    if (size_out != NULL)
        *size_out = size;
    return buffer;
}

void
files_write_whole_file (const char *file_name, bool bin, void *data, size_t size)
{
    FILE *file = fopen(file_name, (bin) ? "wb" : "w");

    if (file == NULL)
        files_fatal_error("failed to open file '%s' for writing: %s", file_name, strerror(errno));

    if (fwrite(data, 1, size, file) != size)
        files_fatal_error("could not write %d bytes to '%s': %s", size, file_name, strerror(errno));
    
    fclose(file);
}
