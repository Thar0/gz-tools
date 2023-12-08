#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gzm.h"
#include "files.h"

static void
bswap (void *dst, const void *data, size_t size)
{
    switch (size)
    {
        case sizeof(uint32_t):
            *(uint32_t*)(dst) = __builtin_bswap32(*(uint32_t*)(data));
            break;
        case sizeof(uint16_t):
            *(uint16_t*)(dst) = __builtin_bswap16(*(uint16_t*)(data));
            break;
        case sizeof(uint8_t):
            *(uint8_t*)(dst) = *(uint8_t*)(data);
            break;
        default:
            fprintf(stderr, "Bad size: %lu\n", size);
            exit(EXIT_FAILURE);
    }
}

static int
serial_read (void *dst, size_t size, uint8_t **p, const uint8_t *end)
{
    if (*p + size <= end)
    {
        bswap(dst, *p, size);
        *p += size;
        return (*p == end) ? -1 : 0;
    }
    return -1;
}

static int
serial_write (const void *src, size_t size, uint8_t **p, const uint8_t *end)
{
    if (*p + size <= end)
    {
        bswap(*p, src, size);
        *p += size;
        return 0;
    }
    return -1;
}

#define gzm_serial_read(field)                                  \
    do {                                                        \
        if (serial_read(&(field), sizeof(field), &p, end) != 0) \
            goto eof;                                           \
    } while (0)

#define gzm_serial_write(field)                                  \
    do {                                                         \
        if (serial_write(&(field), sizeof(field), &p, end) != 0) \
            goto eof;                                            \
    } while (0)

static int
mem_dup (void **buf, size_t size)
{
    void *src = *buf;
    if (src != NULL && size != 0)
    {
        *buf = malloc(size);
        if (*buf == NULL)
            return -1;
        memcpy(*buf, src, size);
    }
    return 0;
}

int
gzm_read (struct gz_macro *gzm, const char *file_name)
{
    size_t i;
    size_t size;
    uint8_t *data = files_read_whole_file(file_name, true, &size);
    uint8_t *p = &data[0];
    uint8_t *end = &data[size];

    memset(gzm, 0, sizeof(struct gz_macro));

    gzm_serial_read(gzm->n_input);
    gzm_serial_read(gzm->n_seed);

    gzm_serial_read(gzm->input_start.pad);
    gzm_serial_read(gzm->input_start.x);
    gzm_serial_read(gzm->input_start.y);

    gzm->input = malloc(gzm->n_input * sizeof(struct movie_input));
    for (i = 0; i < gzm->n_input; ++i)
    {
        struct movie_input *input = &gzm->input[i];

        gzm_serial_read(input->raw.pad);
        gzm_serial_read(input->raw.x);
        gzm_serial_read(input->raw.y);
        gzm_serial_read(input->pad_delta);
    }

    gzm->seed = malloc(gzm->n_seed * sizeof(struct movie_seed));
    for (i = 0; i < gzm->n_seed; ++i)
    {
        struct movie_seed *seed = &gzm->seed[i];

        gzm_serial_read(seed->frame_idx);
        gzm_serial_read(seed->old_seed);
        gzm_serial_read(seed->new_seed);
    }

    gzm_serial_read(gzm->n_oca_input);
    gzm_serial_read(gzm->n_oca_sync);
    gzm_serial_read(gzm->n_room_load);

    gzm->oca_input = malloc(gzm->n_oca_input * sizeof(struct movie_oca_input));
    for (i = 0; i < gzm->n_oca_input; ++i)
    {
        struct movie_oca_input *oca_input = &gzm->oca_input[i];

        gzm_serial_read(oca_input->frame_idx);
        gzm_serial_read(oca_input->pad);
        gzm_serial_read(oca_input->adjusted_x);
        gzm_serial_read(oca_input->adjusted_y);
    }

    gzm->oca_sync = malloc(gzm->n_oca_sync * sizeof(struct movie_oca_sync));
    for (i = 0; i < gzm->n_oca_sync; ++i)
    {
        struct movie_oca_sync *oca_sync = &gzm->oca_sync[i];

        gzm_serial_read(oca_sync->frame_idx);
        gzm_serial_read(oca_sync->audio_frames);
    }

    gzm->room_load = malloc(gzm->n_room_load * sizeof(struct movie_room_load));
    for (i = 0; i < gzm->n_room_load; ++i)
    {
        struct movie_room_load *room_load = &gzm->room_load[i];

        gzm_serial_read(room_load->frame_idx);
    }

    gzm_serial_read(gzm->rerecords);
    gzm_serial_read(gzm->last_recorded_frame);

eof:
    free(data);
    return 0;
}

