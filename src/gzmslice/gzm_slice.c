#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libgzx/gzm.h"

int
main (int argc, const char *argv[])
{
	int exc;
	struct gz_macro input_gzm;
	struct gz_macro output_gzm;
	int start_frame;
	int end_frame;

	if (argc != 5)
	{
		printf("%s: Slice a macro from one frame to another.\n", argv[0]);
		printf("Usage: %s <input> <output> <start_frame> <end-frame>\n", argv[0]);
		return EXIT_FAILURE;
	}
	gzm_read(&input_gzm, argv[1]);
	start_frame = atoi(argv[3]);
	end_frame = atoi(argv[4]);
	if (gzm_slice(&output_gzm, &input_gzm, start_frame, end_frame) != 0)
	{
		printf("Could not slice %s with %s start frame and %s end frame\n", argv[2], argv[3], argv[4]);
		if (start_frame > input_gzm.n_input)
			printf("Start frame %s is larger than macro size\n", argv[3]);
		if (end_frame > input_gzm.n_input)
			printf("End frame %s is larger than macro size\n", argv[4]);
		if (start_frame >= end_frame)
			printf("Start frame %s is greater than or equal to end frame %s\n", argv[3], argv[4]);

		exc = EXIT_FAILURE;
	}
	else
	{
		gzm_write(&output_gzm, argv[2]);
		exc = EXIT_SUCCESS;
	}
	gzm_free(&input_gzm);
	gzm_free(&output_gzm);
	return exc;
}
