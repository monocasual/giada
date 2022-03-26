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

#ifdef WITH_VST

#include "gui/dialogs/pluginWindowGUI.h"
#include "core/const.h"
#include "glue/plugin.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <FL/x.H>
#ifdef G_OS_MAC
#import "utils/cocoa.h" // objective-c
#endif

namespace giada::v
{
gdPluginWindowGUI::gdPluginWindowGUI(c::plugin::Plugin& p)
#ifdef G_OS_MAC
: gdWindow(Fl::w(), Fl::h())
#else
: gdWindow(320, 200)
#endif
, m_plugin(p)
{
	/* Make sure to wait_for_expose() before opening the editor: the window must
	be exposed and visible first. Don't fuck with multithreading! */

	copy_label(m_plugin.name.c_str());
	show();
	wait_for_expose();
	openEditor();
	Fl::flush();
}

/* -------------------------------------------------------------------------- */

gdPluginWindowGUI::~gdPluginWindowGUI()
{
	c::plugin::stopDispatchLoop();
	closeEditor();
	u::log::print("[gdPluginWindowGUI::__cb_close] GUI closed, this=%p\n", (void*)this);
}

/* -------------------------------------------------------------------------- */

void gdPluginWindowGUI::openEditor()
{
	u::log::print("[gdPluginWindowGUI] Opening editor, this=%p, xid=%p\n",
	    this, reinterpret_cast<void*>(fl_xid(this)));

	m_editor.reset(m_plugin.createEditor());
	if (m_editor == nullptr)
	{
		u::log::print("[gdPluginWindowGUI::openEditor] unable to create editor!\n");
		return;
	}
	m_editor->setOpaque(true);

#ifdef G_OS_MAC

	void* cocoaWindow = (void*)fl_xid(this);
	m_editor->addToDesktop(0, cocoa_getViewFromWindow(cocoaWindow));

#else

	m_editor->addToDesktop(0, reinterpret_cast<void*>(fl_xid(this)));

#endif

	const int pluginW = m_editor->getWidth();
	const int pluginH = m_editor->getHeight();

	resize((Fl::w() - pluginW) / 2, (Fl::h() - pluginH) / 2, pluginW, pluginH);

	m_plugin.setResizeCallback([this](int w, int h) {
		resize(x(), y(), w, h);
	});

	c::plugin::startDispatchLoop();
}

/* -------------------------------------------------------------------------- */

void gdPluginWindowGUI::closeEditor()
{
	m_plugin.setResizeCallback(nullptr);
	m_editor.reset();
}
} // namespace giada::v

#endif // #ifdef WITH_VST
