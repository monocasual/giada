/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_SAMPLE_EDITOR_API_H
#define G_SAMPLE_EDITOR_API_H

#include "core/model/model.h"
#include "core/types.h"
#include "core/waveFx.h"
#include <memory>

namespace giada::m::rendering
{
class Reactor;
}

namespace giada::m
{
class KernelAudio;
class ChannelManager;
class Wave;
class SampleEditorApi
{
public:
	SampleEditorApi(KernelAudio&, model::Model&, ChannelManager&, rendering::Reactor&);

	void          loadPreviewChannel(ID sourceChannelId);
	void          freePreviewChannel();
	void          setPreviewTracker(Frame);
	void          setPreviewLoop(bool);
	void          togglePreview();
	Frame         getPreviewTracker();
	ChannelStatus getPreviewStatus();

	void           cut(ID channelId, Frame a, Frame b);
	void           copy(ID channelId, Frame a, Frame b);
	void           paste(ID channelId, Frame a);
	void           silence(ID channelId, Frame a, Frame b);
	void           fade(ID channelId, Frame a, Frame b, wfx::Fade);
	void           smoothEdges(ID channelId, Frame a, Frame b);
	void           reverse(ID channelId, Frame a, Frame b);
	void           normalize(ID channelId, Frame a, Frame b);
	void           trim(ID channelId, Frame a, Frame b);
	void           shift(ID channelId, Frame offset);
	const Channel& toNewChannel(ID channelId, Frame a, Frame b);
	void           setBeginEnd(ID channelId, Frame b, Frame e);
	void           resetBeginEnd(ID channelId);
	void           reload(ID channelId);

private:
	Wave& getWave(ID channelId) const;

	KernelAudio&        m_kernelAudio;
	model::Model&       m_model;
	ChannelManager&     m_channelManager;
	rendering::Reactor& m_reactor;

	/* waveBuffer
	A Wave used during cut/copy/paste operations. */

	std::unique_ptr<m::Wave> m_waveBuffer;
};
} // namespace giada::m

#endif
