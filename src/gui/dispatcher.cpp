/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "dispatcher.h"
#include "core/init.h"
#include "glue/events.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/ui.h"
#include <FL/Fl.H>
#include <cassert>

extern giada::v::Ui g_ui;

namespace giada::v
{
Dispatcher::Dispatcher()
: m_backspace(false)
, m_end(false)
, m_enter(false)
, m_space(false)
, m_esc(false)
, m_key(false)
{
}

/* -------------------------------------------------------------------------- */

void Dispatcher::perform(ID channelId, int event) const
{
	if (event == FL_KEYDOWN)
	{
		if (Fl::event_ctrl())
			c::events::toggleMuteChannel(channelId, Thread::MAIN);
		else if (Fl::event_shift())
			c::events::killChannel(channelId, Thread::MAIN);
		else
			c::events::pressChannel(channelId, G_MAX_VELOCITY, Thread::MAIN);
	}
	else if (event == FL_KEYUP)
		c::events::releaseChannel(channelId, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

/* Walk channels array, trying to match button's bound key with the event. If 
found, trigger the key-press/key-release function. */

void Dispatcher::dispatchChannels(int event) const
{
	g_ui.mainWindow->keyboard->forEachChannel([=](geChannel& c) {
		if (c.handleKey(event))
			perform(c.getData().id, event);
	});
}

/* -------------------------------------------------------------------------- */

void Dispatcher::dispatchKey(int event)
{
	assert(onEventOccured != nullptr);

	/* These events come from the keyboard, not from a direct interaction on the 
	UI with the mouse/touch. */

	if (event == FL_KEYDOWN)
	{
		if (Fl::event_key() == FL_BackSpace && !m_backspace)
		{
			m_backspace = true;
			c::events::rewindSequencer(Thread::MAIN);
		}
		else if (Fl::event_key() == FL_End && !m_end)
		{
			m_end = true;
			c::events::toggleInputRecording();
		}
		else if (Fl::event_key() == FL_Enter && !m_enter)
		{
			m_enter = true;
			c::events::toggleActionRecording();
		}
		else if (Fl::event_key() == ' ' && !m_space)
		{
			m_space = true;
			c::events::toggleSequencer(Thread::MAIN);
		}
		else if (Fl::event_key() == FL_Escape && !m_esc)
		{
			m_esc = true;
			m::init::closeMainWindow();
		}
		else if (!m_key)
		{
			m_key = true;
			onEventOccured();
			dispatchChannels(event);
		}
	}
	else if (event == FL_KEYUP)
	{
		if (Fl::event_key() == FL_BackSpace)
			m_backspace = false;
		else if (Fl::event_key() == FL_End)
			m_end = false;
		else if (Fl::event_key() == ' ')
			m_space = false;
		else if (Fl::event_key() == FL_Enter)
			m_enter = false;
		else if (Fl::event_key() == FL_Escape)
			m_esc = false;
		else
		{
			m_key = false;
			dispatchChannels(event);
		}
	}
}

/* -------------------------------------------------------------------------- */

void Dispatcher::dispatchTouch(const geChannel& gch, bool status)
{
	assert(onEventOccured != nullptr);

	onEventOccured();
	perform(gch.getData().id, status ? FL_KEYDOWN : FL_KEYUP);
}
} // namespace giada::v