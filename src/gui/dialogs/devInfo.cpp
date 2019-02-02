/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
 * -------------------------------------------------------------------------- */


#include <FL/fl_draw.H>
#include "../../core/kernelAudio.h"
#include "../../utils/gui.h"
#include "../../utils/string.h"
#include "../elems/basics/button.h"
#include "../elems/basics/box.h"
#include "window.h"
#include "devInfo.h"


using std::string;
using namespace giada;


gdDevInfo::gdDevInfo(unsigned dev)
	: Fl_Window(340, 300, "Device information")
{
	set_modal();

	text  = new geBox(8, 8, 320, 200, "", (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
	close = new geButton(252, h()-28, 80, 20, "Close");
	end();

	string body  = "";
	int    lines = 7;

	body  = "Device name: " + m::kernelAudio::getDeviceName(dev) + "\n";
	body += "Total output(s): " + u::string::iToString(m::kernelAudio::getMaxOutChans(dev)) + "\n";
	body += "Total intput(s): " + u::string::iToString(m::kernelAudio::getMaxInChans(dev)) + "\n";
	body += "Duplex channel(s): " + u::string::iToString(m::kernelAudio::getDuplexChans(dev)) + "\n";
	body += "Default output: " + string(m::kernelAudio::isDefaultOut(dev) ? "yes" : "no") + "\n";
	body += "Default input: " + string(m::kernelAudio::isDefaultIn(dev) ? "yes" : "no") + "\n";

	int totalFreq = m::kernelAudio::getTotalFreqs(dev);
	body += "Supported frequencies: " + u::string::iToString(totalFreq);

	for (int i=0; i<totalFreq; i++) {
		if (i % 6 == 0) {
			body += "\n    ";  // add new line each 6 printed freqs AND on the first line (i % 0 != 0)
			lines++;
		}
		body += u::string::iToString(m::kernelAudio::getFreq(dev, i)) + "  ";
	}

	text->copy_label(body.c_str());

	/* resize the window to fit the content. fl_height() returns the height
	 * of a line. fl_height() * total lines + margins + button size */

	resize(x(), y(), w(), (lines * fl_height()) + 8 + 8 + 8 + 20);
	close->position(close->x(), (lines * fl_height()) + 8 + 8);

	close->callback(__cb_window_closer, (void*)this);
	u::gui::setFavicon(this);
	show();
}


gdDevInfo::~gdDevInfo() {}