int
gzm_write (const struct gz_macro *gzm, const char *file_name)
{
    size_t i;
    size_t size = GZM_SERIAL_SIZE(gzm);
    uint8_t *data = malloc(size);
    uint8_t *p = &data[0];
    uint8_t *end = &data[size];

    gzm_serial_write(gzm->n_input);
    gzm_serial_write(gzm->n_seed);

    gzm_serial_write(gzm->input_start.pad);
    gzm_serial_write(gzm->input_start.x);
    gzm_serial_write(gzm->input_start.y);

    for (i = 0; i < gzm->n_input; ++i)
    {
        struct movie_input *input = &gzm->input[i];

        gzm_serial_write(input->raw.pad);
        gzm_serial_write(input->raw.x);
        gzm_serial_write(input->raw.y);
        gzm_serial_write(input->pad_delta);
    }

    for (i = 0; i < gzm->n_seed; ++i)
    {
        struct movie_seed *seed = &gzm->seed[i];

        gzm_serial_write(seed->frame_idx);
        gzm_serial_write(seed->old_seed);
        gzm_serial_write(seed->new_seed);
    }

    gzm_serial_write(gzm->n_oca_input);
    gzm_serial_write(gzm->n_oca_sync);
    gzm_serial_write(gzm->n_room_load);

    for (i = 0; i < gzm->n_oca_input; ++i)
    {
        struct movie_oca_input *oca_input = &gzm->oca_input[i];

        gzm_serial_write(oca_input->frame_idx);
        gzm_serial_write(oca_input->pad);
        gzm_serial_write(oca_input->adjusted_x);
        gzm_serial_write(oca_input->adjusted_y);
    }

    for (i = 0; i < gzm->n_oca_sync; ++i)
    {
        struct movie_oca_sync *oca_sync = &gzm->oca_sync[i];

        gzm_serial_write(oca_sync->frame_idx);
        gzm_serial_write(oca_sync->audio_frames);
    }

    for (i = 0; i < gzm->n_room_load; ++i)
    {
        struct movie_room_load *room_load = &gzm->room_load[i];

        gzm_serial_write(room_load->frame_idx);
    }

    gzm_serial_write(gzm->rerecords);
    gzm_serial_write(gzm->last_recorded_frame);

    files_write_whole_file(file_name, true, data, size);
    free(data);
    return 0;
eof:
    // no more room
    free(data);
    return -1;
}

int
gzm_new (struct gz_macro *gzm)
{
    memset(gzm, 0, sizeof(struct gz_macro));
    return 0;
}

int
gzm_free (struct gz_macro *gzm)
{
    free(gzm->input);
    free(gzm->seed);
    free(gzm->oca_input);
    free(gzm->oca_sync);
    free(gzm->room_load);
    memset(gzm, 0, sizeof(struct gz_macro));
    return 0;
}

/* Copy gzm_in to gzm_out */
int
gzm_dup (struct gz_macro *gzm_out, const struct gz_macro *gzm_in)
{
    int ret = 0;

    // Copy structure
    memcpy(gzm_out, gzm_in, sizeof(struct gz_macro));

    // Copy buffers
    ret |= mem_dup((void **)&gzm_out->input, gzm_out->n_input * sizeof(struct movie_input));
    ret |= mem_dup((void **)&gzm_out->seed, gzm_out->n_seed * sizeof(struct movie_seed));
    ret |= mem_dup((void **)&gzm_out->oca_input, gzm_out->n_oca_input * sizeof(struct movie_oca_input));
    ret |= mem_dup((void **)&gzm_out->oca_sync, gzm_out->n_oca_sync * sizeof(struct movie_oca_sync));
    ret |= mem_dup((void **)&gzm_out->room_load, gzm_out->n_room_load * sizeof(struct movie_room_load));
    return ret;
}

