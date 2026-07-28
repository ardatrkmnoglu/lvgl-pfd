#include "include/pfd.h"
#include <lvgl/font/lv_text.h>

static double current_roll = 0.0f;
static double current_pitch = 0.0f;
static double current_acceleration = 0.0f;
static double current_speed = 0.0f;
static double current_altitude = 0.0f;
static double current_heading = 0.0f;
static int current_status = 0;

static char *status_msg1 = "FMC SPD";
static char *status_msg2 = "LNAV";
static char *status_msg3 = "TAXI";

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

static void draw_pitch_ladder(lv_layer_t *layer, int32_t w, int32_t h, float pitch, float roll, int ppu) {
	int cx = w / 2;
	int cy = h / 2 + (int)pitch;

	double rad = DEG_TO_RAD(roll);
	float cos_r = cosf(rad);
	float sin_r = sinf(rad);

#define ROT_X(x, y) (int)((x * cos_r) - (y * sin_r)) + cx
#define ROT_Y(x, y) (int)((x * sin_r) + (y * cos_r)) + cy

	lv_area_t ladder_clip = {140, 320, w - 140, h - 320};
	lv_area_t initial_clip = layer->_clip_area;
	layer->_clip_area = ladder_clip;

	lv_draw_line_dsc_t line_dsc;
	lv_draw_line_dsc_init(&line_dsc);
	line_dsc.color = lv_color_hex(0xffffff);
	line_dsc.width = 3;

	lv_draw_label_dsc_t label_dsc;
	lv_draw_label_dsc_init(&label_dsc);
	label_dsc.color = lv_color_hex(0xffffff);
	label_dsc.font = &lv_font_montserrat_18;
	label_dsc.align = LV_TEXT_ALIGN_CENTER;

	for (int v = -30; v <= 30; v += 5) {
		if (v == 0)
			continue;

		int y_offset = -v * ppu;

		int half_w = (v % 10 == 0) ? 90 : 40;

		line_dsc.p1 = (lv_point_precise_t){ROT_X(-half_w, y_offset),
						   ROT_Y(-half_w, y_offset)};
		line_dsc.p2 = (lv_point_precise_t){ROT_X(half_w, y_offset),
						   ROT_Y(half_w, y_offset)};
		lv_draw_line(layer, &line_dsc);

		if (v % 10 == 0) {
			char buf[8];
			sprintf(buf, "%d", abs(v));

			int tx_L = -half_w - 20;
			int tx_R = half_w + 20;

			lv_point_precise_t t_pt_L = {ROT_X(tx_L, y_offset),
						     ROT_Y(tx_L, y_offset)};
			lv_point_precise_t t_pt_R = {ROT_X(tx_R, y_offset),
						     ROT_Y(tx_R, y_offset)};

			lv_area_t txt_area_L = {t_pt_L.x - 20, t_pt_L.y - 10,
						t_pt_L.x + 20, t_pt_L.y + 10};
			label_dsc.text = buf;
			lv_draw_label(layer, &label_dsc, &txt_area_L);

			lv_area_t txt_area_R = {t_pt_R.x - 20, t_pt_R.y - 10,
						t_pt_R.x + 20, t_pt_R.y + 10};
			lv_draw_label(layer, &label_dsc, &txt_area_R);
		} else {
			double quart_down = v - 2.5;
			double quart_up = v + 2.5;

			double q_offset_d = -quart_down * ppu;
			double q_offset_u = -quart_up * ppu;

			line_dsc.p1 = (lv_point_precise_t){ROT_X(-20, q_offset_d),
							   ROT_Y(-20, q_offset_d)};
			line_dsc.p2 = (lv_point_precise_t){ROT_X(20, q_offset_d),
							   ROT_Y(20, q_offset_d)};
			lv_draw_line(layer, &line_dsc);

			line_dsc.p1 = (lv_point_precise_t){ROT_X(-20, q_offset_u),
							   ROT_Y(-20, q_offset_u)};
			line_dsc.p2 = (lv_point_precise_t){ROT_X(20, q_offset_u),
							   ROT_Y(20, q_offset_u)};
			lv_draw_line(layer, &line_dsc);

		}
	}
	layer->_clip_area = initial_clip;

#undef ROT_X
#undef ROT_Y
}

