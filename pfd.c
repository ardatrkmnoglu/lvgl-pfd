#include "include/pfd.h"

double current_roll = 0.0f;
double current_pitch = 0.0f;
double current_acceleration = 0.0f;
double current_speed = 0.0f;
double current_altitude = 0.0f;
double current_heading = 0.0f;

static void draw_horizon(lv_layer_t *layer, int32_t w, int32_t h, float pitch, float roll) {
	int cx = w / 2;
	int cy = h / 2;
	cy += (int)pitch;

	double rad = DEG_TO_RAD(roll);
	float cos_r = cosf(rad);
	float sin_r = sinf(rad);

	float p_TL_x = -R, p_TL_y = -R;
	float p_TR_x = R, p_TR_y = -R;
	float p_ML_x = -R, p_ML_y = 0;
	float p_MR_x = R, p_MR_y = 0;
	float p_BL_x = -R, p_BL_y = R;
	float p_BR_x = R, p_BR_y = R;

#define ROT_X(x, y) (int)((x * cos_r) - (y * sin_r)) + cx
#define ROT_Y(x, y) (int)((x * sin_r) + (y * cos_r)) + cy

	lv_point_precise_t pt_TL = {ROT_X(p_TL_x, p_TL_y),
				    ROT_Y(p_TL_x, p_TL_y)};
	lv_point_precise_t pt_TR = {ROT_X(p_TR_x, p_TR_y),
				    ROT_Y(p_TR_x, p_TR_y)};
	lv_point_precise_t pt_ML = {ROT_X(p_ML_x, p_ML_y),
				    ROT_Y(p_ML_x, p_ML_y)};
	lv_point_precise_t pt_MR = {ROT_X(p_MR_x, p_MR_y),
				    ROT_Y(p_MR_x, p_MR_y)};
	lv_point_precise_t pt_BL = {ROT_X(p_BL_x, p_BL_y),
				    ROT_Y(p_BL_x, p_BL_y)};
	lv_point_precise_t pt_BR = {ROT_X(p_BR_x, p_BR_y),
				    ROT_Y(p_BR_x, p_BR_y)};

	// sky
	lv_draw_triangle_dsc_t sky_dsc;
	lv_draw_triangle_dsc_init(&sky_dsc);
	sky_dsc.color = lv_color_hex(0x0055ff);
	sky_dsc.opa = LV_OPA_COVER;

	sky_dsc.p[0] = pt_TL;
	sky_dsc.p[1] = pt_TR;
	sky_dsc.p[2] = pt_ML;
	lv_draw_triangle(layer, &sky_dsc);

	sky_dsc.p[0] = pt_TR;
	sky_dsc.p[1] = pt_MR;
	sky_dsc.p[2] = pt_ML;
	lv_draw_triangle(layer, &sky_dsc);

	// earth
	lv_draw_triangle_dsc_t ground_dsc;
	lv_draw_triangle_dsc_init(&ground_dsc);
	ground_dsc.color = lv_color_hex(0x8b4513);
	ground_dsc.opa = LV_OPA_COVER;

	ground_dsc.p[0] = pt_ML;
	ground_dsc.p[1] = pt_MR;
	ground_dsc.p[2] = pt_BL;
	lv_draw_triangle(layer, &ground_dsc);

	ground_dsc.p[0] = pt_MR;
	ground_dsc.p[1] = pt_BR;
	ground_dsc.p[2] = pt_BL;
	lv_draw_triangle(layer, &ground_dsc);

	// horizon
	lv_draw_line_dsc_t horizon_dsc;
	lv_draw_line_dsc_init(&horizon_dsc);
	horizon_dsc.color = lv_color_hex(0xffffff);
	horizon_dsc.width = (double)2.5;
	horizon_dsc.p1 = pt_ML;
	horizon_dsc.p2 = pt_MR;
	lv_draw_line(layer, &horizon_dsc);

#undef ROT_X
#undef ROT_Y
}

