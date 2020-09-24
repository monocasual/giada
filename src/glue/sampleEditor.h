/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <functional>
#include <string>
#include "core/types.h"
#include "core/waveFx.h"


namespace giada {
namespace m
{
class Channel;
class Wave;
}
namespace v 
{
class gdSampleEditor;
}
namespace c {
namespace sampleEditor 
{
struct Data
{
    Data() = default;
    Data(const m::Channel&, const m::Wave&);

    ChannelStatus a_getPreviewStatus() const;
    Frame a_getPreviewTracker() const;

    ID          channelId; 
    ID          waveId; 
    std::string name;
    float       volume;
    float       pan;
    float       pitch;
    Frame       begin;
    Frame       end;
    Frame       shift;
    Frame       waveSize;
    int         waveBits;
    int         waveDuration;
    int         waveRate;
    std::string wavePath;
    bool        isLogical;
};

/* onRefresh --- TODO - wrong name */

void onRefresh(bool gui, std::function<void(v::gdSampleEditor&)> f);

/* getData
Returns a Data object filled with data from a channel. */

Data getData(ID channelId);

/* setBeginEnd
Sets start/end points in the sample editor. */

void setBeginEnd(ID channelId, int b, int e);

void cut(ID channelId, ID waveId, int a, int b);
void copy(ID waveId, int a, int b);
void paste(ID channelId, ID waveId, int a);

void trim(ID channelId, ID waveId, int a, int b);
void reverse(ID channelId, ID waveId, int a, int b);
void normalize(ID channelId, ID waveId, int a, int b);
void silence(ID channelId, ID waveId, int a, int b);
void fade(ID channelId, ID waveId, int a, int b, m::wfx::Fade type);
void smoothEdges(ID channelId, ID waveId, int a, int b);
void shift(ID channelId, ID waveId, int offset);
void reload(ID channelId, ID waveId);

bool isWaveBufferFull();

void playPreview(bool loop);
void stopPreview();
void setPreviewTracker(Frame f);
void cleanupPreview();

/* toNewChannel
Copies the selected range into a new sample channel. */

void toNewChannel(ID channelId, ID waveId, int a, int b);
}}} // giada::c::sampleEditor::

#endif
