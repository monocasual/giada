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


#ifndef G_MASTER_CHANNEL_H
#define G_MASTER_CHANNEL_H


#include "core/channels/channel.h"


namespace giada {
namespace m 
{
class MasterChannel : public Channel
{
public:

	MasterChannel(int bufferSize, ID id);
	MasterChannel(const patch::Channel& p, int bufferSize);

	MasterChannel* clone() const override;
	void load(const patch::Channel& p) override;
	void parseEvents(mixer::FrameEvents fe) override {};
	void render(AudioBuffer& out, const AudioBuffer& in, AudioBuffer& inToOut, 
		bool audible, bool running) override;
	void start(int frame, bool doQuantize, int velocity) override {};
	void kill(int localFrame) override {};
	void empty() override {};
	void stopBySeq(bool chansStopOnSeqHalt) override {};
	void stop() override {};
	void rewindBySeq() override {};
	void setMute(bool value) override {};
	void setSolo(bool value) override {};
	void receiveMidi(const MidiEvent& midiEvent) override {};
};

}} // giada::m::


#endif