/* Trim gz macro `gzm` to end on `end` (exclusive) */
int
gzm_trim (struct gz_macro *gzm, uint32_t end)
{
    if (end > gzm->n_input)
        return -1;

    // Trim input
    gzm->n_input = end;
    gzm->input = realloc(gzm->input, gzm->n_input * sizeof(struct movie_input));

    // Trim seed
    size_t n_seed = 0;
    for (struct movie_seed *seed = &gzm->seed[0]; seed < &gzm->seed[gzm->n_seed]; seed++)
    {
        if (seed->frame_idx < gzm->n_input)
            n_seed++;
    }
    gzm->n_seed = n_seed;
    gzm->seed = realloc(gzm->seed, gzm->n_seed * sizeof(struct movie_seed));

    // Trim oca input
    size_t n_oca_input = 0;
    for (struct movie_oca_input *oca_input = &gzm->oca_input[0]; oca_input < &gzm->oca_input[gzm->n_oca_input]; oca_input++)
    {
        if (oca_input->frame_idx < gzm->n_input)
            n_oca_input++;
    }
    gzm->n_oca_input = n_oca_input;
    gzm->oca_input = realloc(gzm->oca_input, gzm->n_oca_input * sizeof(struct movie_oca_input));

    // Trim oca sync
    size_t n_oca_sync = 0;
    for (struct movie_oca_sync *oca_sync = &gzm->oca_sync[0]; oca_sync < &gzm->oca_sync[gzm->n_oca_sync]; oca_sync++)
    {
        if (oca_sync->frame_idx < gzm->n_input)
            n_oca_sync++;
    }
    gzm->n_oca_sync = n_oca_sync;
    gzm->oca_sync = realloc(gzm->oca_sync, gzm->n_oca_sync * sizeof(struct movie_oca_sync));

    // Trim room load
    size_t n_room_load = 0;
    for (struct movie_room_load *room_load = &gzm->room_load[0]; room_load < &gzm->room_load[gzm->n_room_load]; room_load++)
    {
        if (room_load->frame_idx < gzm->n_input)
            n_room_load++;
    }
    gzm->n_room_load = n_room_load;
    gzm->room_load = realloc(gzm->room_load, gzm->n_room_load * sizeof(struct movie_room_load));

    // Adjust last recorded frame
    gzm->last_recorded_frame = gzm->n_input - 1;
    return 0;
}

