/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * patch
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef __PATCH_DEPR_H__
#define __PATCH_DEPR_H__

#include <stdio.h>
#include <string>
#include <stdint.h>
#include "dataStorageIni.h"
#include "const.h"


class Patch_DEPR_ : public DataStorageIni
{
private:

	int readMasterPlugins(int type);

public:

	char  name[G_MAX_PATCHNAME_LEN];
	float version;
	int   lastTakeId;
	int   samplerate;

	int         open(const char *file);
	void        setDefault();
	int         close();

	void				getName       ();
	int         getNumChans   ();
	int					getNumColumns ();
	std::string getSamplePath (int i);
	float       getVol        (int i);
	int         getMode       (int i);
	int         getMute       (int i);
	int         getMute_s     (int i);
	int         getSolo       (int i);
	int         getBegin      (int i);
	int         getEnd        (int i, unsigned sampleSize);
	float       getBoost      (int i);
	float       getPanLeft    (int i);
	float       getPanRight   (int i);
	float       getPitch      (int i);
	bool        getRecActive  (int i);
	int         getColumn     (int i);
	int         getIndex      (int i);
	int         getType       (int i);
	int         getKey        (int i);
	uint32_t    getMidiValue  (int i, const char *c);
	float       getOutVol     ();
	float       getInVol      ();
	float       getBpm        ();
	int         getBars       ();
	int         getBeats      ();
	int         getQuantize   ();
	bool        getMetronome  ();
	int         getLastTakeId ();
	int         getSamplerate ();

	int         readRecs();
#ifdef WITH_VST
	int         readPlugins();
#endif
};

#endif
