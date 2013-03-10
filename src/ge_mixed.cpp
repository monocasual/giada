/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_mixed
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#include <math.h>
#include "ge_mixed.h"
#include "gd_mainWindow.h"
#include "const.h"
#include "mixer.h"
#include "graphics.h"
#include "recorder.h"
#include "gui_utils.h"
#include "channel.h"


extern Mixer 		     G_Mixer;
extern unsigned 		 G_beats;
extern bool  		     G_audio_status;
extern Patch 		     f_patch;
extern Conf  		     f_conf;
extern gdMainWindow *mainWin;


void __cb_window_closer(Fl_Widget *v, void *p) {
	delete (Fl_Window*)p;
}


/* ------------------------------------------------------------------ */


gButton::gButton(int X,int Y,int W,int H,const char *L, const char **imgOff, const char **imgOn)
	: gClick(X,Y,W,H,L,imgOff,imgOn) {}


/* ------------------------------------------------------------------ */


gStatus::gStatus(int x, int y, int w, int h, channel *ch, const char *L)
: Fl_Box(x, y, w, h, L), ch(ch) {}

void gStatus::draw() {

	fl_rect(x(), y(), w(), h(), COLOR_BD_0);		          // reset border
	fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);		  // reset background

	if (ch != NULL) {
		if (ch->status & (STATUS_WAIT | STATUS_ENDING | REC_ENDING | REC_WAITING)) {
			fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_2);	// status wait
			fl_rect(x(), y(), w(), h(), COLOR_BD_1);
		}
		else if (ch->status == STATUS_PLAY)
			fl_rect(x(), y(), w(), h(), COLOR_BD_1);
		else
			fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // status empty


		if (G_Mixer.chanInput == ch)
			fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_3);	    // take in progress
		else if (recorder::active && recorder::canRec(ch))
			fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_4);     // action record

		/* equation for the progress bar:
		 * ((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

		int mixerPos = G_Mixer.getChanPos(ch);
		if (mixerPos == -1)
			mixerPos = 0;
		else
			mixerPos = (mixerPos * (w()-1)) / (ch->end - ch->start);
		fl_rectf(x()+1, y()+1, mixerPos, h()-2, COLOR_BG_2);
	}
}


/* ------------------------------------------------------------------ */


gClick::gClick(int x, int y, int w, int h, const char *L, const char **imgOff, const char **imgOn)
: Fl_Button(x, y, w, h, L),
	imgOff(imgOff),
	imgOn(imgOn),
	bgColor0(COLOR_BG_0),
	bgColor1(COLOR_BG_1),
	bdColor(COLOR_BD_0),
	txtColor(COLOR_TEXT_0)	{}

void gClick::draw() {

	if (!active()) txtColor = bdColor;
	else 					 txtColor = COLOR_TEXT_0;

	fl_rect(x(), y(), w(), h(), bdColor);             // borders
	if (value()) {													          // -- clicked
		if (imgOn != NULL)
			fl_draw_pixmap(imgOn, x()+1, y()+1);
		else
			fl_rectf(x(), y(), w(), h(), bgColor1);       // covers the border
	}
	else {                                            // -- not clicked
		fl_rectf(x()+1, y()+1, w()-2, h()-2, bgColor0); // bg inside the border
		if (imgOff != NULL)
			fl_draw_pixmap(imgOff, x()+1, y()+1);
	}
	if (!active())
		fl_color(FL_INACTIVE_COLOR);

	fl_color(txtColor);
	fl_font(FL_HELVETICA, 11);
	fl_draw(label(), x(), y(), w(), h(), FL_ALIGN_CENTER);
}


/* ------------------------------------------------------------------ */


gClickRepeat::gClickRepeat(int x, int y, int w, int h, const char *L, const char **imgOff, const char **imgOn)
: Fl_Repeat_Button(x, y, w, h, L), imgOff(imgOff), imgOn(imgOn) {}