int
gzm_cat (struct gz_macro *gzm, const struct gz_macro *gzm1, const struct gz_macro *gzm2)
{
    // Zero destination
    memset(gzm, 0, sizeof(struct gz_macro));

    // Copy inputs
    gzm->n_input = gzm1->n_input + gzm2->n_input;
    if (gzm->n_input != 0)
    {
        gzm->input = malloc(gzm->n_input * sizeof(struct movie_input));
        if (gzm1->input != NULL)
            memcpy(&gzm->input[0],             gzm1->input, gzm1->n_input * sizeof(struct movie_input));
        if (gzm2->input != NULL)
            memcpy(&gzm->input[gzm1->n_input], gzm2->input, gzm2->n_input * sizeof(struct movie_input));
    }

    // Copy seed
    gzm->n_seed = gzm1->n_seed + gzm2->n_seed;
    if (gzm->n_seed)
    {
        gzm->seed = malloc(gzm->n_seed * sizeof(struct movie_seed));
        if (gzm1->seed != NULL)
            memcpy(&gzm->seed[0],            gzm1->seed, gzm1->n_seed * sizeof(struct movie_seed));
        if (gzm2->seed != NULL)
            memcpy(&gzm->seed[gzm1->n_seed], gzm2->seed, gzm2->n_seed * sizeof(struct movie_seed));

        // Adjust frame numbers
        for (int i = 0; i < gzm2->n_seed; i++)
            gzm->seed[gzm1->n_seed + i].frame_idx += gzm1->n_input;
    }

    // Copy input_start of gzm1, TODO what about input_start of gzm2?
    gzm->input_start = gzm1->input_start;

    // Copy oca input if present
    gzm->n_oca_input = gzm1->n_oca_input + gzm2->n_oca_input;
    if (gzm->n_oca_input != 0)
    {
        gzm->oca_input = malloc(gzm->n_oca_input * sizeof(struct movie_oca_input));
        if (gzm1->oca_input != NULL)
            memcpy(&gzm->oca_input[0],                 gzm1->oca_input, gzm1->n_oca_input * sizeof(struct movie_oca_input));
        if (gzm2->oca_input != NULL)
            memcpy(&gzm->oca_input[gzm1->n_oca_input], gzm2->oca_input, gzm2->n_oca_input * sizeof(struct movie_oca_input));

        // Adjust frame numbers
        for (int i = 0; i < gzm2->n_oca_input; i++)
            gzm->oca_input[gzm1->n_oca_input + i].frame_idx += gzm1->n_input;
    }

    // Copy oca sync if present
    gzm->n_oca_sync = gzm1->n_oca_sync + gzm2->n_oca_sync;
    if (gzm->n_oca_sync != 0)
    {
        gzm->oca_sync = malloc(gzm->n_oca_sync * sizeof(struct movie_oca_sync));
        if (gzm1->oca_sync != NULL)
            memcpy(&gzm->oca_sync[0],                gzm1->oca_sync, gzm1->n_oca_sync * sizeof(struct movie_oca_sync));
        if (gzm2->oca_sync != NULL)
            memcpy(&gzm->oca_sync[gzm1->n_oca_sync], gzm2->oca_sync, gzm2->n_oca_sync * sizeof(struct movie_oca_sync));

        // Adjust frame numbers
        for (int i = 0; i < gzm2->n_oca_sync; i++)
            gzm->oca_sync[gzm1->n_oca_sync + i].frame_idx += gzm1->n_input;
    }

    // Copy room load if present
    gzm->n_room_load = gzm1->n_room_load + gzm2->n_room_load;
    if (gzm->n_room_load != 0)
    {
        gzm->room_load = malloc(gzm->n_room_load * sizeof(struct movie_room_load));
        if (gzm1->room_load != NULL)
            memcpy(&gzm->room_load[0],                 gzm1->room_load, gzm1->n_room_load * sizeof(struct movie_room_load));
        if (gzm2->room_load != NULL)
            memcpy(&gzm->room_load[gzm1->n_room_load], gzm2->room_load, gzm2->n_room_load * sizeof(struct movie_room_load));

        // Adjust frame numbers
        for (int i = 0; i < gzm2->n_room_load; i++)
            gzm->room_load[gzm1->n_room_load + i].frame_idx += gzm1->n_input;
    }

    gzm->rerecords = gzm1->rerecords + gzm2->rerecords;
    gzm->last_recorded_frame = 0; // TODO how to merge this if at all
    return 0;
}

