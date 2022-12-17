#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libgzx/gzm.h"

int
main (int argc, const char *argv[])
{
    int exc;
    struct gz_macro gzm1;
    struct gz_macro gzm2;
    struct gz_macro gzm_out;

    if (argc != 4)
    {
        printf("%s: Concatenate two macros at the last/first frame that saved an rng seed.\n", argv[0]);
        printf("Usage: %s <input1> <input2> <output>\n", argv[0]);
        return EXIT_FAILURE;
    }

    gzm_read(&gzm1, argv[1]);
    gzm_read(&gzm2, argv[2]);
    if (gzm_cat_r(&gzm_out, &gzm1, &gzm2) != 0)
    {
        printf("Could not concat %s with %s\n", argv[1], argv[2]);
        if (gzm1.n_seed == 0)
            printf("%s does not have any saved rng seeds\n", argv[1]);
        if (gzm2.n_seed == 0)
            printf("%s does not have any saved rng seeds\n", argv[2]);
        exc = EXIT_FAILURE;
    }
    else
    {
        gzm_write(&gzm_out, argv[3]);
        exc = EXIT_SUCCESS;
    }
    gzm_free(&gzm1);
    gzm_free(&gzm2);
    gzm_free(&gzm_out);
    return exc;
}
