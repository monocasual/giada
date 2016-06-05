/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include "../gui/dialogs/gd_browser.h"
#include "../gui/dialogs/gd_warnings.h"
#include "storage.h"
#include "browser.h"


using std::string;


void glue_loadPatchBrowser(void *data)
{
  gdLoadBrowser *browser = (gdLoadBrowser*) data;

	bool isProject = gIsProject(browser->getSelectedItem());
	int res = glue_loadPatch(browser->getSelectedItem(), browser->getStatus(), isProject);

  browser->hideStatus();

	if (res == PATCH_UNREADABLE) {
		if (isProject)
			gdAlert("This project is unreadable.");
		else
			gdAlert("This patch is unreadable.");
	}
	else
	if (res == PATCH_INVALID) {
		if (isProject)
			gdAlert("This project is not valid.");
		else
			gdAlert("This patch is not valid.");
	}
	else
		browser->do_callback();
}