void gClickRepeat::draw() {
	if (value()) {															 // -- clicked
		fl_rectf(x(), y(), w(), h(), COLOR_BG_1);  // bg
		if (imgOn != NULL)
			fl_draw_pixmap(imgOn, x()+1, y()+1);
	}
	else {                                       // -- not clicked
		fl_rectf(x(), y(), w(), h(), COLOR_BG_0);  // bg
		fl_rect(x(), y(), w(), h(), COLOR_BD_0);   // border
		if (imgOff != NULL)
			fl_draw_pixmap(imgOff, x()+1, y()+1);
	}
	if (!active())
		fl_color(FL_INACTIVE_COLOR);

	fl_color(COLOR_TEXT_0);
	fl_font(FL_HELVETICA, 11);
	fl_draw(label(), x(), y(), w(), h(), FL_ALIGN_CENTER);
}


/* ------------------------------------------------------------------ */


gInput::gInput(int x, int y, int w, int h, const char *L)
: Fl_Input(x, y, w, h, L) {
	//Fl::set_boxtype(G_BOX, gDrawBox, 1, 1, 2, 2);
	box(G_BOX);
	labelsize(11);
	labelcolor(COLOR_TEXT_0);
	color(COLOR_BG_DARK);
	textcolor(COLOR_TEXT_0);
	cursor_color(COLOR_TEXT_0);
	selection_color(COLOR_BD_0);
	textsize(11);

}


/* ------------------------------------------------------------------ */


gDial::gDial(int x, int y, int w, int h, const char *L)
: Fl_Dial(x, y, w, h, L) {
	labelsize(11);
	labelcolor(COLOR_TEXT_0);
	align(FL_ALIGN_LEFT);
	type(FL_FILL_DIAL);
	angles(0, 360);
	color(COLOR_BG_0);            // background
	selection_color(COLOR_BG_1);   // selection
}

void gDial::draw() {
	double angle = (angle2()-angle1())*(value()-minimum())/(maximum()-minimum()) + angle1();

	fl_color(COLOR_BG_0);
	fl_pie(x(), y(), w(), h(), 270-angle1(), angle > angle1() ? 360+270-angle : 270-360-angle);

	fl_color(COLOR_BD_0);
	fl_arc(x(), y(), w(), h(), 0, 360);
	fl_pie(x(), y(), w(), h(), 270-angle, 270-angle1());
}

/* ------------------------------------------------------------------ */


gBox::gBox(int x, int y, int w, int h, const char *L, Fl_Align al)
: Fl_Box(x, y, w, h, L) {
	labelsize(11);
	box(FL_NO_BOX);
	labelcolor(COLOR_TEXT_0);
	if (al != 0)
		align(al | FL_ALIGN_INSIDE);
}


/* ------------------------------------------------------------------ */


gCheck::gCheck(int x, int y, int w, int h, const char *L)
: Fl_Check_Button(x, y, w, h, L) {}

