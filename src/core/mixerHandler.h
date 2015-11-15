/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#ifndef MIXERHANDLER_H
#define MIXERHANDLER_H


#include "recorder.h"
#include "patch.h"


/* stopSequencer
 * stop the sequencer, with special case if samplesStopOnSeqHalt is
 * true. */

void mh_stopSequencer();

void mh_rewindSequencer();

/* uniqueSolo
 * true if ch is the only solo'd channel in mixer. */

bool mh_uniqueSolo(class Channel *ch);

/* loadPatch
 * load a path or a project (if isProject) into Mixer. If isProject, path
 * must contain the address of the project folder. */

void mh_loadPatch_DEPR_(bool isProject, const char *projPath=0);
void mh_readPatch();

/* startInputRec - record from line in
 * creates a new empty wave in the first available channels and returns
 * the chan number chosen, otherwise -1 if there are no more empty
 * channels available. */

SampleChannel *mh_startInputRec();

SampleChannel *mh_stopInputRec();

/* uniqueSamplename
 * return true if samplename 'n' is unique. Requires SampleChannel *ch
 * in order to skip check against itself. */

bool mh_uniqueSamplename(class SampleChannel *ch, const char *name);

#ifdef WITH_VST

static int __mh_readPatchPlugins__(gVector<Patch::plugin_t> *list, int type);

#endif

#endif