static void draw_chevron(lv_layer_t *layer, int32_t w, int32_t h) {
	int mcx = w / 2;
	int mcy = h / 2;

	lv_draw_line_dsc_t aircraft_dsc;
	lv_draw_line_dsc_init(&aircraft_dsc);
	aircraft_dsc.color = lv_color_hex(0xffffff);
	aircraft_dsc.width = 8;

	// chevron wings (--)
	aircraft_dsc.p1 = (lv_point_precise_t){mcx - 120, mcy};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx - 40, mcy};
	lv_draw_line(layer, &aircraft_dsc);
	aircraft_dsc.p1 = (lv_point_precise_t){mcx - 40, mcy - (aircraft_dsc.width / 2.0)};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx - 40, mcy + 24};
	lv_draw_line(layer, &aircraft_dsc);

	aircraft_dsc.p1 = (lv_point_precise_t){mcx + 40, mcy};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx + 120, mcy};
	lv_draw_line(layer, &aircraft_dsc);
	aircraft_dsc.p1 = (lv_point_precise_t){mcx + 40, mcy - (aircraft_dsc.width / 2.0)};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx + 40, mcy + 24};
	lv_draw_line(layer, &aircraft_dsc);

	aircraft_dsc.p1 = (lv_point_precise_t){mcx, mcy - 5};
	aircraft_dsc.p2 = (lv_point_precise_t){mcx, mcy + 5};
	lv_draw_line(layer, &aircraft_dsc);
}

