#ifndef PFD_H
#define PFD_H

#include <lvgl/lvgl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)

#define SCR_WIDTH 1500
#define SCR_HEIGHT 900
#define R (SCR_WIDTH + SCR_HEIGHT)

/* tape dimensions */
#define TAPE_WIDTH 100
#define TAPE_HEIGHT (SCR_HEIGHT * (0.75))

/* tape location */
#define TAPE_LOC_LEFT 0
#define TAPE_LOC_RIGHT 1

/* tape info */
#define TAPE_INFO_SPEED 0
#define TAPE_INFO_ALTITUDE 1
#define TAPE_INFO_HEADING 2
#define TAPE_INFO_ACCEL 3

/* on-tape pointer coords */
// right-hand side of the tape
#define POINTER_RIGHT_X1(x) ((x) + TAPE_WIDTH - 5)
#define POINTER_RIGHT_Y1(y) ((y) + (TAPE_HEIGHT / 2) - 2)
#define POINTER_RIGHT_X2(x) ((x) + TAPE_WIDTH + 15)
#define POINTER_RIGHT_Y2(y) ((y) + (TAPE_HEIGHT / 2) + 2)
// left-hand side of the tape
#define POINTER_LEFT_X1(x) ((x) - 15)
#define POINTER_LEFT_Y1(y) ((y) + (TAPE_HEIGHT / 2) - 2)
#define POINTER_LEFT_X2(x) ((x) + 5)
#define POINTER_LEFT_Y2(y) ((y) + (TAPE_HEIGHT / 2) + 2)

/* functions */
static void draw_horizon(lv_layer_t *layer, int32_t w, int32_t h, float pitch, float roll);
static void draw_chevron(lv_layer_t *layer, int32_t w, int32_t h);
static void create_side_tape(int x, int y, lv_layer_t *layer, int tape_loc,
		      int tape_info, int tape_step, double ppu);
static void create_heading_tape(lv_layer_t *layer, int32_t w, int32_t h, float heading);
static void pfd_draw(lv_event_t *e);

#endif // PFD_H
