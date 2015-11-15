/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
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


#ifndef GLUE_STORAGE_H
#define GLUE_STORAGE_H


#include "../core/patch.h"


using std::string;


int glue_loadPatch  (const string &fullPath, class gProgress *status, bool isProject);
int glue_loadPatch__DEPR__(const char *fname, const char *fpath, class gProgress *status, bool isProject);
int glue_savePatch  (const string &fullPath, const string &name, bool isProject);
int glue_saveProject(const string &folderPath, const string &projName);

static void __glue_fillPatchGlobals__(const string &name);
static void __glue_fillPatchChannels__(bool isProject);
static void __glue_fillPatchColumns__();

#ifdef WITH_VST
static void __glue_fillPatchGlobalsPlugins__(gVector <Plugin *> *host, gVector<Patch::plugin_t> *patch);
#endif

static void __setProgressBar__(class gProgress *status, float v);

#endif