void gCheck::draw() {

	int color = !active() ? FL_INACTIVE_COLOR : COLOR_BD_0;

	if (value()) {
		fl_rect(x(), y(), 12, 12, (Fl_Color) color);
		fl_rectf(x(), y(), 12, 12, (Fl_Color) color);
	}
	else {
		fl_rectf(x(), y(), 12, 12, FL_BACKGROUND_COLOR);
		fl_rect(x(), y(), 12, 12, (Fl_Color) color);
	}

	fl_rectf(x()+20, y(), w(), h(), FL_BACKGROUND_COLOR);  // clearer
	fl_font(FL_HELVETICA, 11);
	fl_color(COLOR_TEXT_0);
	fl_draw(label(), x()+20, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
}


/* ------------------------------------------------------------------ */


gRadio::gRadio(int x, int y, int w, int h, const char *L)
: Fl_Radio_Button(x, y, w, h, L) {}

void gRadio::draw() {

	int color = !active() ? FL_INACTIVE_COLOR : COLOR_BD_0;

	if (value()) {
		fl_rect(x(), y(), 12, 12, (Fl_Color) color);
		fl_rectf(x(), y(), 12, 12, (Fl_Color) color);
	}
	else {
		fl_rectf(x(), y(), 12, 12, FL_BACKGROUND_COLOR);
		fl_rect(x(), y(), 12, 12, (Fl_Color) color);
	}

	fl_rectf(x()+20, y(), w(), h(), FL_BACKGROUND_COLOR);  // clearer
	fl_font(FL_HELVETICA, 11);
	fl_color(COLOR_TEXT_0);
	fl_draw(label(), x()+20, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
}


/* ------------------------------------------------------------------ */


gProgress::gProgress(int x, int y, int w, int h, const char *L)
: Fl_Progress(x, y, w, h, L) {
	color(COLOR_BG_0, COLOR_BD_0);
	box(G_BOX);

}


/* ------------------------------------------------------------------ */


gSoundMeter::gSoundMeter(int x, int y, int w, int h, const char *L)
	: Fl_Box(x, y, w, h, L),
		clip(false),
		mixerPeak(0.0f),
		peak(0.0f),
		peak_old(0.0f),
		db_level(0.0f),
		db_level_old(0.0f) {}

void gSoundMeter::draw() {

	fl_rect(x(), y(), w(), h(), COLOR_BD_0);

	/* peak = the highest value inside the frame */

	peak = 0.0f;
	float tmp_peak = 0.0f;

	tmp_peak = fabs(mixerPeak);
	if (tmp_peak > peak)
		peak = tmp_peak;

	clip = peak >= 1.0f ? true : false; // 1.0f is considered clip


	/*  dBFS (full scale) calculation, plus decay of -2dB per frame */

	db_level = 20 * log10(peak);
	if (db_level < db_level_old)
		if (db_level_old > -DB_MIN_SCALE)
			db_level = db_level_old - 2.0f;

	db_level_old = db_level;

	/* graphical part */

	float px_level = 0.0f;
	if (db_level < 0.0f)
		px_level = ((w()/DB_MIN_SCALE) * db_level) + w();
	else
		px_level = w();

	fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);
	fl_rectf(x()+1, y()+1, (int) px_level, h()-2, clip || !G_audio_status ? COLOR_ALERT : COLOR_BD_0);
}

/* ------------------------------------------------------------------ */

gBeatMeter::gBeatMeter(int x, int y, int w, int h, const char *L)
	: Fl_Box(x, y, w, h, L) {}

void gBeatMeter::draw() {

	int cursorW = (w()/MAX_BEATS);

	fl_rect(x(), y(), w(), h(), COLOR_BD_0);													      // border
	fl_rectf(x()+1, y()+1, w()-2, h()-2, FL_BACKGROUND_COLOR);  						// bg
	fl_rectf(x()+(G_beats*cursorW)+3, y()+3, cursorW-5, h()-6, COLOR_BG_2); // cursor

	/* beat cells */

	fl_color(COLOR_BD_0);
	for (int i=1; i<=G_Mixer.beats; i++)
		fl_line(x()+cursorW*i, y()+1, x()+cursorW*i, y()+h()-2);

	/* bar line */

	fl_color(COLOR_BG_2);
	int delta = G_Mixer.beats / G_Mixer.bars;
	for (int i=1; i<G_Mixer.bars; i++)
		fl_line(x()+cursorW*(i*delta), y()+1, x()+cursorW*(i*delta), y()+h()-2);

	/* unused grey area */

	fl_rectf(x()+(G_Mixer.beats*cursorW)+1, y()+1, ((MAX_BEATS-G_Mixer.beats)*cursorW)-1, h()-2, COLOR_BG_1);
}


/* ------------------------------------------------------------------ */


//int gModeBox::id_generator = 0;

gModeBox::gModeBox(int x, int y, int w, int h, channel *ch, const char *L)
	: Fl_Menu_Button(x, y, w, h, L), ch(ch)
{
	box(G_BOX);
	textsize(11);
	textcolor(COLOR_TEXT_0);
	color(COLOR_BG_0);
	add("Loop . basic", 	  0, cb_change_chanmode, (void *)LOOP_BASIC);
	add("Loop . once", 		  0, cb_change_chanmode, (void *)LOOP_ONCE);
	add("Loop . repeat", 	  0, cb_change_chanmode, (void *)LOOP_REPEAT);
	add("Oneshot . basic",  0, cb_change_chanmode, (void *)SINGLE_BASIC);
	add("Oneshot . press",  0, cb_change_chanmode, (void *)SINGLE_PRESS);
	add("Oneshot . retrig", 0, cb_change_chanmode, (void *)SINGLE_RETRIG);
	//id = id_generator++;
}

void gModeBox::draw() {
	fl_rect(x(), y(), w(), h(), COLOR_BD_0);		// border
	switch (ch->mode) {
		case LOOP_BASIC:
			fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
			break;
		case LOOP_ONCE:
			fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
			break;
		case LOOP_REPEAT:
			fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
			break;
		case SINGLE_BASIC:
			fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
			break;
		case SINGLE_PRESS:
			fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
			break;
		case SINGLE_RETRIG:
			fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
			break;
	}
}

void gModeBox::cb_change_chanmode(Fl_Widget *v, void *p) { ((gModeBox*)v)->__cb_change_chanmode((intptr_t)p); }

void gModeBox::__cb_change_chanmode(int mode) {
	ch->mode = mode;

	/* what to do when the channel is playing and you change the mode?
	 * Nothing, since v0.5.3. Just refresh the action editor window, in
	 * case it's open */

	gu_refreshActionEditor();
}

/* ------------------------------------------------------------------ */

gChoice::gChoice(int x, int y, int w, int h, const char *l, bool ang)
	: Fl_Choice(x, y, w, h, l), angle(ang) {
	labelsize(11);
	labelcolor(COLOR_TEXT_0);
	box(FL_BORDER_BOX);
	textsize(11);
	textcolor(COLOR_TEXT_0);
	color(COLOR_BG_0);
}

void gChoice::draw() {
	fl_rectf(x(), y(), w(), h(), COLOR_BG_0);              // bg
	fl_rect(x(), y(), w(), h(), (Fl_Color) COLOR_BD_0);    // border
	if (angle)
		fl_polygon(x()+w()-8, y()+h()-1, x()+w()-1, y()+h()-8, x()+w()-1, y()+h()-1);

	/* pick up the text() from the selected item (value()) and print it in
	 * the box and avoid overflows */

	fl_color(!active() ? COLOR_BD_0 : COLOR_TEXT_0);
	if (value() != -1) {
		if (fl_width(text(value())) < w()-8) {
			fl_draw(text(value()), x(), y(), w(), h(), FL_ALIGN_CENTER);
		}
		else {
			std::string tmp = text(value());
			int size        = tmp.size();
			while (fl_width(tmp.c_str()) >= w()-16) {
				tmp.resize(size);
				size--;
			}
			tmp += "...";
			fl_draw(tmp.c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER);
		}

	}
}

/* ------------------------------------------------------------------ */

void gDrawBox(int x, int y, int w, int h, Fl_Color c) {
	fl_color(c);
  fl_rectf(x, y, w, h);
  fl_color(COLOR_BD_0);
  fl_rect(x, y, w, h);
}


/* ------------------------------------------------------------------ */


gSlider::gSlider(int x, int y, int w, int h, const char *l)
	: Fl_Slider(x, y, w, h, l)
{
	type(FL_HOR_FILL_SLIDER);

	labelsize(11);
	align(FL_ALIGN_LEFT);
	labelcolor(COLOR_TEXT_0);

	box(G_BOX);
	color(COLOR_BG_0);
	selection_color(COLOR_BD_0);
}

