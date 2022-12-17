#ifndef GZM_H_
#define GZM_H_

#include <stdint.h>

#define PAD_A(pad)   (((pad) >> 15) & 1)
#define PAD_B(pad)   (((pad) >> 14) & 1)
#define PAD_Z(pad)   (((pad) >> 13) & 1)
#define PAD_S(pad)   (((pad) >> 12) & 1)
#define PAD_DU(pad)  (((pad) >> 11) & 1)
#define PAD_DD(pad)  (((pad) >> 10) & 1)
#define PAD_DL(pad)  (((pad) >>  9) & 1)
#define PAD_DR(pad)  (((pad) >>  8) & 1)
#define PAD_RST(pad) (((pad) >>  7) & 1) // reset signal
#define PAD_L(pad)   (((pad) >>  5) & 1)
#define PAD_R(pad)   (((pad) >>  4) & 1)
#define PAD_CU(pad)  (((pad) >>  3) & 1)
#define PAD_CD(pad)  (((pad) >>  2) & 1)
#define PAD_CL(pad)  (((pad) >>  1) & 1)
#define PAD_CR(pad)  (((pad) >>  0) & 1)

typedef struct
{
  uint16_t          pad;                        /* 0x0000 */
  int8_t            x;                          /* 0x0002 */
  int8_t            y;                          /* 0x0003 */
                                                /* 0x0004 */
} z64_controller_t;

struct movie_input
{
  z64_controller_t      raw;                    /* 0x0000 */
  uint16_t              pad_delta;              /* 0x0004 */
                                                /* 0x0006 */
};

struct movie_seed
{
  int                   frame_idx;              /* 0x0000 */
  uint32_t              old_seed;               /* 0x0004 */
  uint32_t              new_seed;               /* 0x0008 */
                                                /* 0x000C */
};

struct movie_oca_input
{
  int32_t               frame_idx;              /* 0x0000 */
  uint16_t              pad;                    /* 0x0004 */
  int8_t                adjusted_x;             /* 0x0006 */
  int8_t                adjusted_y;             /* 0x0007 */
                                                /* 0x0008 */
};

struct movie_oca_sync
{
  int32_t               frame_idx;              /* 0x0000 */
  int32_t               audio_frames;           /* 0x0004 */
                                                /* 0x0008 */
};

struct movie_room_load
{
  int32_t               frame_idx;              /* 0x0000 */
                                                /* 0x0004 */
};

struct gz_macro
{
    uint32_t                 n_input;
    uint32_t                 n_seed;
    z64_controller_t         input_start;
    struct movie_input      *input;              // length n_input
    struct movie_seed       *seed;               // length n_seed
// the following may not exist unless it was needed
    uint32_t                 n_oca_input;
    uint32_t                 n_oca_sync;
    uint32_t                 n_room_load;
    struct movie_oca_input  *oca_input;          // length n_oca_input
    struct movie_oca_sync   *oca_sync;           // length n_oca_sync
    struct movie_room_load  *room_load;          // length n_room_load
// the following may not exist in earlier versions
    uint32_t                 rerecords;
    uint32_t                 last_recorded_frame;
};

#define GZM_SERIAL_SIZE(gzm)                                \
   (sizeof((gzm)->n_input) +                                \
    sizeof((gzm)->n_seed) +                                 \
    sizeof((gzm)->input_start) +                            \
    (gzm)->n_input * sizeof(struct movie_input) +           \
    (gzm)->n_seed * sizeof(struct movie_seed) +             \
    sizeof((gzm)->n_oca_input) +                            \
    sizeof((gzm)->n_oca_sync) +                             \
    sizeof((gzm)->n_room_load) +                            \
    (gzm)->n_oca_input * sizeof(struct movie_oca_input) +   \
    (gzm)->n_oca_sync * sizeof(struct movie_oca_sync) +     \
    (gzm)->n_room_load * sizeof(struct movie_room_load) +   \
    sizeof((gzm)->rerecords) +                              \
    sizeof((gzm)->last_recorded_frame))

// File IO

int
gzm_read (struct gz_macro *gzm, const char *file_name);

int
gzm_write (const struct gz_macro *gzm, const char *file_name);

// New/Free

int
gzm_new (struct gz_macro *gzm);

int
gzm_free (struct gz_macro *gzm);

// Transformations

int
gzm_dup (struct gz_macro *gzm_out, const struct gz_macro *gzm_in);

int
gzm_trim (struct gz_macro *gzm, uint32_t end);

int
gzm_cat (struct gz_macro *gzm, const struct gz_macro *gzm1, const struct gz_macro *gzm2);

int
gzm_cat_r (struct gz_macro *gzm, const struct gz_macro *gzm1, const struct gz_macro *gzm2);

// Printing

void
gzm_print_pad (const z64_controller_t *cont);

void
gzm_print_stats (const struct gz_macro *gzm);

void
gzm_print_input (const struct gz_macro *gzm, const int i);

void
gzm_print_inputs (const struct gz_macro *gzm);

void
gzm_print_seeds (const struct gz_macro *gzm);

#endif
