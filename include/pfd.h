#ifndef PFD_H
#define PFD_H

#include <lvgl/lvgl.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* mathematical definitions */
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)
#define ROT_X(x, y) (int)(((x) * cos_r) - ((y) * sin_r)) + cx
#define ROT_Y(x, y) (int)(((x) * sin_r) + ((y) * cos_r)) + cy

/* screen dimensions */
#define SCR_WIDTH  480
#define SCR_HEIGHT 272
#define R (SCR_WIDTH + SCR_HEIGHT)

/* tape dimensions */
#define TAPE_WIDTH  (SCR_WIDTH * (0.1))
#define TAPE_HEIGHT (SCR_HEIGHT * (0.75))

/* tape location */
#define TAPE_LOC_LEFT  0
#define TAPE_LOC_RIGHT 1

/* tape info */
#define TAPE_INFO_SPEED    0
#define TAPE_INFO_ALTITUDE 1
#define TAPE_INFO_HEADING  2
#define TAPE_INFO_ACCEL    3

/* on-tape pointer coords */
#define POINTER_PADDING (SCR_WIDTH / 100.0)
// right-hand side of the tape
#define POINTER_RIGHT_X1(x) ((x) + TAPE_WIDTH - (POINTER_PADDING / 3))
#define POINTER_RIGHT_Y1(y) ((y) + (TAPE_HEIGHT / 2) - 2)
#define POINTER_RIGHT_X2(x) ((x) + TAPE_WIDTH + POINTER_PADDING)
#define POINTER_RIGHT_Y2(y) ((y) + (TAPE_HEIGHT / 2) + 2)
// left-hand side of the tape
#define POINTER_LEFT_X1(x) ((x) - POINTER_PADDING)
#define POINTER_LEFT_Y1(y) ((y) + (TAPE_HEIGHT / 2) - 2)
#define POINTER_LEFT_X2(x) ((x) + (POINTER_PADDING / 3))
#define POINTER_LEFT_Y2(y) ((y) + (TAPE_HEIGHT / 2) + 2)

/* status */
#define STATUS_PARK    0
#define STATUS_TAXI    1
#define STATUS_TAKEOFF 2
#define STATUS_CRUISE  3
#define STATUS_LANDING 4

/* fonts */
static lv_font_t *font_b612_20 = NULL;
static lv_font_t *font_b612_mono_10 = NULL;
static lv_font_t *font_b612_mono_12 = NULL;
static lv_font_t *font_b612_mono_14 = NULL;
static lv_font_t *font_b612_mono_16 = NULL;
static lv_font_t *font_b612_mono_18 = NULL;
static lv_font_t *font_b612_mono_20 = NULL;
static lv_font_t *font_b612_mono_22 = NULL;
static lv_font_t *font_b612_mono_24 = NULL;
static lv_font_t *font_b612_mono_bold_12 = NULL;
static lv_font_t *font_b612_mono_bold_14 = NULL;
static lv_font_t *font_b612_mono_bold_16 = NULL;
static lv_font_t *font_b612_mono_bold_18 = NULL;
static lv_font_t *font_b612_mono_bold_20 = NULL;
static lv_font_t *font_b612_mono_bold_22 = NULL;
static lv_font_t *font_b612_mono_bold_24 = NULL;
static lv_font_t *font_b612_mono_bold_26 = NULL;
static lv_font_t *font_b612_mono_bold_28 = NULL;
static lv_font_t *font_b612_mono_bold_30 = NULL;
static lv_font_t *font_b612_mono_bold_32 = NULL;
static lv_font_t *font_b612_mono_bold_34 = NULL;
static lv_font_t *font_b612_mono_bold_36 = NULL;
static lv_font_t *font_b612_mono_bold_38 = NULL;

#define PATH_REGULAR "/home/ardatrkmnoglu/Documents/B612-Regular.ttf"
#define PATH_MONO_REGULAR "/home/ardatrkmnoglu/Documents/B612Mono-Regular.ttf"
#define PATH_MONO_BOLD "/home/ardatrkmnoglu/Documents/B612Mono-Bold.ttf"

#define FONT_TAPE ((SCR_WIDTH >= 1000) ? font_b612_mono_20 : font_b612_mono_10)
#define FONT_TAPE_VAL ((SCR_WIDTH >= 1000) ? font_b612_mono_bold_24 : font_b612_mono_bold_12)
#define FONT_LADDER ((SCR_WIDTH >= 1000) ? font_b612_mono_24 : font_b612_mono_12)
#define FONT_HDG ((SCR_WIDTH >= 1000) ? font_b612_mono_16 : font_b612_mono_10)
#define FONT_FMA ((SCR_WIDTH >= 1000) ? font_b612_mono_bold_32 : font_b612_mono_bold_16)
#define FONT_FLTDIR ((SCR_WIDTH >= 1000) ? font_b612_mono_bold_38 : font_b612_mono_bold_18)

/* colors */
#define COLOR_SKY_BLUE   0x0055ff
#define COLOR_GND_GREEN  0x0e8040
#define COLOR_GND_ORANGE 0x8b4513

/* functions */
static void draw_horizon(lv_layer_t *layer, int32_t w, int32_t h, float pitch, float roll);
static void draw_pitch_ladder(lv_layer_t *layer, int32_t w, int32_t h, float pitch, float roll, int ppu);
static void draw_chevron(lv_layer_t *layer, int32_t w, int32_t h);
static void create_side_tape(lv_layer_t *layer, int x, int y, int tape_loc,
		      int tape_info, int tape_step, double ppu);
static void create_heading_tape(lv_layer_t *layer, int32_t w, int32_t h, float heading);
static void draw_roll_indicator(lv_layer_t *layer, int32_t w, int32_t h, float roll);
static void print_fma(lv_layer_t *layer, const char *msg1, const char *msg2, const char *msg3);
static void pfd_draw(lv_event_t *e);

#endif // PFD_H
