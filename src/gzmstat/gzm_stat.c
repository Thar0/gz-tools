#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libgzx/gzm.h"

int
main (int argc, const char *argv[])
{
    struct gz_macro gzm;

    if (argc != 2)
    {
        printf("%s: Print information about a macro.\n", argv[0]);
        printf("Usage: %s <input>\n", argv[0]);
        return EXIT_FAILURE;
    }

    gzm_read(&gzm, argv[1]);

    printf("%s:\n", argv[1]);
    gzm_print_stats(&gzm);
    gzm_print_seeds(&gzm);
    //gzm_print_inputs(&gzm);

    gzm_free(&gzm);

    return EXIT_SUCCESS;
}