static void create_side_tape(lv_layer_t *layer, int x, int y, int tape_loc,
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
		diff = SCR_HEIGHT / 5;
		break;
	case TAPE_INFO_ALTITUDE:
		current_val = current_altitude;
		diff = SCR_HEIGHT;
		break;
	case TAPE_INFO_HEADING:
		current_val = current_heading;
		diff = SCR_WIDTH / 20;
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

	char buf[8];
	sprintf(buf, "%.1f", current_val);

	lv_area_t val_area = {tape_loc == TAPE_LOC_LEFT ? (x + TAPE_WIDTH + 15) : (x - 115),
			      y + (TAPE_HEIGHT / 2) - 30,
			      tape_loc == TAPE_LOC_LEFT ? (x + TAPE_WIDTH + 105) : (x - 15),
			      y + (TAPE_HEIGHT / 2) + 30};

	lv_draw_rect_dsc_t val_bg_dsc;
	lv_draw_rect_dsc_init(&val_bg_dsc);
	val_bg_dsc.bg_color = lv_color_hex(0x111111);
	val_bg_dsc.bg_opa = LV_OPA_COVER;
	val_bg_dsc.radius = 3;
	lv_draw_rect(layer, &val_bg_dsc, &val_area);

	lv_area_t val_text_area = {tape_loc == TAPE_LOC_LEFT ? (x + TAPE_WIDTH + 15) : (x - 115),
			      y + (TAPE_HEIGHT / 2) - 15,
			      tape_loc == TAPE_LOC_LEFT ? (x + TAPE_WIDTH + 105) : (x - 15),
			      y + (TAPE_HEIGHT / 2) + 15};

	label_dsc.font = &lv_font_montserrat_24;
	label_dsc.align = LV_TEXT_ALIGN_CENTER;
	label_dsc.text = buf;
	lv_draw_label(layer, &label_dsc, &val_text_area);

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

static void print_fma(lv_layer_t *layer, const char *msg1, const char *msg2, const char *msg3) {
	int cx = SCR_WIDTH / 2;
	char buf1[16];
	char buf2[16];
	char buf3[16];
	sprintf(buf1, "%s", msg1);
	sprintf(buf2, "%s", msg2);
	sprintf(buf3, "%s", msg3);

	lv_draw_rect_dsc_t fma_bg_dsc;
	lv_draw_rect_dsc_init(&fma_bg_dsc);
	fma_bg_dsc.bg_color = lv_color_hex(0x313131);
	fma_bg_dsc.bg_opa = LV_OPA_70;

	lv_area_t fma1_area = {cx - 400, 25,
			       cx - 150, 100};
	lv_draw_rect(layer, &fma_bg_dsc, &fma1_area);

	lv_area_t fma2_area = {cx - 125, 25,
			       cx + 125, 100};
	lv_draw_rect(layer, &fma_bg_dsc, &fma2_area);

	lv_area_t fma3_area = {cx + 150, 25,
			       cx + 400, 100};
	lv_draw_rect(layer, &fma_bg_dsc, &fma3_area);

	lv_draw_label_dsc_t fma_label_dsc;
	lv_draw_label_dsc_init(&fma_label_dsc);
	fma_label_dsc.color = lv_color_hex(0x00ff00);
	fma_label_dsc.align = LV_TEXT_ALIGN_CENTER;
	fma_label_dsc.font = &lv_font_montserrat_28;

	fma_label_dsc.text = msg1;
	lv_draw_label(layer, &fma_label_dsc, &fma1_area);
	fma_label_dsc.text = msg2;
	lv_draw_label(layer, &fma_label_dsc, &fma2_area);
	fma_label_dsc.text = msg3;
	lv_draw_label(layer, &fma_label_dsc, &fma3_area);

}

static void input_event(lv_event_t *e) {
	uint32_t key = lv_event_get_key(e);

	switch (key) {
		case 'w':
			current_pitch -= 1.0f;
			break;
		case 's':
			current_pitch += 1.0f;
			break;
		case 'a':
			current_roll += 0.5f;
			break;
		case 'd':
			current_roll -= 0.5f;
			break;

		case LV_KEY_UP:
			current_speed += 2.5f;
			break;
		case LV_KEY_DOWN:
			current_speed -= 2.5f;
			break;
		case LV_KEY_LEFT:
			if (current_status == STATUS_TAXI)
				current_heading -= 2.0f;
			break;
		case LV_KEY_RIGHT:
			if (current_status == STATUS_TAXI)
				current_heading += 2.0f;
			break;

		case '1':
			current_status = STATUS_TAXI;
			status_msg3 = "TAXI";
			break;
		case '2':
			current_status = STATUS_TAKEOFF;
			status_msg3 = "TKOFF";
			break;
		case '3':
			current_status = STATUS_CRUISE;
			status_msg3 = "CRUISE";
			break;
		case '4':
			current_status = STATUS_LANDING;
			status_msg3 = "LND";
			break;
	}

	if (current_speed < 0) current_speed = 0;

	if (current_roll < -45.0f) current_roll = -45.0f;
	if(current_roll > 45.0f) current_roll = 45.0f;
}

static void pfd_draw(lv_event_t *e) {
	lv_obj_t *obj = lv_event_get_target(e);
	lv_layer_t *layer = lv_event_get_layer(e);

	int32_t w = lv_obj_get_width(obj);
	int32_t h = lv_obj_get_height(obj);

	draw_horizon(layer, w, h, current_pitch, current_roll);

	draw_pitch_ladder(layer, w, h, current_pitch, current_roll, 12);

	create_heading_tape(layer, w, h, current_heading);

	create_side_tape(layer, 20,
			 (lv_obj_get_height(obj) - TAPE_HEIGHT) / 2,
			 TAPE_LOC_LEFT, TAPE_INFO_SPEED, 20, 2);
	create_side_tape(layer, lv_obj_get_width(obj) - TAPE_WIDTH - 20,
			 (lv_obj_get_height(obj) - TAPE_HEIGHT) / 2,
			 TAPE_LOC_RIGHT, TAPE_INFO_ALTITUDE, 100, 0.5);

	print_fma(layer, status_msg1, status_msg2, status_msg3);

	draw_chevron(layer, w, h);
}

int main() {
	lv_init();

	lv_display_t *dpy = lv_sdl_window_create(SCR_WIDTH, SCR_HEIGHT);

	lv_indev_t *kb = lv_sdl_keyboard_create();

	lv_group_t *g = lv_group_create();
	lv_group_set_default(g);
	lv_indev_set_group(kb, g);

	lv_obj_t *screen = lv_obj_create(lv_screen_active());
	lv_obj_set_size(screen, SCR_WIDTH, SCR_HEIGHT);
	lv_obj_center(screen);

	lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
	lv_obj_set_style_border_width(screen, 0, 0);
	lv_obj_set_style_radius(screen, 0, 0);

	lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
	lv_group_add_obj(g, screen);

	lv_obj_add_event_cb(screen, pfd_draw, LV_EVENT_DRAW_MAIN, NULL);
	lv_obj_add_event_cb(screen, input_event, LV_EVENT_KEY, NULL);

	// initial data
	current_roll = 0.0;
	current_pitch = 0.0;
	current_speed = 0.0;
	current_altitude = 75.5;
	current_heading = 0.0;

	// pfd simulation
	while (1) {
		float turn_rate = current_roll * 0.05f;
		current_heading -= turn_rate;
		if(current_heading >= 360.0f) current_heading -= 360.0f;
		if(current_heading < 0.0f) current_heading += 360.0f;

		float climb_rate = (current_pitch * 0.1f) * (current_speed / 200.0f);
		current_altitude += climb_rate;
		if (current_altitude < 0) current_altitude = 0;

		lv_obj_invalidate(screen);
		lv_timer_handler();
		usleep(20000);
	}
	return 0;
}
