/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "../core/types.h"


class geWaveform;


namespace giada {
namespace m
{
class SampleChannel;
}
namespace c {
namespace sampleEditor 
{
/* setBeginEnd
Sets start/end points in the sample editor. */

void setBeginEnd(m::SampleChannel* ch, int b, int e);

void cut(m::SampleChannel* ch, int a, int b);
void copy(m::SampleChannel* ch, int a, int b);

/* paste
Pastes what's defined in m_copyBuffer into channel 'ch' at point 'a'. If 
m_copyBuffer is empty, does nothing. */

void paste(m::SampleChannel* ch, int a);

void trim(m::SampleChannel* ch, int a, int b);
void reverse(m::SampleChannel* ch, int a, int b);
void normalizeHard(m::SampleChannel* ch, int a, int b);
void silence(m::SampleChannel* ch, int a, int b);
void fade(m::SampleChannel* ch, int a, int b, int type);
void smoothEdges(m::SampleChannel* ch, int a, int b);
void shift(m::SampleChannel* ch, int offset);

bool isWaveBufferFull();

/* setPlayHead
Changes playhead's position. Used in preview. */

void setPlayHead(m::SampleChannel* ch, int f);

void setPreview(m::SampleChannel* ch, PreviewMode mode);
void rewindPreview(m::SampleChannel* ch);

/* toNewChannel
Copies the selected range into a new sample channel. */

void toNewChannel(m::SampleChannel* ch, int a, int b);
}}}; // giada::c::sampleEditor::

#endif
