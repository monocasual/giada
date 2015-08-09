/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channelButton
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#ifndef GE_CHANNEL_BUTTON_H
#define GE_CHANNEL_BUTTON_H


#include "ge_mixed.h"


class gChannelButton : public gClick
{
private:

	std::string key;

public:

	gChannelButton(int x, int y, int w, int h, const char *l=0);

	virtual int handle(int e) = 0;

	void draw();
	void setKey(const char *k);
	void setPlayMode();
	void setEndingMode();
	void setDefaultMode(const char *l=0);
	void setInputRecordMode();
	void setActionRecordMode();
};


#endif
