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


#ifndef GE_SAMPLE_ACTION_H
#define GE_SAMPLE_ACTION_H


#include "../../../core/recorder.h"
#include "baseAction.h"


class SampleChannel;


namespace giada {
namespace v
{
class geSampleAction : public geBaseAction
{
private:

	const SampleChannel* m_ch;

public:

	geSampleAction(Pixel x, Pixel y, Pixel w, Pixel h, const SampleChannel* ch,
		m::recorder::action a1, m::recorder::action a2);

	void draw() override;
};
}} // giada::v::


#endif
