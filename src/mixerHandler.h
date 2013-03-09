/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixerHandler
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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


/* startChan
 * starts a channel. do_quantize = false (don't quantize) when Mixer is
 * reading actions from Recorder::.  */

void mh_startChan(struct channel *ch, bool do_quantize=true);

void mh_stopChan(struct channel *ch);

void mh_deleteChannel(struct channel *ch);

void mh_freeChannel(struct channel *ch);


/* killChan
 * kill a chan abruptly, without checking LOOP_ENDING and such */

void mh_killChan(int c);


void mh_muteChan(channel *ch, bool internal=false);
void mh_unmuteChan(channel *ch, bool internal=false);

void mh_loadPatch();

int mh_loadChan(const char *file, struct channel *ch, bool push=true);


/* startInputRec - record from line in
 * creates a new empty wave in the first available channels and returns
 * the chan number chosen, otherwise -1 if there are no more empty
 * channels available. */

struct channel *mh_startInputRec();

channel *mh_stopInputRec();

void mh_rewind();

#endif
