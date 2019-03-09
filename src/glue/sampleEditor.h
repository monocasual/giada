/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef G_GLUE_SAMPLE_EDITOR_H
#define G_GLUE_SAMPLE_EDITOR_H


#include "core/types.h"


namespace giada {
namespace c {
namespace sampleEditor 
{
/* setBeginEnd
Sets start/end points in the sample editor. */

void setBeginEnd(ID chanID, int b, int e);

void cut(ID chanID, int a, int b);
void copy(ID chanID, int a, int b);

/* paste
Pastes what's defined in m_copyBuffer into channel 'ch' at point 'a'. If 
m_copyBuffer is empty, does nothing. */

void paste(ID chanID, int a);

void trim(ID chanID, int a, int b);
void reverse(ID chanID, int a, int b);
void normalizeHard(ID chanID, int a, int b);
void silence(ID chanID, int a, int b);
void fade(ID chanID, int a, int b, int type);
void smoothEdges(ID chanID, int a, int b);
void shift(ID chanID, int offset);
void reload(ID chanID);

bool isWaveBufferFull();

/* setPlayHead
Changes playhead's position. Used in preview. */

void setPlayHead(ID chanID, Frame f);

void setPreview(ID chanID, PreviewMode mode);
void rewindPreview(ID chanID);

/* toNewChannel
Copies the selected range into a new sample channel. */

void toNewChannel(ID chanID, int a, int b);
}}}; // giada::c::sampleEditor::

#endif