static void draw_chevron(lv_layer_t *layer, int32_t w, int32_t h) {
	int mcx = w / 2;
	int mcy = h / 2;

	lv_draw_line_dsc_t aircraft_dsc;
	lv_draw_line_dsc_init(&aircraft_dsc);
	aircraft_dsc.color = lv_color_hex(0xffffff);
	aircraft_dsc.width = 5;

	// chevron wings (--)
	aircraft_dsc.p1 = (lv_point_precise_t){mcx - 60, mcy};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx - 20, mcy};
	lv_draw_line(layer, &aircraft_dsc);

	aircraft_dsc.p1 = (lv_point_precise_t){mcx + 20, mcy};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx + 60, mcy};
	lv_draw_line(layer, &aircraft_dsc);

	aircraft_dsc.p1 = (lv_point_precise_t){mcx, mcy - 5};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx, mcy + 5};
	lv_draw_line(layer, &aircraft_dsc);
}

static void create_side_tape(int x, int y, lv_layer_t *layer, int tape_loc,
		      int tape_info, int tape_step, double ppu) {
	lv_area_t tape_area = {x, y, x + TAPE_WIDTH, y + TAPE_HEIGHT};

	lv_area_t initial_clip = layer->_clip_area;
	layer->_clip_area = tape_area;

	lv_draw_rect_dsc_t bg_dsc;
	lv_draw_rect_dsc_init(&bg_dsc);
	bg_dsc.bg_color = lv_color_hex(0x111111);
	bg_dsc.bg_opa = LV_OPA_80;
	lv_draw_rect(layer, &bg_dsc, &tape_area);

	lv_draw_rect_dsc_t pointer_dsc;
	lv_draw_rect_dsc_init(&pointer_dsc);
	pointer_dsc.bg_color = lv_color_hex(0x00ff00);

	lv_draw_line_dsc_t line_dsc;
	lv_draw_line_dsc_init(&line_dsc);
	line_dsc.color = lv_color_hex(0xffffff);
	line_dsc.width = 2;

	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init(&label_dsc);
	label_dsc.color = lv_color_hex(0xffffff);
	label_dsc.font = &lv_font_montserrat_18;

	double current_val = 0;
	int diff = 0;
	switch (tape_info) {
	case TAPE_INFO_SPEED:
		current_val = current_speed;
		diff = 180;
		break;
	case TAPE_INFO_ALTITUDE:
		current_val = current_altitude;
		diff = 700;
		break;
	case TAPE_INFO_HEADING:
		current_val = current_heading;
		diff = 60;
		break;
	case TAPE_INFO_ACCEL:
		current_val = current_acceleration;
		diff = 100;
		break;
	default:
		break;
	}

	int min_val = ((int)current_val - diff) / tape_step * tape_step;
	int max_val = ((int)current_val + diff) / tape_step * tape_step;

	for (int v = min_val; v <= max_val; v += tape_step) {
		if (v < 0)
			continue;

		int delta_v = v - (int)current_val;
		int y_pos = (y + (TAPE_HEIGHT / 2)) - (delta_v * ppu);

		if (tape_loc == TAPE_LOC_LEFT) {
			line_dsc.p1 =
			    (lv_point_precise_t){x + TAPE_WIDTH - 20, y_pos};
			line_dsc.p2 =
			    (lv_point_precise_t){x + TAPE_WIDTH, y_pos};
		} else if (tape_loc == TAPE_LOC_RIGHT) {
			line_dsc.p1 = (lv_point_precise_t){x, y_pos};
			line_dsc.p2 = (lv_point_precise_t){x + 20, y_pos};
		}
		lv_draw_line(layer, &line_dsc);

		char buf[16];
		sprintf(buf, "%d", v);

		lv_area_t text_area;
		if (tape_loc == TAPE_LOC_LEFT) {
			text_area.x1 = x + 10;
			text_area.y1 = y_pos - 9;
			text_area.x2 = x + TAPE_WIDTH - 25;
			text_area.y2 = y_pos + 10;
		} else if (tape_loc == TAPE_LOC_RIGHT) {
			text_area.x1 = x + 25;
			text_area.y1 = y_pos - 7;
			text_area.x2 = x + TAPE_WIDTH;
			text_area.y2 = y_pos + 10;
		}
		label_dsc.text = buf;
		lv_draw_label(layer, &label_dsc, &text_area);
	}
	layer->_clip_area = initial_clip;

	lv_area_t pointer_area;
	if (tape_loc == TAPE_LOC_LEFT) {
		pointer_area.x1 = POINTER_RIGHT_X1(x);
		pointer_area.y1 = POINTER_RIGHT_Y1(y);
		pointer_area.x2 = POINTER_RIGHT_X2(x);
		pointer_area.y2 = POINTER_RIGHT_Y2(y);
	} else if (tape_loc == TAPE_LOC_RIGHT) {
		pointer_area.x1 = POINTER_LEFT_X1(x);
		pointer_area.y1 = POINTER_LEFT_Y1(y);
		pointer_area.x2 = POINTER_LEFT_X2(x);
		pointer_area.y2 = POINTER_LEFT_Y2(y);
	}
	lv_draw_rect(layer, &pointer_dsc, &pointer_area);
}

