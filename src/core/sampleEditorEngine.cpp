/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/sampleEditorEngine.h"
#include "core/engine.h"

namespace giada::m
{
SampleEditorEngine::SampleEditorEngine(Engine& e, ChannelManager& cm)
: m_engine(e)
, m_channelManager(cm)
{
}

/* -------------------------------------------------------------------------- */

void SampleEditorEngine::setBeginEnd(ID channelId, Frame b, Frame e)
{
	m_channelManager.setBeginEnd(channelId, b, e);
}

void SampleEditorEngine::resetBeginEnd(ID channelId)
{
	m_channelManager.resetBeginEnd(channelId);
}
} // namespace giada::m