/* Concat 2 gz macros at the last recorded rng seed frame index, and take old_seed from the first macro and new_seed from the second */
int
gzm_cat_r (struct gz_macro *gzm, const struct gz_macro *gzm1, const struct gz_macro *gzm2)
{
    // Each macro must have recorded at least one rng seed for this process to work
    if (gzm1->n_seed == 0 || gzm2->n_seed == 0)
        return -1;

    // Zero destination
    memset(gzm, 0, sizeof(struct gz_macro));

    // Find the frame to stitch on
    int last_frame_1 = gzm1->seed[gzm1->n_seed - 1].frame_idx;
    // Find the frame in the second macro to start from
    int first_frame_2 = gzm2->seed[0].frame_idx;

    // Copy inputs
    gzm->n_input = (last_frame_1 - 0) + (gzm2->n_input - first_frame_2);
    gzm->input = malloc(gzm->n_input * sizeof(struct movie_input));
    memcpy(&gzm->input[0],            &gzm1->input[0],             ( last_frame_1 -             0) * sizeof(struct movie_input));
    memcpy(&gzm->input[last_frame_1], &gzm2->input[first_frame_2], (gzm2->n_input - first_frame_2) * sizeof(struct movie_input));

    // Copy input_start of gzm1, TODO what about input_start of gzm2?
    gzm->input_start = gzm1->input_start;

    int frame_adj = last_frame_1 - first_frame_2;

    // Copy seeds (-1 since we merge last seed of first with first seed of second)
    gzm->n_seed = gzm1->n_seed + gzm2->n_seed - 1;
    gzm->seed = malloc(gzm->n_seed * sizeof(struct movie_seed));
    memcpy(&gzm->seed[0], &gzm1->seed[0], gzm1->n_seed * sizeof(struct movie_seed));
    gzm->seed[gzm1->n_seed - 1].new_seed = gzm2->seed[0].new_seed;
    memcpy(&gzm->seed[gzm1->n_seed], &gzm2->seed[1], (gzm2->n_seed - 1) * sizeof(struct movie_seed));
    // Increment seed frames
    for (int i = gzm1->n_seed; i < gzm->n_seed; i++)
        gzm->seed[i].frame_idx += frame_adj;

    // Copy oca input if present
    int n_oca_input_1 = gzm1->n_oca_input;
    for (int i = gzm1->n_oca_input - 1; i >= 0; i--)
    {
        if (gzm1->oca_input[i].frame_idx >= last_frame_1)
            n_oca_input_1--;
        else
            break;
    }
    int n_oca_input_2 = gzm2->n_oca_input;
    for (int i = 0; i < gzm2->n_oca_input; i++)
    {
        if (gzm2->oca_input[i].frame_idx < first_frame_2)
            n_oca_input_2--;
        else
            break;
    }
    gzm->n_oca_input = n_oca_input_1 + n_oca_input_2;
    if (gzm->n_oca_input != 0)
    {
        gzm->oca_input = malloc(gzm->n_oca_input * sizeof(struct movie_oca_input));
        if (gzm1->oca_input != NULL)
            memcpy(&gzm->oca_input[0],             &gzm1->oca_input[0], n_oca_input_1 * sizeof(struct movie_oca_input));
        if (gzm2->oca_input != NULL)
            memcpy(&gzm->oca_input[n_oca_input_1], &gzm2->oca_input[gzm2->n_oca_input - n_oca_input_2], n_oca_input_2 * sizeof(struct movie_oca_input));

        // Adjust frame numbers
        for (int i = 0; i < n_oca_input_2; i++)
            gzm->oca_input[n_oca_input_1 + i].frame_idx += frame_adj;
    }

    // Copy oca sync if present
    int n_oca_sync_1 = gzm1->n_oca_sync;
    for (int i = gzm1->n_oca_sync - 1; i >= 0; i--)
    {
        if (gzm1->oca_sync[i].frame_idx >= last_frame_1)
            n_oca_sync_1--;
        else
            break;
    }
    int n_oca_sync_2 = gzm2->n_oca_sync;
    for (int i = 0; i < gzm2->n_oca_sync; i++)
    {
        if (gzm2->oca_sync[i].frame_idx < first_frame_2)
            n_oca_sync_2--;
        else
            break;
    }
    gzm->n_oca_sync = n_oca_sync_1 + n_oca_sync_2;
    if (gzm->n_oca_sync != 0)
    {
        gzm->oca_sync = malloc(gzm->n_oca_sync * sizeof(struct movie_oca_sync));
        if (gzm1->oca_sync != NULL)
            memcpy(&gzm->oca_sync[0],             &gzm1->oca_sync[0], n_oca_sync_1 * sizeof(struct movie_oca_sync));
        if (gzm2->oca_sync != NULL)
            memcpy(&gzm->oca_sync[n_oca_sync_1], &gzm2->oca_sync[gzm2->n_oca_sync - n_oca_sync_2], n_oca_sync_2 * sizeof(struct movie_oca_sync));

        // Adjust frame numbers
        for (int i = 0; i < n_oca_sync_2; i++)
            gzm->oca_sync[n_oca_sync_1 + i].frame_idx += frame_adj;
    }

    // Copy room load if present
    int n_room_load_1 = gzm1->n_room_load;
    for (int i = gzm1->n_room_load - 1; i >= 0; i--)
    {
        if (gzm1->room_load[i].frame_idx >= last_frame_1)
            n_room_load_1--;
        else
            break;
    }
    int n_room_load_2 = gzm2->n_room_load;
    for (int i = 0; i < gzm2->n_room_load; i++)
    {
        if (gzm2->room_load[i].frame_idx < first_frame_2)
            n_room_load_2--;
        else
            break;
    }
    gzm->n_room_load = n_room_load_1 + n_room_load_2;
    if (gzm->n_room_load != 0)
    {
        gzm->room_load = malloc(gzm->n_room_load * sizeof(struct movie_room_load));
        if (gzm1->room_load != NULL)
            memcpy(&gzm->room_load[0],             &gzm1->room_load[0], n_room_load_1 * sizeof(struct movie_room_load));
        if (gzm2->room_load != NULL)
            memcpy(&gzm->room_load[n_room_load_1], &gzm2->room_load[gzm2->n_room_load - n_room_load_2], n_room_load_2 * sizeof(struct movie_room_load));

        // Adjust frame numbers
        for (int i = 0; i < n_room_load_2; i++)
            gzm->room_load[n_room_load_1 + i].frame_idx += frame_adj;
    }

    gzm->rerecords = gzm1->rerecords + gzm2->rerecords;
    gzm->last_recorded_frame = 0; // TODO how to merge this if at all
    return 0;
}

