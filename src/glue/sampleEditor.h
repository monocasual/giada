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


#ifndef G_GLUE_SAMPLE_EDITOR_H
#define G_GLUE_SAMPLE_EDITOR_H


class SampleChannel;
class geWaveform;


namespace giada {
namespace c     {
namespace sampleEditor 
{


/* setBeginEndChannel
Sets start/end points in the sample editor. */

void setBeginEndChannel(SampleChannel* ch, int b, int e);

void cut(SampleChannel* ch, int a, int b);
void trim(SampleChannel* ch, int a, int b);
void silence(SampleChannel* ch, int a, int b);
void fade(SampleChannel* ch, int a, int b, int type);
void smoothEdges(SampleChannel* ch, int a, int b);
void setStartEnd(SampleChannel* ch, int a, int b);

/* setPlayHead
Changes playhead's position. Used in preview. */

void setPlayHead(SampleChannel* ch, int f);

/* togglePreview
Sets preview to on or off. */

void togglePreview(SampleChannel* ch);

void rewindPreview(SampleChannel* ch);
}}}; // giada::c::sampleEditor::

#endif
