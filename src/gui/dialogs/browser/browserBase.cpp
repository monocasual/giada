/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "core/graphics.h"
#include "core/conf.h"
#include "core/const.h"
#include "utils/gui.h"
#include "utils/fs.h"
#include "gui/elems/browser.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/progress.h"
#include "gui/elems/basics/check.h"
#include "browserBase.h"


namespace giada {
namespace v
{
gdBrowserBase::gdBrowserBase(const std::string& title, const std::string& path, 
	std::function<void(void*)> callback, ID channelId)
: gdWindow   (m::conf::conf.browserX, m::conf::conf.browserY, m::conf::conf.browserW, 
	          m::conf::conf.browserH, title.c_str()),
  m_callback (callback),
  m_channelId(channelId)
{
	set_non_modal();

	groupTop = new Fl_Group(8, 8, w()-16, 48);
    hiddenFiles = new geCheck(groupTop->x(), groupTop->y(), 400, 20, "Show hidden files");
		where   = new geInput(groupTop->x(), hiddenFiles->y()+hiddenFiles->h()+8, 20, 20);
		updir   = new geButton(groupTop->x()+groupTop->w()-20, where->y(), 20, 20, "", updirOff_xpm, updirOn_xpm);
	groupTop->end();
	groupTop->resizable(where);

	hiddenFiles->callback(cb_toggleHiddenFiles, (void*) this);

	where->readonly(true);
	where->cursor_color(G_COLOR_BLACK);
	where->value(path.c_str());

	updir->callback(cb_up, (void*) this);

	browser = new geBrowser(8, groupTop->y()+groupTop->h()+8, w()-16, h()-101);
	browser->loadDir(path);
	if (path == m::conf::conf.browserLastPath)
		browser->preselect(m::conf::conf.browserPosition, m::conf::conf.browserLastValue);

	Fl_Group* groupButtons = new Fl_Group(8, browser->y()+browser->h()+8, w()-16, 20);
		ok      = new geButton(w()-88, groupButtons->y(), 80, 20);
		cancel  = new geButton(w()-ok->w()-96, groupButtons->y(), 80, 20, "Cancel");
		status  = new geProgress(8, groupButtons->y(), cancel->x()-16, 20);
		status->minimum(0);
		status->maximum(1);
		status->hide();   // show the bar only if necessary
	groupButtons->resizable(status);
	groupButtons->end();

	end();

	cancel->callback(cb_close, (void*) this);

	resizable(browser);
	size_range(320, 200);

	u::gui::setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


gdBrowserBase::~gdBrowserBase()
{
	m::conf::conf.browserX = x();
	m::conf::conf.browserY = y();
	m::conf::conf.browserW = w();
	m::conf::conf.browserH = h();
	m::conf::conf.browserPosition  = browser->position();
	m::conf::conf.browserLastPath  = browser->getCurrentDir();
	m::conf::conf.browserLastValue = browser->value();
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::cb_up(Fl_Widget* v, void* p) { ((gdBrowserBase*)p)->cb_up(); }
void gdBrowserBase::cb_close(Fl_Widget* v, void* p) { ((gdBrowserBase*)p)->cb_close(); }
void gdBrowserBase::cb_toggleHiddenFiles(Fl_Widget *v, void *p) { ((gdBrowserBase*)p)->cb_toggleHiddenFiles(); }


/* -------------------------------------------------------------------------- */


void gdBrowserBase::cb_up()
{
	browser->loadDir(u::fs::getUpDir(browser->getCurrentDir()));
	where->value(browser->getCurrentDir().c_str());
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::cb_close()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::cb_toggleHiddenFiles()
{
	browser->toggleHiddenFiles();
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::setStatusBar(float v)
{
	status->value(status->value() + v);
	Fl::wait(0);
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::showStatusBar()
{
	status->show();
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::hideStatusBar()
{
	status->hide();
}


/* -------------------------------------------------------------------------- */


std::string gdBrowserBase::getCurrentPath() const 
{ 
	return where->value(); 
}


ID gdBrowserBase::getChannelId() const 
{ 
	return m_channelId; 
}


std::string gdBrowserBase::getSelectedItem() const 
{
	return browser->getSelectedItem();
}


/* -------------------------------------------------------------------------- */


void gdBrowserBase::fireCallback() const
{ 
	m_callback((void*)this);
}

}} // giada::v::
