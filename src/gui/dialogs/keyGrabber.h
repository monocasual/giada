/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GD_KEYGRABBER_H
#define GD_KEYGRABBER_H

#include "src/gui/dialogs/window.h"
#include <FL/Fl.H>
#include <functional>

namespace giada::c::channel
{
struct Data;
}

namespace giada::v
{
class geBox;
class geTextButton;
class gdKeyGrabber : public gdWindow
{
public:
	gdKeyGrabber(int key);

	int  handle(int e) override;
	void rebuild() override;

	/* onSetKey
	Callback fired when this widget has grabbed an event. Returns a boolean value
	to inform the widget if the key is valid. */

	std::function<bool(int key)> onSetKey;

private:
	int m_key;

	geBox*        m_text;
	geTextButton* m_clear;
	geTextButton* m_cancel;
};
} // namespace giada::v

#endif
