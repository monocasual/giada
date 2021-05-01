/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "devInfo.h"
#include "core/kernelAudio.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "window.h"
#include <FL/fl_draw.H>

namespace giada::v
{
gdDevInfo::gdDevInfo(m::kernelAudio::Device device)
: gdWindow(340, 300, "Device information")
{
	set_modal();

	text  = new geBox(8, 8, 320, 200, "", (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_TOP));
	close = new geButton(252, h() - 28, 80, 20, "Close");
	end();

	std::string body  = "";
	int         lines = 7;

	body = "Device name: " + device.name + "\n";
	body += "Total output(s): " + std::to_string(device.maxOutputChannels) + "\n";
	body += "Total input(s): " + std::to_string(device.maxInputChannels) + "\n";
	body += "Duplex channel(s): " + std::to_string(device.maxDuplexChannels) + "\n";
	body += "Default output: " + std::string(device.isDefaultOut ? "yes" : "no") + "\n";
	body += "Default input: " + std::string(device.isDefaultIn ? "yes" : "no") + "\n";

	body += "Supported frequencies: " + std::to_string(device.sampleRates.size());

	int i = 0;
	for (int sampleRate : device.sampleRates)
	{
		if (i % 6 == 0)
		{
			body += "\n    "; // add new line each 6 printed freqs AND on the first line (i % 0 != 0)
			lines++;
		}
		body += std::to_string(sampleRate) + "  ";
		i++;
	}

	text->copy_label(body.c_str());

	/* Resize the window to fit the content. fl_height() returns the height of a 
	line. fl_height() * total lines + margins + button size */

	resize(x(), y(), w(), (lines * fl_height()) + 8 + 8 + 8 + 20);
	close->position(close->x(), (lines * fl_height()) + 8 + 8);

	close->callback(cb_window_closer, (void*)this);
	u::gui::setFavicon(this);
	show();
}
} // namespace giada::v
