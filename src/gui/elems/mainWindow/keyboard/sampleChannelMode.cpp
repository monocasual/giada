/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_modeBox
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

#include "gui/elems/mainWindow/keyboard/sampleChannelMode.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/menu.h"
#include "gui/graphics.h"
#include "utils/gui.h"
#include <FL/fl_draw.H>
#include <cassert>

namespace giada::v
{
geSampleChannelMode::geSampleChannelMode(int x, int y, int w, int h, c::channel::Data& d)
: geImageButton(x, y, w, h, nullptr, nullptr)
, m_channel(d)
{
	onClick = [this]()
	{
		openMenu();
	};

	refresh(m_channel.sample->mode);
}

/* -------------------------------------------------------------------------- */

void geSampleChannelMode::refresh(SamplePlayerMode mode)
{
	switch (mode)
	{
	case SamplePlayerMode::LOOP_BASIC:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopBasic);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopBasic);
		break;
	case SamplePlayerMode::LOOP_ONCE:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopOnce);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopOnce);
		break;
	case SamplePlayerMode::LOOP_ONCE_BAR:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopOnceBar);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopOnceBar);
		break;
	case SamplePlayerMode::LOOP_REPEAT:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopRepeat);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::loopRepeat);
		break;
	case SamplePlayerMode::SINGLE_BASIC:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotBasic);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotBasic);
		break;
	case SamplePlayerMode::SINGLE_BASIC_PAUSE:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotBasicPause);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotBasicPause);
		break;
	case SamplePlayerMode::SINGLE_PRESS:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotPress);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotPress);
		break;
	case SamplePlayerMode::SINGLE_RETRIG:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotRetrig);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotRetrig);
		break;
	case SamplePlayerMode::SINGLE_ENDLESS:
		m_imgOff = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotEndless);
		m_imgOn  = std::make_unique<Fl_SVG_Image>(nullptr, graphics::oneshotEndless);
		break;
	default:
		assert(false);
		break;
	}

	redraw();
}

/* -------------------------------------------------------------------------- */

void geSampleChannelMode::openMenu()
{
	geMenu menu;

	menu.addItem((ID)SamplePlayerMode::LOOP_BASIC, "Loop - Basic");
	menu.addItem((ID)SamplePlayerMode::LOOP_ONCE, "Loop - Once");
	menu.addItem((ID)SamplePlayerMode::LOOP_ONCE_BAR, "Loop - Once bar");
	menu.addItem((ID)SamplePlayerMode::LOOP_REPEAT, "Loop - Repeat");
	menu.addItem((ID)SamplePlayerMode::SINGLE_BASIC, "Oneshot - Basic");
	menu.addItem((ID)SamplePlayerMode::SINGLE_BASIC_PAUSE, "Oneshot - Basic pause");
	menu.addItem((ID)SamplePlayerMode::SINGLE_PRESS, "Oneshot - Press");
	menu.addItem((ID)SamplePlayerMode::SINGLE_RETRIG, "Oneshot - Retrig");
	menu.addItem((ID)SamplePlayerMode::SINGLE_ENDLESS, "Oneshot - Endless");

	menu.onSelect = [this](ID id)
	{
		const SamplePlayerMode mode = static_cast<SamplePlayerMode>(id);
		c::channel::setSamplePlayerMode(m_channel.id, mode);
		refresh(mode);
	};

	menu.popup();
}
} // namespace giada::v