int
gzm_slice(struct gz_macro *output_gzm, const struct gz_macro *input_gzm, uint32_t frame_start, uint32_t frame_end) 
{ 
    // Each macro must have recorded at least one rng seed for this process to work and be within the bounds of the macro frames
    if (input_gzm->n_seed == 0 || frame_start > input_gzm->n_input || frame_end > input_gzm->n_input || frame_end <= frame_start)
        return -1;

    // Zero destination
    memset(output_gzm, 0, sizeof(struct gz_macro));
    // Copy inputs
    output_gzm->n_input = frame_end - frame_start;
    output_gzm->input = malloc(output_gzm->n_input * sizeof(struct movie_input));
    memcpy(&output_gzm->input[0], &input_gzm->input[frame_start], output_gzm->n_input * sizeof(struct movie_input));
	
	int n_seed = 0;
	int first_seed_idx = 0;
	int last_seed_idx = 0;
	bool first_seed_idx_set = false;
	for (int i = 0; i < input_gzm->n_seed; i++)
	{
		if (input_gzm->seed[i].frame_idx >= frame_start) {
			if (input_gzm->seed[i].frame_idx <= frame_end) { 
				if(!first_seed_idx_set) {
					first_seed_idx = i;
					first_seed_idx_set = true;
				}
				input_gzm->seed[i].frame_idx -= frame_start;
				n_seed++;
			}
			else { 
				last_seed_idx = i - 1;
				break;
			}
		}
	}
	output_gzm->n_seed = n_seed;
	output_gzm->seed = malloc(output_gzm->n_seed * sizeof(struct movie_seed));
	memcpy(&output_gzm->seed[0], &input_gzm->seed[first_seed_idx], (last_seed_idx - first_seed_idx + 1) * sizeof(struct movie_seed));

	if (input_gzm->n_oca_input != 0 && input_gzm->oca_input != NULL) { 
		// Copy oca input if present
		int n_oca_input = 0;
		int first_idx = 0;
		int last_idx = 0;
		bool first_idx_set = false;
		for (int i = 0; i < input_gzm->n_oca_input; i++)
		{
			if (input_gzm->oca_input[i].frame_idx >= frame_start) {
				if (input_gzm->oca_input[i].frame_idx <= frame_end) { 
					if(!first_idx_set) {
						first_idx = i;
						first_idx_set = true;
					}
					input_gzm->oca_input[i].frame_idx -= frame_start;
					n_oca_input++;
				}
				else { 
					last_idx = i - 1;
					break;
				}
			}
		}
		output_gzm->n_oca_input = n_oca_input;
		output_gzm->oca_input = malloc(output_gzm->n_oca_input * sizeof(struct movie_oca_input));
		memcpy(&output_gzm->oca_input[0], &input_gzm->oca_input[first_idx], (last_idx - first_idx + 1) * sizeof(struct movie_oca_input));
	}
	
	if (input_gzm->n_oca_sync != 0 && input_gzm->oca_sync != NULL) { 
		// Copy oca sync if present
		int n_oca_sync = 0;
		int first_idx = 0;
		int last_idx = 0;
		bool first_idx_set = false;
		for (int i = 0; i < input_gzm->n_oca_sync; i++)
		{
			if (input_gzm->oca_sync[i].frame_idx >= frame_start) {
				if (input_gzm->oca_sync[i].frame_idx <= frame_end) { 
					if(!first_idx_set) {
						first_idx = i;
						first_idx_set = true;
					}
					input_gzm->oca_sync[i].frame_idx -= frame_start;
					n_oca_sync++;
				}
				else { 
					last_idx = i - 1;
					break;
				}
			}
		}
		output_gzm->n_oca_sync = n_oca_sync;
		output_gzm->oca_sync = malloc(output_gzm->n_oca_sync * sizeof(struct movie_oca_sync));
		memcpy(&output_gzm->oca_sync[0], &input_gzm->oca_sync[first_idx], (last_idx - first_idx + 1) * sizeof(struct movie_oca_sync));
	}
		
	if (input_gzm->n_room_load != 0 && input_gzm->room_load != NULL) { 
		// Copy room load if present
		int n_room_load = 0;
		int first_idx = 0;
		int last_idx = 0;
		bool first_idx_set = false;
		for (int i = 0; i < input_gzm->n_room_load; i++)
		{
			if (input_gzm->room_load[i].frame_idx >= frame_start) {
				if (input_gzm->room_load[i].frame_idx <= frame_end) { 
					if(!first_idx_set) {
						first_idx = i;
						first_idx_set = true;
					}
					input_gzm->room_load[i].frame_idx -= frame_start;
					n_room_load++;
				}
				else { 
					last_idx = i - 1;
					break;
				}
			}
		}
		output_gzm->n_room_load = n_room_load;
		output_gzm->room_load = malloc(output_gzm->n_room_load * sizeof(struct movie_room_load));
		memcpy(&output_gzm->room_load[0], &input_gzm->room_load[first_idx], (last_idx - first_idx + 1) * sizeof(struct movie_room_load));
	}
    output_gzm->rerecords = input_gzm->rerecords; // TODO how to get this accurately if at all
    output_gzm->last_recorded_frame = frame_end - frame_start;
    return 0;
}

