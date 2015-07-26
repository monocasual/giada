/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_devInfo
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include "gd_devInfo.h"
#include "ge_mixed.h"
#include "core/kernelAudio.h"
#include "gui_utils.h"


gdDevInfo::gdDevInfo(unsigned dev)
: Fl_Window(340, 300, "Device information") {
	set_modal();

	text  = new gBox(8, 8, 320, 200, "", (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
	close = new gClick(252, h()-28, 80, 20, "Close");
	end();

	std::string bufTxt;
	char bufNum[128];
	int  lines = 0;

	bufTxt  = "Device name: ";
	bufTxt += +kernelAudio::getDeviceName(dev);
	bufTxt += "\n";
	lines++;

	bufTxt += "Total output(s): ";
	sprintf(bufNum, "%d\n", kernelAudio::getMaxOutChans(dev));
	bufTxt += bufNum;
	lines++;

	bufTxt += "Total intput(s): ";
	sprintf(bufNum, "%d\n", kernelAudio::getMaxInChans(dev));
	bufTxt += bufNum;
	lines++;

	bufTxt += "Duplex channel(s): ";
	sprintf(bufNum, "%d\n", kernelAudio::getDuplexChans(dev));
	bufTxt += bufNum;
	lines++;

	bufTxt += "Default output: ";
	sprintf(bufNum, "%s\n", kernelAudio::isDefaultOut(dev) ? "yes" : "no");
	bufTxt += bufNum;
	lines++;

	bufTxt += "Default input: ";
	sprintf(bufNum, "%s\n", kernelAudio::isDefaultIn(dev) ? "yes" : "no");
	bufTxt += bufNum;
	lines++;

	int totalFreq = kernelAudio::getTotalFreqs(dev);
	bufTxt += "Supported frequencies: ";
	sprintf(bufNum, "%d", totalFreq);
	bufTxt += bufNum;
	lines++;

	for (int i=0; i<totalFreq; i++) {
		sprintf(bufNum, "%d  ", kernelAudio::getFreq(dev, i));
		if (i%6 == 0) {    // new line each X printed freqs AND on the first line (i%0 != 0)
			bufTxt += "\n    ";
			lines++;
		}
		bufTxt += bufNum;
	}

	text->copy_label(bufTxt.c_str());

	/* resize the window to fit the content. fl_height() returns the height
	 * of a line. fl_height() * total lines + margins + button size */

	resize(x(), y(), w(), lines*fl_height() + 8 + 8 + 8 + 20);
	close->position(close->x(), lines*fl_height() + 8 + 8);

	close->callback(__cb_window_closer, (void*)this);
	gu_setFavicon(this);
	show();
}


gdDevInfo::~gdDevInfo() {}
