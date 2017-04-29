/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../core/const.h"
#include "../core/channel.h"
#include "../core/recorder.h"
#include "../utils/gui.h"
#include "recorder.h"


using namespace giada;


namespace
{
void updateChannel(geChannel *gch)
{
  gch->ch->hasActions = recorder::hasActions(gch->ch->index);
  if (!gch->ch->hasActions)
    static_cast<geSampleChannel*>(gch)->hideActionButton();
  /* TODO - set mute=false */
  gu_refreshActionEditor(); // refresh a.editor window, it could be open
}
}; // {namespace}


/* -------------------------------------------------------------------------- */


void glue_clearAllActions(geChannel *gch)
{
  if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
    return;
  recorder::clearChan(gch->ch->index);
  updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearVolumeActions(geChannel *gch)
{
  if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
    return;
  recorder::clearAction(gch->ch->index, G_ACTION_VOLUME);
  updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearStartStopActions(geChannel *gch)
{
  if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
    return;
  recorder::clearAction(gch->ch->index, G_ACTION_KEYPRESS | G_ACTION_KEYREL | G_ACTION_KILL);
  updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearMuteActions(geChannel *gch)
{
  if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
    return;
  recorder::clearAction(gch->ch->index, G_ACTION_MUTEON | G_ACTION_MUTEOFF);
  updateChannel(gch);
}