static void create_heading_tape(lv_layer_t *layer, int32_t w, int32_t h, float heading) {
	int32_t hdg_tape_w = 400;
	int32_t hdg_tape_h = 40;
	int32_t hdg_tape_x = (w - hdg_tape_w) / 2;
	int32_t hdg_tape_y = h - hdg_tape_h - 20;
	int32_t hdg_tape_cx = hdg_tape_x + (hdg_tape_w / 2);

	lv_area_t hdg_area = {hdg_tape_x,
			      hdg_tape_y,
			      hdg_tape_x + hdg_tape_w,
			      hdg_tape_y + hdg_tape_h};

	lv_area_t orig_clip = layer->_clip_area;
	layer->_clip_area = hdg_area;

	lv_draw_rect_dsc_t hdg_bg_dsc;
	lv_draw_rect_dsc_init(&hdg_bg_dsc);
	hdg_bg_dsc.bg_color = lv_color_hex(0x111111);
	hdg_bg_dsc.bg_opa = LV_OPA_80;
	lv_draw_rect(layer, &hdg_bg_dsc, &hdg_area);

	int32_t hdg_step = 10;
	float hdg_px_per_deg = 4.0f;

	int32_t h_min = ((int32_t)heading - 60) / hdg_step * hdg_step;
	int32_t h_max = ((int32_t)heading + 60) / hdg_step * hdg_step;

	lv_draw_line_dsc_t hdg_line_dsc;
	lv_draw_line_dsc_init(&hdg_line_dsc);
	hdg_line_dsc.color = lv_color_hex(0xFFFFFF);
	hdg_line_dsc.width = 2;

	lv_draw_label_dsc_t hdg_label_dsc;
	lv_draw_label_dsc_init(&hdg_label_dsc);
	hdg_label_dsc.color = lv_color_hex(0xFFFFFF);

	for (int32_t v = h_min; v <= h_max; v += hdg_step) {
		int32_t display_val = v;
		while (display_val < 0)
			display_val += 360;
		while (display_val >= 360)
			display_val -= 360;

		int32_t delta_v = v - (int32_t)heading;
		int32_t x_pos = hdg_tape_cx + (int32_t)(delta_v * hdg_px_per_deg);

		hdg_line_dsc.p1 = (lv_point_precise_t){x_pos, hdg_tape_y};
		hdg_line_dsc.p2 = (lv_point_precise_t){x_pos, hdg_tape_y + 10};
		lv_draw_line(layer, &hdg_line_dsc);

		if (display_val % 30 == 0) {
			char buf[4];
			if (display_val == 0)
				sprintf(buf, "N");
			else if (display_val == 90)
				sprintf(buf, "E");
			else if (display_val == 180)
				sprintf(buf, "S");
			else if (display_val == 270)
				sprintf(buf, "W");
			else
				sprintf(buf, "%d", display_val / 10);

			lv_area_t txt_area = {x_pos - 15, hdg_tape_y + 15,
					      x_pos + 15, hdg_tape_y + 35};
			hdg_label_dsc.text = buf;
			lv_draw_label(layer, &hdg_label_dsc, &txt_area);
		}
	}
	layer->_clip_area = orig_clip;

	hdg_line_dsc.color = lv_color_hex(0xFFFF00);
	hdg_line_dsc.width = 3;
	hdg_line_dsc.p1 = (lv_point_precise_t){hdg_tape_cx, hdg_tape_y - 5};
	hdg_line_dsc.p2 = (lv_point_precise_t){hdg_tape_cx, hdg_tape_y + 15};
	lv_draw_line(layer, &hdg_line_dsc);

	int exact_hdg = (int)heading;
	while (exact_hdg < 0)
		exact_hdg += 360;
	while (exact_hdg >= 360)
		exact_hdg -= 360;

	char buf_exact_hdg[8];
	sprintf(buf_exact_hdg, "%03d°", exact_hdg);

	lv_area_t ehdg_area = {hdg_tape_cx - 20, hdg_tape_y - 28,
			       hdg_tape_cx + 20, hdg_tape_y - 5};

	lv_draw_rect_dsc_t ehdg_bg_dsc;
	lv_draw_rect_dsc_init(&ehdg_bg_dsc);
	ehdg_bg_dsc.bg_color = lv_color_hex(0x111111);
	ehdg_bg_dsc.bg_opa = LV_OPA_COVER;
	ehdg_bg_dsc.radius = 3;
	lv_draw_rect(layer, &ehdg_bg_dsc, &ehdg_area);

	lv_draw_label_dsc_t ehdg_label_dsc;
	lv_draw_label_dsc_init(&ehdg_label_dsc);
	ehdg_label_dsc.color = lv_color_hex(0xffffff);
	ehdg_label_dsc.align = LV_TEXT_ALIGN_CENTER;

	lv_area_t ehdg_text_area = {hdg_tape_cx - 20, hdg_tape_y - 25,
				    hdg_tape_cx + 20, hdg_tape_y - 5};
	ehdg_label_dsc.text = buf_exact_hdg;
	lv_draw_label(layer, &ehdg_label_dsc, &ehdg_text_area);
}