void
gzm_print_pad (const z64_controller_t *cont)
{
    uint16_t pad = cont->pad;

    printf("{ %4d, %4d }, %s %s %s %s %s %s %s %s %s %s %s %s %s %s%s\n", cont->x, cont->y,
        PAD_A(pad)   ? "A " : "  ",
        PAD_B(pad)   ? "B " : "  ",
        PAD_Z(pad)   ? "Z " : "  ",
        PAD_S(pad)   ? "S " : "  ",
        PAD_DU(pad)  ? "DU" : "  ",
        PAD_DD(pad)  ? "DD" : "  ",
        PAD_DL(pad)  ? "DL" : "  ",
        PAD_DR(pad)  ? "DR" : "  ",
        PAD_L(pad)   ? "L " : "  ",
        PAD_R(pad)   ? "R " : "  ",
        PAD_CU(pad)  ? "CU" : "  ",
        PAD_CD(pad)  ? "CD" : "  ",
        PAD_CL(pad)  ? "CL" : "  ",
        PAD_CR(pad)  ? "CR" : "  ",
        PAD_RST(pad) ? " [RESET]" : "");
}

void
gzm_print_stats (const struct gz_macro *gzm)
{
    printf("n_input: %d\n", gzm->n_input);
    printf("n_seed: %d\n", gzm->n_seed);

    printf("n_oca_input: %d\n", gzm->n_oca_input);
    printf("n_oca_sync: %d\n", gzm->n_oca_sync);
    printf("n_room_load: %d\n", gzm->n_room_load);

    printf("rerecords: %d\n", gzm->rerecords);
    printf("last_recorded_frame: %d\n", gzm->last_recorded_frame);
}

void
gzm_print_input (const struct gz_macro *gzm, const int i)
{
    if (i > gzm->n_input)
        return;
    
    struct movie_input *inp = &gzm->input[i];
    printf("{ 0x%04X }, ", inp->pad_delta); gzm_print_pad(&inp->raw);
}

void
gzm_print_inputs (const struct gz_macro *gzm)
{
    printf("gzm has %u inputs:\n", gzm->n_input);

    for (int i = 0; i < gzm->n_input; i++)
        gzm_print_input(gzm, i);
}

void
gzm_print_seeds (const struct gz_macro *gzm)
{
    printf("gzm has %u seeds:\n", gzm->n_seed);

    struct movie_seed *seeds = gzm->seed;
    for (int i = 0; i < gzm->n_seed; i++)
        printf("  frame: %u, old: %08x, new: %08x\n", seeds[i].frame_idx, seeds[i].old_seed, seeds[i].new_seed);
}
