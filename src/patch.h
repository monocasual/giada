/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011 Giovanni A. Zuliani | Monocasual
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


#ifndef __PATCH_H__
#define __PATCH_H__

#include <stdio.h>
#include <string>
#include "init.h"
#include "recorder.h"
#include "utils.h"
#include "dataStorage.h"


class Patch : public DataStorage {

public:

	char name[MAX_PATCHNAME_LEN];
	int  lastTakeId;
	int  samplerate;

	int         open(const char *file);
	void        setDefault();
	int         close();

	void				getName();
	std::string getSamplePath(int chan);
	float       getVol       (int chan);
	int         getMode      (int chan);
	int         getMute      (int chan);
	int         getStart     (int chan);
	int         getEnd       (int chan, unsigned sampleSize);
	float       getBoost     (int chan);
	float       getPanLeft   (int chan);
	float       getPanRight  (int chan);
	float       getPitch     (int chan);
	bool        getRecActive (int chan);
	float       getOutVol    ();
	float       getInVol     ();
	float       getBpm       ();
	int         getBars      ();
	int         getBeats     ();
	int         getQuantize  ();
	bool        getMetronome ();
	int         getLastTakeId();
	int         getSamplerate();

	int         write(const char *file, const char *name);
	int         readRecs();
	int         readPlugins();
};

#endif