static void pfd_draw(lv_event_t *e) {
	lv_obj_t *obj = lv_event_get_target(e);
	lv_layer_t *layer = lv_event_get_layer(e);

	int32_t w = lv_obj_get_width(obj);
	int32_t h = lv_obj_get_height(obj);

	draw_horizon(layer, w, h, current_pitch, current_roll);

	create_heading_tape(layer, w, h, current_heading);

	create_side_tape(20, (lv_obj_get_height(obj) - TAPE_HEIGHT) / 2, layer,
			 TAPE_LOC_LEFT, TAPE_INFO_SPEED, 10, 2);
	create_side_tape(lv_obj_get_width(obj) - TAPE_WIDTH - 20,
			 (lv_obj_get_height(obj) - TAPE_HEIGHT) / 2, layer,
			 TAPE_LOC_RIGHT, TAPE_INFO_ALTITUDE, 100, 0.5);

	draw_chevron(layer, w, h);
}

int main() {
	lv_init();

	lv_display_t *dpy = lv_sdl_window_create(SCR_WIDTH, SCR_HEIGHT);

	lv_obj_t *pfd_screen = lv_obj_create(lv_screen_active());
	lv_obj_set_size(pfd_screen, SCR_WIDTH, SCR_HEIGHT);
	lv_obj_center(pfd_screen);

	lv_obj_set_style_bg_color(pfd_screen, lv_color_hex(0x000000), 0);
	lv_obj_set_style_border_width(pfd_screen, 0, 0);
	lv_obj_set_style_radius(pfd_screen, 0, 0);

	lv_obj_add_event_cb(pfd_screen, pfd_draw, LV_EVENT_DRAW_MAIN, NULL);

	// initial mock data for simulation
	current_roll = 0.0;
	current_pitch = 0.0;
	current_speed = 512.5;
	current_altitude = 10150.0;
	current_heading = 274.0;
	current_acceleration = 50;

	// pfd simulation
	while (1) {
		static double t = 0;
		t += 0.05f;
		current_acceleration = 50 - (5 * t);

		if (t > 10)
			current_acceleration = -0.25;
		if (current_speed < 0)
			current_speed = 0;
		current_roll = sinf(t) * 30.0f; // roll: -30 ~ +30°
		current_pitch = cosf(t) * 40.0f; // pitch: -40 ~ +40 px
		current_speed += current_acceleration; // speed: 512.5 + (50*t - 5*t^2) km/h
		current_altitude += 0.1 * current_pitch;
		current_heading += sinf(t) * 2;
		lv_obj_invalidate(pfd_screen);
		lv_timer_handler();
		usleep(20000);
	}
	return 0;
}
